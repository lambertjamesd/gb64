.align 16
lsbBitMultiply:
    .half 0x0080
    .half 0x0100
    .half 0x0200
    .half 0x0400
    .half 0x0800
    .half 0x1000
    .half 0x2000
    .half 0x4000

.align 16
msbBitMultiply:
    .half 0x0004
    .half 0x0008
    .half 0x0010
    .half 0x0020
    .half 0x0040
    .half 0x0080
    .half 0x0100
    .half 0x0200

.align 16
lsbBitMultiplyFlipX:
    .half 0x4000
    .half 0x2000
    .half 0x1000
    .half 0x0800
    .half 0x0400
    .half 0x0200
    .half 0x0100
    .half 0x0080

.align 16
msbBitMultiplyFlipX:
    .half 0x0200
    .half 0x0100
    .half 0x0080
    .half 0x0040
    .half 0x0020
    .half 0x0010
    .half 0x0008
    .half 0x0004

.align 2
lsbBitMask:
    .half 0x40

.align 2
msbBitMask:
    .half 0x200

.align 16
ppuTask:
    .space PPUTask_sizeof

.align 16
    .space 8 # instead of clipping pixels, extra memeory is used that can be overwitten without issue
scanline:
    .space GB_SCREEN_WD
    .space 8 # instead of clipping pixels, extra memeory is used that can be overwitten without issue

.align 16
tilemap:
    .space GB_TILEMAP_W
tilemapAttrs:
    .space GB_TILEMAP_W

.align 16
window:
    .space GB_TILEMAP_W
windowAttrs:
    .space GB_TILEMAP_W

tileAttrCache:
    .space GB_MAX_VISIBLE_TILES

.align 16
tilemapTileCache:
    .space GB_TILE_SIZE * (GB_MAX_VISIBLE_TILES + GB_MAX_VISIBLE_SPRITES)
tilemapTileCacheInfo:
    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1
    .half -1
    .half -1
    .half -1

    .half -1
    .half -1

currentTile:
    .half tilemapTileCache
currentTileAttr:
    .half tileAttrCache
currentWindowY:
    .half 0

.align 16
sprites:
    .space SPRITE_SIZE * SPRITE_MAX_COUNT

#define STACK_SIZE  64
stackEnd:
    .space STACK_SIZE

startShiftXFlip:
    .byte 7
    .byte 0
startDirXFlip:
    .byte -1
    .byte 1

.align 8
cyclesWaitingForMode2:
    .word 0
cyclesWaitingForMode3:
    .word 0