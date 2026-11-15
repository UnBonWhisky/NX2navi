// Graphics tiers (graphics_tier in nx2navi.ini, NX2NAVI_GRAPHICS_TIER; cfg.c "graphics tiers"): five sets of defaults
// for the display settings, picked from the card's video memory. Header only, so that tests/gfx_tier_check.c checks
// the choice and the table without a device.
#pragma once
#include <stdint.h>

enum { GFX_TIER_MINIMUM, GFX_TIER_LOW, GFX_TIER_BALANCED, GFX_TIER_HIGH, GFX_TIER_ULTRA, GFX_TIER_N };

static const char *const gfx_tier_names[GFX_TIER_N] = {"minimum", "low", "balanced", "high", "ultra"};

// The cards each tier is for: up to 4, 6, 8 and 12 GB of video memory, and more. mb is the card's dedicated video
// memory in MiB, or with vram_emulate_mb the budget the run emulates; the edges lie between the sizes cards come in, so
// that either figure lands in the same tier (a 6 GB card reports 6,144 MiB and a budget of 4,700 to 5,500 MiB; an 8 GB
// card 8,192 and about 7,200; a 12 GB card 12,288 and 10,200 to 11,400; a 16 GB card 16,384 and about 14,500).
#define GFX_TIER_MINIMUM_MB 4608
#define GFX_TIER_LOW_MB 6656
#define GFX_TIER_BALANCED_MB 8704
#define GFX_TIER_HIGH_MB 12800

// The tier for mb MiB of video memory (0: not known) on an integrated GPU or not; -1 when nothing is known, which leaves
// the table's defaults. An integrated GPU, whose video memory is a share of the system's, gets the minimum tier.
static inline int gfx_tier_pick(int64_t mb, int integrated) {
    if (integrated) return GFX_TIER_MINIMUM;
    if (mb <= 0) return -1;
    return mb <= GFX_TIER_MINIMUM_MB ? GFX_TIER_MINIMUM : mb <= GFX_TIER_LOW_MB ? GFX_TIER_LOW
         : mb <= GFX_TIER_BALANCED_MB ? GFX_TIER_BALANCED : mb <= GFX_TIER_HIGH_MB ? GFX_TIER_HIGH : GFX_TIER_ULTRA;
}

// The tier from the setting (auto, off or a tier's name; NULL or "" for auto): the tier's index, -1 for off, -2 for
// auto (pick from the video memory), -3 for a value that is none of those.
static inline int gfx_tier_setting(const char *v) {
    if (!v || !*v || !__builtin_strcmp(v, "auto")) return -2;
    if (!__builtin_strcmp(v, "off")) return -1;
    for (int i = 0; i < GFX_TIER_N; i++)
        if (!__builtin_strcmp(v, gfx_tier_names[i])) return i;
    return -3;
}

// Each tier's values, below what the player sets and above the settings table. The 3D resolution is the scene's share
// of the game resolution without DLSS or FSR 3.1 (scene_share: 0.75 is the game's own most, 1 the menu's Native) and,
// with one of them on, the mode nearest that share (dlss: performance renders half, quality two thirds, dlaa all); the
// tier never turns either on or off. Sharpness is the FSR 1 sharpening of the scaling to the window (sharpness). The
// console stays docked in every tier: with operation_mode = handheld the game draws its handheld picture (1920x1080)
// into the top-left three quarters of the 2560x1440 frame and the rest stays black (runs t-4000 and t-6000,
// 2026-11-15), so the handheld profile of the minimum and low tiers is left out until that is fixed. Texture detail:
// the levels texture_detail = auto gives such cards (vk_tex_detail.h); GFX_TIER_HIGH_DETAIL says what the high tier
// gets and why.
// The high tier keeps reduced texture detail. Full went past a 12 GB card's budget on the 13-minute route (Hyrule Field,
// Kakariko, the Deku Tree, the Market, Kokiri Forest) with the high tier's other values, on an RTX 4070 (2026-11-15,
// runs hr-full and hr-red): full peaked at 11,142 MiB in use against a 10,199 MiB budget, 338 frames over 50 ms and 23
// over 100, 44.2 fps; reduced peaked at 9,145 MiB of 11,391, 55 and 16, 48.4 fps.
#define GFX_TIER_HIGH_DETAIL "reduced"
typedef struct { const char *key, *value[GFX_TIER_N]; } GfxTierRow;
static const GfxTierRow gfx_tier_rows[] = {
    {"SCENE_SHARE", {"0.5", "0.75", "1", "1", "1"}},
    {"DLSS", {"performance", "quality", "dlaa", "dlaa", "dlaa"}},
    {"SHARPNESS", {"30", "30", "30", "40", "50"}},
    {"TEXTURE_DETAIL", {"low", "low", "reduced", GFX_TIER_HIGH_DETAIL, "full"}},
};
