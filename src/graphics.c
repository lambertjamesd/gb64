

#include <ultra64.h>
#include "graphics.h"
#include "../boot.h"
#include "debug_out.h"
#include "../memory.h"
#include "gameboy.h"
#include "../gfx_extend.h"


Gfx* gCurrentDP;

// For some reason gScreenBuffer is overwritten by 8 bytes without
// this padding
u64 gPadding;

u8 __attribute__((aligned(8))) gScreenBuffer[GB_SCREEN_W * (GB_SCREEN_H + 1)];

Gfx gDPCommands[0x800];

u16 gScreenPalette[MAX_PALLETE_SIZE];

struct RenderBlockInformation gRenderInformation[GB_SCREEN_H];
int gRenderInformationCount;

void prepareGraphicsPallete(struct GraphicsState* state);

void flushDPCommands() {
    osWritebackDCache(gDPCommands, sizeof(gDPCommands));
    IO_WRITE(DPC_END_REG, OS_K0_TO_PHYSICAL(gCurrentDP));
}

#define COPY_SCREEN_STRIP(dl, y, maxY, scale, scaleInv)                     \
    gDPLoadTextureTile(                                                     \
        dl,                                                                 \
        (int)gScreenBuffer + y * GB_SCREEN_W,                        \
        G_IM_FMT_CI, G_IM_SIZ_8b,                                           \
        GB_SCREEN_W, maxY - y,                                              \
        0, 0,                                                               \
        GB_SCREEN_W - 1, (maxY - y) - 1,                                    \
        0,                                                                  \
        G_TX_CLAMP, G_TX_CLAMP,                                             \
        G_TX_NOMASK, G_TX_NOMASK,                                           \
        G_TX_NOLOD, G_TX_NOLOD                                              \
    );                                                                      \
    gDPTextureRectangle(                                                    \
        dl,                                                                 \
        (SCREEN_WD << 1) - (((scale) * GB_SCREEN_W) >> 15),                 \
        (SCREEN_HT << 1) - (((scale) * (GB_SCREEN_H / 2 - y)) >> 14),       \
        (SCREEN_WD << 1) + (((scale) * GB_SCREEN_W) >> 15),                 \
        (SCREEN_HT << 1) - (((scale) * (GB_SCREEN_H / 2 - maxY)) >> 14),    \
        G_TX_RENDERTILE,                                                    \
        0, 0,                                                               \
        (scaleInv >> 6), (scaleInv >> 6)                                    \
    )

int compareSprites(struct Sprite a, struct Sprite b)
{
    return a.x - b.x;
}

void applyGrayscalePallete(struct GraphicsState* state) {
    memCopy(&gScreenPalette[state->palleteWriteIndex], gGameboy.memory.vram.colorPalettes, sizeof(u16) * PALETTE_COUNT);
}

static long gScreenScales[ScreenScaleSettingCount] = {
    0x20000,
    0x28000,
    0x30000,
};

static long gInvScreenScales[ScreenScaleSettingCount] = {
    0x08000,
    0x06666,
    0x05555,
};

