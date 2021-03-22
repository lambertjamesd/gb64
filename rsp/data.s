
.align 16
ppuTask:
    .space PPUTask_sizeof

.align 16
scanline:
    .space GB_SCREEN_WD

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