
# entry point
ppuMain: 
    # initialize stack pointer
    ori $sp, zero, stackEnd + STACK_SIZE

    # load ucode_data
    ori a0, zero, 0
    lw a1, osTask_ucode_data($0)
    lw a2, osTask_ucode_data_size($0)
    addi a2, a2, -1
    jal DMAproc
    ori a3, zero, 0

    li(s2, 0)

loadSprites:
    mfc0 $at, SP_STATUS
    andi $at, $at, MODE_2_FLAG
    beq $at, zero, loadSprites

    li(a0, sprites) # DMEM target
    lw a1, (ppuTask + PPUTask_memorySource)(zero)
    addi a1, a1, Memory_misc_sprites # RAM source
    li(a2, SPRITE_SIZE * SPRITE_MAX_COUNT - 1) # len
    jal DMAproc
    li(a3, 0) # is read

    jal DMAWait
    nop

    jal sortSprites
    ori a0, s2, 0
    
    # increment current y
    addi s2, s2, 1

loadLine:
    # busy loop to wait for cpu to signal mode 3
    mfc0 $at, SP_STATUS
    andi $at, $at, MODE_3_FLAG
    beq $at, zero, loadLine

    # load PPUTask
    ori a0, zero, ppuTask
    lw a1, osTask_data_ptr($0)
    li(a2, PPUTask_sizeof-1)
    jal DMAproc
    ori a3, zero, 0

    # wait for PPUTask to finish loading
    jal DMAWait
    # note delay slot
    nop
    
    # get the tilemap values and attibutes
    li(a2, LCDC_BG_TILE_MAP)

    # get the current line
    lbu a1, (ppuTask + PPUTask_ly)(zero)
    # get the current screen offset
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    add a1, a1, $at # relative offset
    andi a1, a1, 0xFF # wrap to 256 pixels and mask to tile

    jal precacheLine
    ori a0, zero, tilemap # dma target

    # reset the tile cache
    li($at, tilemapTileCache)
    sh $at, currentTile(zero)

    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_OBJ_ENABLE
    bne $at, zero, checkForSprites
    nop
    # remove sprites if disabled
    sw zero, sprites(zero)

checkForSprites:
    jal loadSpriteTiles
    nop

checkForWindow:
    # window x position
    li(s0, GB_SCREEN_WD)

    # check if the window is enabled
    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_WIN_E
    beq $at, zero, precacheTilemap

    # check if the window is below the current row
    lbu t0, (ppuTask + PPUTask_ly)(zero)
    lbu t1, (ppuTask + PPUTask_wy)(zero)
    sub $at, t1, t0
    bgtz $at, precacheTilemap

    # check if window x is less than the screen width
    lbu t0, (ppuTask + PPUTask_wx)(zero)
    addi t0, t0, -WINDOW_X_OFFSET
    addi $at, t0, -GB_SCREEN_WD
    bgez $at, precacheTilemap

    lhu s1, currentWindowY(zero) # load current window y (note delay slot)

    # TODO handle negative window position
    ori s0, t0, 0 # set new window position

    addi $at, s1, 1 # increment and save current window y
    sh $at, currentWindowY(zero)

    li(a2, LCDC_WIN_TILE_MAP) # flag for selecting background source
    ori a1, s1, 0 # y line of row
    jal precacheLine
    ori a0, zero, window # dma target

precacheTilemap:
    #check if tilemap is visible
    blez s0, precacheWindow 

    lbu $at, (ppuTask + PPUTask_scx)(zero)
    srl a1, $at, 3
    add a2, $at, s0 # end pixel row at window position
    addi a2, a2, -1
    srl a2, a2, 3
    # calculate number of tiles
    sub a2, a2, a1 
    addi a2, a2, 1

    lbu $at, (ppuTask + PPUTask_lcdc)(zero)
    andi $at, $at, LCDC_BG_ENABLE
    bne $at, zero, precacheTilemap_enabled
    nop
    jal precacheBlankTiles
    nop
    j precacheWindow_check
    nop

    # get the tiles needed to render the current row
precacheTilemap_enabled:
    jal precacheTiles
    li(a0, tilemap)

precacheWindow_check:
    # check if the window is visible
    li(a2, GB_SCREEN_WD)
    beq s0, a2, beginDrawingRow

precacheWindow:
    # calculate the number of tiles to render
    li(a2, GB_SCREEN_WD-1)
    sub a2, a2, s0 
    srl a2, a2, 3
    addi a2, a2, 1
    # window always starts with the first tile
    li(a1, 0)
    jal precacheTiles
    li(a0, window)

beginDrawingRow:
    # clear the MODE_3_FLAG bit
    li($at, MODE_3_FLAG_CLR | MODE_2_FLAG_CLR)
    mtc0 $at, SP_STATUS

    # reset the tile cache
    li($at, tilemapTileCache)
    sh $at, currentTile(zero)

    jal drawSprites
    nop

    # check if tilemap is visible
    blez s0, drawWindow

    # a0 pixel x
    # a1 pixel count
    # a2 sprite y
    # a3 src x
    li(a0, 0) # pixel x
    ori a1, s0, 0 # pixel count
    
    lbu a2, (ppuTask + PPUTask_ly)(zero)
    lbu $at, (ppuTask + PPUTask_scy)(zero)
    sub a2, a2, $at
    andi a2, a2, 0x7 # sprite y pos

    lbu a3, (ppuTask + PPUTask_scx)(zero)
    li($at, 8)
    sub a3, $at, a3
    jal copyTileLine
    andi a3, a3, 0x7 # sprite x pos

drawWindow:
    ori a0, s0, 0 # pixel x
    li(a1, GB_SCREEN_WD)
    sub a1, a1, a0 # pixel count
    blez a1, writeOutPixels # check if the window is visible
    andi a2, s1, 0x7 # sprite y pos
    jal copyTileLine
    ori a3, zero, 0 # sprite x pos

writeOutPixels:
    jal writeScanline
    nop

    # reset the scanline buffer
    ori a0, zero, scanline
    lw a1, osTask_ucode_data($0)
    addi a1, a1, scanline
    ori a2, GB_SCREEN_WD-1
    jal DMAproc
    ori a3, zero, 0

    # check if this is the last line
    lbu $at, (ppuTask + PPUTask_ly)(zero)
    addi $at, $at, -(GB_SCREEN_HT - 1)

    # render the next line if there is more data to render
    bne $at, zero, loadSprites
    nop

    break