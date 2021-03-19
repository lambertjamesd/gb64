
.align 8
ppuTask:
    .space PPUTask_sizeof

.align 8
scanline:
    .space GB_SCREEN_WD

.align 8
tilemap:
    .space GB_TILEMAP_W
tilemapAttrs:
    .space GB_TILEMAP_W

.align 8
window:
    .space GB_TILEMAP_W
windowAttrs:
    .space GB_TILEMAP_W

.align 8
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
currentWindowY:
    .half 0

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