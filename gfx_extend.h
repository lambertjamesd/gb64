
#ifndef __GFX_EXTEND_H__
#define __GFX_EXTEND_H__

#include <ultra64.h>

union OtherModes {    
    struct {
        u32 word0;
        u32 word1;
    };

    struct {
        u64 command:8;

        u64 atomic_prim:1;
        u64 reserved0:1;
        u64 cycle_type:2;
        u64 persp_tex_en:1;
        u64 detail_tex_en:1;
        u64 sharpen_tex_en:1;
        u64 tex_lod_en:1;

        u64 en_tlut:1;
        u64 tlut_type:1;
        u64 sample_type:1;
        u64 mid_texel:1;
        u64 bi_lerp_0:1;
        u64 bi_lerp_1:1;
        u64 convert_one:1;
        u64 key_en:1;

        u64 rgb_dither_sel:2;
        u64 alpha_dither_sel:2;
        u64 reserved1:4;

        u64 m1a0:2;
        u64 m1a1:2;
        u64 m1b0:2;
        u64 m1b1:2;

        u64 m2a0:2;
        u64 m2a1:2;
        u64 m2b0:2;
        u64 m2b1:2;
        
        u64 reserved2:1;
        u64 force_blend:1;
        u64 alpha_cvg_select:1;
        u64 cvg_times_alpha:1;
        u64 z_mode:2;
        u64 cvg_dest:2;

        u64 color_on_cvg:1;
        u64 image_read_en:1;
        u64 z_update_en:1;
        u64 z_compare_en:1;
        u64 antialias_en:1;
        u64 z_source_sel:1;
        u64 dither_alpha_en:1;
        u64 alpha_compare_en:1;
    };

    long long int	force_structure_alignment;
};

#endif