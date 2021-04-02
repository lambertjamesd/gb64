
###############################################
# Procedure to ensure the requested tile is
# loaded into DRAM
# Registers:
#   a0 the offset of the tile into struct GraphicsMemory
#
#   v0 pointer to the tile in DRAM
requestTile:
    # load the result, the pointer to the
    # current tile
    lhu v0, currentTile(zero)
    # retrieve the current tile ID
    addi v1, v0, -tilemapTileCache
    # convert 16 byte per tile offset to a 2 byte index
    srl v1, v1, 3
    # check if the tile id matches
    lhu $at, tilemapTileCacheInfo(v1)
    beq $at, a0, nextTile # bail out early 
    addi $at, v0, GB_TILE_SIZE # increment tile pointer
    sh $at, currentTile(zero) # save tile pointer

    sh a0, tilemapTileCacheInfo(v1) # save new id

    lw a1, (ppuTask + PPUTask_graphicsSource)(zero)
    add a1, a1, a0 # load source of tile from main ram
    ori a0, v0, 0 # set the target in DMEM
    ori a2, zero, GB_TILE_SIZE - 1 # size of dma copy
    j DMAproc # copy tile into result
    ori a3, zero, 0 # mark as read

nextTile:
    jr $ra
    sh $at, currentTile(zero) # save tile pointer

blankTile:
    # retrieve current tile
    lhu v0, currentTile(zero)

    # set to blank tile
    sw zero, 0(v0)
    sw zero, 4(v0)
    sw zero, 8(v0)
    sw zero, 12(v0)

    # calculate index for cache info
    addi v1, v0, -tilemapTileCache
    srl v1, v1, 3
    li($at, -1)
    # set tile id to -1
    sh $at, tilemapTileCacheInfo(v1)

    addi $at, v0, GB_TILE_SIZE # increment tile pointer
    jr $ra
    sh $at, currentTile(zero) # save tile pointer


###############################################
# Loads the tilemap row into memory
# a0 - target dma target
# a1 - y line of row
# a2 - tile map flag

precacheLine:
    addi $sp, $sp, -4
    sw return, 0($sp)

    srl a1, a1, 3 # divde by 8 (8 pixels per tile)
    sll a1, a1, 5 # multiply by 32 (32 tiles per row)
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    and $at, $at, a2 # determine which tile map to use
    slt $at, zero, $at
    # convert to a tilemap offset
    sll $at, $at, 10
    # store the offset into vram memory
    add a1, $at, a1
    lw $at, (ppuTask + PPUTask_graphicsSource)(zero)
    addi $at, $at, GraphicsMemory_tilemap0
    add a1, $at, a1 # calculate final ram address for tilemap row
    
    ori a2, zero, GB_TILEMAP_W - 1 # dma length
    jal DMAproc # read tilemap row
    ori a3, zero, 0 # is read

    # check for gameboy color
    lhu $at, (ppuTask + PPUTask_flags)(zero)
    andi $at, $at, PPU_TASK_FLAGS_COLOR
    beq $at, zero, skipTilemapInfo
    # note delay slot
    addi a1, a1, GraphicsMemory_tilemapOffset # offset source from ram

    jal DMAproc
    addi a0, a0, (tilemapAttrs-tilemap) # offset target into DMEM

skipTilemapInfo:

    lw return, 0($sp)
    j DMAWait
    addi $sp, $sp, 4

###############################################
# a2 - number of sprites to load
precacheBlankTiles:
    addi $sp, $sp, -4
    sw return, 0($sp)
precacheBlankTiles_next:
    beq a2, zero, precacheBlankTilesFinish
    addi a2, a2, -1
    jal blankTile
    li(return, precacheBlankTiles_next)
precacheBlankTilesFinish:
    lw return, 0($sp)
    jr return
    addi $sp, $sp, 4

###############################################
# Loads the tilemap row into memory
#
# a0 - address of tile source
# a1 - offset into tile source
# a2 - number of sprites to load

precacheTiles:
    addi $sp, $sp, -4
    sw return, 0($sp)

    ori s5, a0, 0
    ori s6, a1, 0
    ori s7, a2, 0

    lhu t0, currentTileAttr(zero)

precacheNextTile:
    beq s7, zero, precacheTilesFinish
    add a1, s5, s6 # get the tile address
    lbu a0, 0(a1) # load the tile index

    # 
    # calculate which tile range to use
    #
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    # determine which tile offset to use
    andi $at, $at, LCDC_BG_TILE_DATA 
    sll $at, $at, 3 # if set, LCDC_BG_TILE_DATA this values becomes 0x80
    # a when LCDC_BG_TILE_DATA is set
    # it should select the lower range
    xori $at, $at, 0x80 
    # selectes betweeen the tile range 8000-8FFF and 8800-97ff
    add a0, a0, $at
    andi a0, a0, 0xFF
    add a0, a0, $at

    sll a0, a0, 4 # convert the tile index to a relative tile pointer

    #
    # calculate which tile bank to use
    #
    lbu $at, (tilemapAttrs - tilemap)(a1)
    sb $at, 0(t0) # store current tile attribute into cache
    andi $at, $at, TILE_ATTR_VRAM_BANK
    sll $at, $at, 10 # converts 0x08 flag to 0x2000 offset
    add a0, a0, $at

    jal requestTile
    addi s6, s6, 1
    andi s6, s6, 0x1f # wrap to 32 tiles
    addi t0, t0, 1 # next tile attr
    j precacheNextTile
    addi s7, s7, -1

precacheTilesFinish:
    sh t0, currentTileAttr(zero)

    lw return, 0($sp)

    jr return
    addi $sp, $sp, 4

###############################################
# Tiles To Scanline
# a0 pixel x
# a1 pixel count
# a2 sprite y
# a3 src x

