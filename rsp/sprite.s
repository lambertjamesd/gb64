

###############################################
# Write scanline
# a0 - current y
sortSprites:
    li(t0, sprites) # read head
    li(t1, sprites) # write head

    # calculate sprite y size
    lbu t2, (ppuTask + PPUTask_lcdc)(zero)
    andi t2, t2, LCDC_OBJ_SIZE
    sll t2, t2, 1
    addi t2, t2, 0x8 # sprite size y

sortNextSprite:
    # check if sprite is hidden off the right side
    lbu t3, SPRITE_X(t0)
    slti $at, t3, SPRITE_OFFSCREEN_X
    beq $at, zero, checkNextSprite

    # check if sprite is on current line
    lbu $at, SPRITE_Y(t0)
    addi $at, $at, SPRITE_SHIFT_Y
    sub $at, a0, $at
    bltz $at, checkNextSprite
    slt $at, $at, t2
    beq $at, zero, checkNextSprite

    lw t4, 0(t0) # load sprite from read head

    # check if sprites should be sorted
    lhu $at, (ppuTask + PPUTask_flags)(zero)
    andi $at, $at, PPU_TASK_FLAGS_COLOR
    bne $at, zero, writeNextSprite
    ori t5, t1, 0 # write out at end of list

    li(t6, sprites) # compare head

checkNextSlot:
    # check if compare head has reached the write head
    beq t6, t1, writeNextSprite

    # compare the x positions of the tiles
    lbu $at, SPRITE_X(t6)
    slt $at, t3, $at
    beq $at, zero, checkNextSlot
    addi t6, t6, SPRITE_SIZE

    addi t6, t6, -SPRITE_SIZE
shiftSprites:
    addi t5, t5, -SPRITE_SIZE
    # shift sprite one slot
    lw $at, 0(t5)
    sw $at, SPRITE_SIZE(t5)
    bne t5, t6, shiftSprites
    nop

writeNextSprite:
    sw t4, 0(t5)
    # increment write head
    addi t1, t1, SPRITE_SIZE

checkNextSprite:
    addi t0, t0, SPRITE_SIZE
    slti $at, t0, sprites + SPRITE_SIZE * SPRITE_MAX_COUNT
    beq $at, zero, finishSortSprites
    slti $at, t1, sprites + SPRITE_SIZE * SPRITE_MAX_PER_LINE
    bne $at, zero, sortNextSprite
    nop
finishSortSprites:
    jr return
    sw zero, 0(t1) # null terminate sprite list

###############################################
# loads the tiles needed to render sprites
#
loadSpriteTiles:
    addi $sp, $sp, -4
    sw return, 0($sp)

    lbu t2, (ppuTask + PPUTask_lcdc)(zero)
    andi t2, t2, LCDC_OBJ_SIZE

    # check for gameboy color
    lhu t3, (ppuTask + PPUTask_flags)(zero)
    andi t3, t3, PPU_TASK_FLAGS_COLOR

    li(t0, sprites)
loadNextSpriteTile:
    lw t1, 0(t0)
    # check for null terminated sprite
    beq t1, zero, loadSpriteTilesFinish
    lbu a0, SPRITE_TILE(t0)

    # check for 16 high tile and flipped tile
    beq t2, zero, loadSpriteTileCheckVRAMBank
    lbu t4, SPRITE_Y(t0) # load sprite y in delay slot

    # in sprites are 2 tile high, the last bit
    # is treated as a 0
    andi a0, a0, 0xFFFE

    addi t4, t4, SPRITE_SHIFT_Y
    lbu $at, (ppuTask + PPUTask_ly)(zero)
    sub $at, $at, t4

    # $at will be 0x8 for the second tile
    # and 0 for the first tile
    andi $at, $at, 0x8
    srl $at, $at, 3 # shift 0x8 to be 0x1
    # toggle the last bit of the tile index 
    # if this is the second tile
    xor a0, a0, $at

    # if the sprite is flipped the bottom tile
    # becomes the top tile and top becomes
    # the bottom
    lbu $at, SPRITE_FLAGS(t0)
    andi $at, $at, SPRITE_FLAGS_FLIP_Y
    srl $at, $at, 6
    xor a0, a0, $at