void beginScreenDisplayList(struct GameboyGraphicsSettings* settings, void* colorBuffer)
{
    gCurrentDP = gDPCommands;

    gDPPipeSync(gCurrentDP++);
    gDPTileSync(gCurrentDP++);
    gDPLoadSync(gCurrentDP++);
    gDPSetColorImage(gCurrentDP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, OS_K0_TO_PHYSICAL(colorBuffer));
    gDPPipeSync(gCurrentDP++);
    gDPSetScissor(gCurrentDP++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
    gDPSetFillColor(gCurrentDP++, GPACK_RGBA5551(1, 1, 1, 1) << 16 | GPACK_RGBA5551(1, 1, 1, 1));
    gDPSetOtherMode(gCurrentDP++, 0x300000, 0);
    gDPFillRectangle(gCurrentDP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPPipeSync(gCurrentDP++);

    union OtherModes otherModes;
    otherModes.force_structure_alignment = 0;

    otherModes.cycle_type = 0; // 1 Cycle
    otherModes.persp_tex_en = 0;
    otherModes.en_tlut = 1;
    otherModes.tlut_type = 0; // RGBA 16
    otherModes.sample_type = settings->smooth ? 1 : 0;

    otherModes.bi_lerp_0 = 1;
    otherModes.bi_lerp_1 = 1;

    otherModes.m1a0 = 0;
    otherModes.m1a1 = 0;
    otherModes.m1b0 = 3;
    otherModes.m1b1 = 3;
    otherModes.m2a0 = 0;
    otherModes.m2a1 = 0;
    otherModes.m2b0 = 2;
    otherModes.m2b1 = 2;

    otherModes.reserved2 = 0;
    otherModes.force_blend = 1;
    otherModes.alpha_cvg_select = 0;
    otherModes.cvg_times_alpha = 0;

    otherModes.z_mode = 0;
    otherModes.cvg_dest = 0;
    otherModes.color_on_cvg = 0;
    otherModes.image_read_en = 0;
    otherModes.z_update_en = 0;
    otherModes.z_compare_en = 0;
    otherModes.antialias_en = 0;

    gDPSetOtherMode(gCurrentDP++, otherModes.word0, otherModes.word1);

    gDPSetCombineMode(gCurrentDP++, G_CC_BLENDRGBA, G_CC_BLENDRGBA);
    gDPSetPrimColor(gCurrentDP++, 0, 0, 255, 255, 255, 255);

    // wait for DP to be available
    while (IO_READ(DPC_STATUS_REG) & (DPC_STATUS_END_VALID | DPC_STATUS_START_VALID));

    // start dp commands
    IO_WRITE(DPC_STATUS_REG, DPC_CLR_FLUSH | DPC_CLR_FREEZE | DPC_CLR_XBUS_DMEM_DMA);
    IO_WRITE(DPC_START_REG, OS_K0_TO_PHYSICAL(gDPCommands));

    flushDPCommands();
}

void waitForRDP() {
    // set flag for exit early in case
    // screen was turned off while rending the previous frame
    IO_WRITE(SP_STATUS_REG, SP_SET_SIG2);
    // wait for DP to be available
    while (IO_READ(DPC_STATUS_REG) & (DPC_STATUS_END_VALID | DPC_STATUS_START_VALID));
}

void initGraphicsState(
    struct Memory* memory,
    struct GraphicsState* state,
    struct GameboyGraphicsSettings* settings,
    int gbc,
    void* colorBuffer
)
{
    gPalleteDirty = 1;
    state->settings = *settings;
    state->gbc = gbc;
    state->row = 0;
    state->lastRenderedRow = 0;
    state->palleteWriteIndex = 0;

    gRenderInformationCount = 0;

    beginScreenDisplayList(&state->settings, colorBuffer);
    prepareGraphicsPallete(state);
}

void renderScreenBlock(struct GraphicsState* state)
{
    if (state->lastRenderedRow != state->row)
    {
        // Record information about this block
        // so it can be rendered when paused
        gRenderInformation[gRenderInformationCount].row = state->row;
        gRenderInformation[gRenderInformationCount].palleteWriteIndex = state->palleteWriteIndex - PALETTE_COUNT;
        ++gRenderInformationCount;

        COPY_SCREEN_STRIP(
            gCurrentDP++, 
            state->lastRenderedRow, 
            state->row, 
            gScreenScales[state->settings.scaleSetting], 
            gInvScreenScales[state->settings.scaleSetting]
        );
        flushDPCommands();
        state->lastRenderedRow = state->row;
    }
}

void prepareGraphicsPallete(struct GraphicsState* state)
{
    if (gPalleteDirty)
    {
        gPalleteDirty = 0;

        if (state->palleteWriteIndex >= MAX_PALLETE_SIZE)
        {
            return;
        }

        if (state->gbc)
        {
            int i;

            for (i = 0; i < PALETTE_COUNT; ++i)
            {
                gScreenPalette[i + state->palleteWriteIndex] = GBC_TO_N64_COLOR(gGameboy.memory.vram.colorPalettes[i]);
            }
        }
        else
        {
            applyGrayscalePallete(state);
        }

        renderScreenBlock(state);
        gDPLoadTLUT_pal256(gCurrentDP++, gScreenPalette + state->palleteWriteIndex);
        flushDPCommands();

        state->palleteWriteIndex += PALETTE_COUNT;
    }
}

void finishScreen(struct GraphicsState* state)
{
    state->row = GB_SCREEN_H;
    renderScreenBlock(state);

    gDPPipeSync(gCurrentDP++);
    gDPTileSync(gCurrentDP++);
    gDPLoadSync(gCurrentDP++);
    gDPFullSync(gCurrentDP++);
    flushDPCommands();
}

void rerenderLastFrame(struct GameboyGraphicsSettings* settings, void* colorBuffer)
{
    beginScreenDisplayList(settings, colorBuffer);

    int i;
    u16 palleteWriteIndex = 0;
    u16 lastRenderedRow = 0;

    gDPLoadTLUT_pal256(gCurrentDP++, gScreenPalette);

    for (i = 0; i < gRenderInformationCount; ++i) {
        struct RenderBlockInformation* info = &gRenderInformation[i];
        if (info->palleteWriteIndex != palleteWriteIndex) {
            gDPLoadTLUT_pal256(gCurrentDP++, gScreenPalette + info->palleteWriteIndex);
            palleteWriteIndex = info->palleteWriteIndex;
        }
        
        COPY_SCREEN_STRIP(
            gCurrentDP++, 
            lastRenderedRow, 
            info->row, 
            gScreenScales[settings->scaleSetting], 
            gInvScreenScales[settings->scaleSetting]
        );

        lastRenderedRow = info->row;
    }

    gDPPipeSync(gCurrentDP++);
    gDPTileSync(gCurrentDP++);
    gDPLoadSync(gCurrentDP++);
    gDPFullSync(gCurrentDP++);
    flushDPCommands();

    waitForRDP();
}

int palleteUsedCount() {
    if (gRenderInformationCount > 0) {
        return gRenderInformation[gRenderInformationCount - 1].palleteWriteIndex / PALETTE_COUNT + 1;
    }

    return 0;
}