copyTileLineV:
    # align to tile
    add a1, a1, a3
    sub a0, a0, a3

    # create a zero vector
    vxor $v31, $v31, $v31

    # load mask for least significat bits
    lsv $v5[0], lsbBitMask(zero)
    
    # load mask for most significat bits
    lsv $v6[0], msbBitMask(zero)
    
    addi a0, a0, scanline

    # store end of tilemap scanline
    # used to prevent the tilemap
    # from overwriting the window
    add t0, a0, a1
    lw t3, 0(t0)
    lw t2, 4(t0)
    sw t3, overscanBuffer(zero)
    sw t2, (overscanBuffer + 4)(zero)

    # convert y to pixel offset 
    # (2 bytes per row of pixels in a tile)
    sll a2, a2, 1

    # load pointer into tile cache
    lhu t0, currentTile(zero)
    # load pointer into tile attribute cache
    lhu t1, currentTileAttr(zero)

    # check if gameboy color for global priority bit
    lhu t8, (ppuTask + PPUTask_flags)(zero)
    andi t8, t8, PPU_TASK_FLAGS_COLOR

    # check the global priority bit
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_BG_ENABLE
    xori $at, $at, LCDC_BG_ENABLE
    and t8, t8, $at

copyTileLineV_nextTile:
    # load tile attributes
    lbu t4, 0(t1)

    andi $at, t4, TILE_ATTR_V_FLIP
    # calculate pixel row address
    add t3, t0, a2
    beq $at, zero, copyTileLineV_skipVFlip
    nop

    addi t3, t0, 14
    sub t3, t3, a2

copyTileLineV_skipVFlip:
    # load pixel row into v0
    lsv $v0[0], 0(t3)

    # load the tile flip offset
    andi t2, t4, TILE_ATTR_H_FLIP

    # retrieve multiply lsb bits for each pixel
    lqv $v1[0], lsbBitMultiply(t2)
    # mask out tile pallete
    andi t5, t4, TILE_ATTR_PALETTE

    # retrive multiply msb bits for each pixel
    lqv $v3[0], msbBitMultiply(t2)
    # convert pallete index into a byte offset
    # with shift by 8 bits to upper byte
    sll t5, t5, 10 

    # shift least significant bits into place
    vmudm $v2, $v1, $v0[0]
    # move pallete into vector registers
    mtc2 t5, $v7[0]

    # shift most siginificat bits into place
    vmudn $v4, $v3, $v0[0]
    # load mask to remove priority bit
    li(t6, 0x7f00)
    
    # mask lsb
    vand $v2, $v2, $v5[0]
    # transfer mask to vector register
    mtc2 t6, $v10[0]

    # mask msb
    vand $v4, $v4, $v6[0]
    # load mask to read priority bit
    li(t7, 0x8000)

    # shift one more bit
    vadd $v2, $v2, $v2
    # move priority mask to a vector register
    mtc2 t7, $v11[0]

    # load the previous pixels
    lpv $v8[0], 0(a0)

    # shift one more bit
    vadd $v2, $v2, $v2

    # mask out priority bit and store obj color value
    vand $v10, $v8, $v10[0]

    # combine lsb and msb
    vor $v2, $v2, $v4

    # mask out color information and store priority bit
    vand $v11, $v8, $v11[0]

    # mix in pallete color
    vadd $v3, $v2, $v7[0]

    #
    # BG vs Sprite truth table
    #
    # OB  | BG  | PRI | Use BG
    # ------------------------
    #  0  |  0  |  0  |  1 
    #  0  |  0  |  1  |  1 
    #  0  |  1  |  0  |  1 
    #  0  |  1  |  1  |  1 
    #  1  |  0  |  0  |  0 
    #  1  |  0  |  1  |  0 
    #  1  |  1  |  0  |  0 
    #  1  |  1  |  1  |  1 
    #
    # Use BG = !OB | BG && PRI
    # PRI = OB_PRI | TILE_PRI
    #

    # check if existing pixel is 0
    veq $v30, $v10, $v31
    # check if the tile has priority
    andi $at, t4, TILE_ATTR_PRIORITY
    # store background into obj slot if obj is 0
    vmrg $v10, $v3, $v10
    # check if the global priority bit is set
    beq t8, zero, skipSpritePriority
    # move priority bit into a vector register
    mtc2 $at, $v9[0]

    # move sprite into background pixel
    vmrg $v3, $v10, $v10
skipSpritePriority:
    # check if background is 0
    veq $v30, $v2, $v31
    # calculate OB_PRI | TILE_PRI
    vor $v11, $v11, $v9[0]
    # store obj into background if background is 0
    vmrg $v3, $v10, $v3

    # check if priority is 0
    veq $v30, $v31, $v11
    # decrement pixels remaning
    addi a1, a1, -GB_TILE_WIDTH
    # store obj into background if priority is 0
    vmrg $v3, $v10, $v3
    # increment to next tile attr
    addi t1, t1, 1

    # store the pixels
    spv $v3[0], 0(a0)
    # increment to next tile
    addi t0, t0, GB_TILE_SIZE

    bgtz a1, copyTileLineV_nextTile
    # increment current x
    addi a0, a0, GB_TILE_WIDTH

copyTileLineV_finish:

    # restore end of tilemap scanline
    # used to prevent the tilemap
    # from overwriting the window
    add $at, a0, a1
    lw t3, overscanBuffer(zero)
    lw t2, (overscanBuffer + 4)(zero)
    sw t3, 0($at)
    sw t2, 4($at)
    
    # save current tile attribute before exiting
    sh t1, currentTileAttr(zero)
    jr return
    # save current tile before exiting
    sh t0, currentTile(zero)