loadSpriteTileCheckVRAMBank:
    # convert tile index into a tile offset
    sll a0, a0, 4 

    # determine if the second tile bank should be used
    lbu $at, SPRITE_FLAGS(t0)
    andi $at, $at, SPRITE_FLAGS_VRAM_BANK
    srl $at, $at, 3

    # only use second bank on gameboy color
    and $at, $at, t3 

    # convert flag to an offset into the second bank
    sll $at, $at, 13

    # potentially offset the tile index into the second bank
    add a0, a0, $at

    # load the tile into the cache
    jal requestTile
    nop
    j loadNextSpriteTile
    addi t0, t0, SPRITE_SIZE

loadSpriteTilesFinish:
    lw return, 0($sp)
    jr return
    addi $sp, $sp, 4

###############################################
# draws the visible sprites to the screen
#
drawSpritesV:
    # load sprites pointer
    li(t0, sprites)
    # load tile pointer
    lhu t1, currentTile(zero)
    # load current y
    lbu t2, (ppuTask + PPUTask_ly)(zero)

    # create a zero vector
    vxor $v31, $v31, $v31
    # check for gameboy color
    lhu t9, (ppuTask + PPUTask_flags)(zero)
    # load lsb mask
    lsv $v5[0], lsbBitMask(zero)
    # mask out gbc bit
    andi t9, t9, PPU_TASK_FLAGS_COLOR
    # load msb mask
    lsv $v6[0], msbBitMask(zero)

drawNextSpriteV:
    lbu t3, SPRITE_Y(t0)
    # check for null sprite
    beq t3, zero, drawSpritesVFinish
    addi t3, t3, SPRITE_SHIFT_Y
    # calculate relative sprite line
    sub t3, t2, t3
    # wrap y value for 16px high sprites
    andi t3, t3, 0x7

    lbu t4, SPRITE_FLAGS(t0)
    andi $at, t4, SPRITE_FLAGS_FLIP_Y

    beq $at, zero, drawSpriteVCheckPallete
    li($at, 7)
    # flip y value
    sub t3, $at, t3
drawSpriteVCheckPallete:
    beq t9, zero, drawSpriteVDMGPallete
    andi t8, t4, SPRITE_FLAGS_GBC_PALETTE
    j drawSpriteVCheckPriority
    sll t8, t8, 10
drawSpriteVDMGPallete:
    andi t8, t4, SPRITE_FLAGS_DMA_PALETTE
    sll t8, t8, 6
drawSpriteVCheckPriority:
    # convert line to pointer offset
    sll t3, t3, 1 
    # offset into current tile
    add t3, t3, t1

    # load pixel row into v0
    lsv $v0[0], 0(t3)

    # load the tile flip offset
    andi t5, t4, SPRITE_FLAGS_FLIP_X

    # retrieve multiply lsb bits for each pixel
    lqv $v1[0], lsbBitMultiply(t5)
    # retrive multiply msb bits for each pixel
    lqv $v3[0], msbBitMultiply(t5)

    # load sprite x
    lbu t7, SPRITE_X(t0)
    # shift least significant bits into place
    vmudm $v2, $v1, $v0[0]
    # offset sprite x by 8
    addi t7, t7, SPRITE_SHIFT_X + scanline
    # shift most siginificat bits into place
    vmudn $v4, $v3, $v0[0]

    # mask lsb
    vand $v2, $v2, $v5[0]

    # load previous pixel value
    lpv $v10[0], 0(t7)

    # read sprite priority flag
    andi $at, t4, SPRITE_FLAGS_PRIO
    # shift lsbs once
    vadd $v2, $v2, $v2

    # shift priority flag into upper bit
    sll $at, $at, 8
    # mask msb
    vand $v4, $v4, $v6[0]

    # combine priority flag with pallete offset
    or t8, t8, $at
    # shift lsbs once
    vadd $v2, $v2, $v2

    # offset pallete index into obj section
    addi t8, t8, (OBJ_PALETTE_INDEX_START << 8)
    # combine lsb and msb
    vor $v2, $v2, $v4

    # transfer pallete offset into a vector register
    mtc2 t8, $v7[0]
    # add pallete
    vadd $v9, $v2, $v7[0]

    # check if there is already a pixel value
    veq $v30, $v10, $v31
    # use existing value if it is present
    vmrg $v9, $v9, $v10

    # check if pixel value should be written
    veq $v30, $v2, $v31
    # determine the value to write
    vmrg $v2, $v10, $v9

    # write out sprite
    spv $v2[0], 0(t7)

    addi t0, t0, SPRITE_SIZE
    j drawNextSpriteV
    addi t1, t1, GB_TILE_SIZE

drawSpritesVFinish:
    jr return
    sh t1, currentTile(zero)