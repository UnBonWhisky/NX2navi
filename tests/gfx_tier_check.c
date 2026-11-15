// runtime/gfx_tier.h: the graphics tier graphics_tier = auto picks (cfg.c "graphics tiers") for the cards' video memory
// and for the budgets vram_emulate_mb takes, the setting's words, and the tiers' values. Built and run by
// tests/gfx_tier_check.py; prints each failed check and exits 1 on any.
#include <stdio.h>
#include <string.h>

#include "gfx_tier.h"

static int fails;

static const char *name(int t) { return t >= 0 && t < GFX_TIER_N ? gfx_tier_names[t] : t == -1 ? "none" : "?"; }

static void expect(long long mb, int igpu, int want, const char *what) {
    int got = gfx_tier_pick(mb, igpu);
    if (got != want) fails++, printf("FAIL: %s (%lld MiB, integrated %d): %s, not %s\n", what, mb, igpu, name(got), name(want));
    else printf("  ok: %-58s %6lld MiB -> %s\n", what, mb, name(got));
}

static void setting(const char *v, int want) {
    int got = gfx_tier_setting(v);
    if (got != want) fails++, printf("FAIL: graphics_tier = '%s' reads %d, not %d\n", v ? v : "(unset)", got, want);
    else printf("  ok: graphics_tier = '%s' reads %d\n", v ? v : "(unset)", got);
}

static const char *value(const char *key, int t) {
    for (size_t i = 0; i < sizeof gfx_tier_rows / sizeof *gfx_tier_rows; i++)
        if (!strcmp(gfx_tier_rows[i].key, key)) return gfx_tier_rows[i].value[t];
    return NULL;
}

static void row(const char *key, const char *const want[GFX_TIER_N]) {
    for (int t = 0; t < GFX_TIER_N; t++) {
        const char *got = value(key, t);
        if (!got || strcmp(got, want[t])) fails++, printf("FAIL: %s on %s is %s, not %s\n", key, name(t), got ? got : "(none)", want[t]);
    }
    printf("  ok: %s per tier\n", key);
}

int main(void) {
    // ---- the cards' own memory (DXGI's dedicated memory)
    expect(2048, 0, GFX_TIER_MINIMUM, "a 2 GB card");
    expect(4096, 0, GFX_TIER_MINIMUM, "a 4 GB card (RTX 3050 Laptop 4 GB)");
    expect(6144, 0, GFX_TIER_LOW, "a 6 GB card (RTX 3050 Laptop 6 GB, RTX 3060 Laptop)");
    expect(8192, 0, GFX_TIER_BALANCED, "an 8 GB card (RTX 4060, RX 7600)");
    expect(10240, 0, GFX_TIER_HIGH, "a 10 GB card (RTX 3080)");
    expect(12282, 0, GFX_TIER_HIGH, "a 12 GB card (RTX 4070, as DXGI reports it)");
    expect(16384, 0, GFX_TIER_ULTRA, "a 16 GB card");
    expect(32607, 0, GFX_TIER_ULTRA, "a 32 GB card (RTX 5090)");
    // ---- budgets (vram_emulate_mb), which land in the same tier as the card they stand for
    expect(4000, 0, GFX_TIER_MINIMUM, "vram_emulate_mb = 4000");
    expect(4730, 0, GFX_TIER_LOW, "a 6 GB laptop card's budget (4,730 MiB)");
    expect(5000, 0, GFX_TIER_LOW, "vram_emulate_mb = 5000");
    expect(6000, 0, GFX_TIER_LOW, "vram_emulate_mb = 6000");
    expect(7365, 0, GFX_TIER_BALANCED, "an 8 GB card's budget (RX 7600, 7,365 MiB)");
    expect(8000, 0, GFX_TIER_BALANCED, "vram_emulate_mb = 8000");
    expect(11432, 0, GFX_TIER_HIGH, "a 12 GB card's budget (RTX 4070, 11,432 MiB)");
    expect(15569, 0, GFX_TIER_ULTRA, "a 16 GB card's budget (RX 6800 XT, 15,569 MiB)");
    // ---- the edges
    expect(GFX_TIER_MINIMUM_MB, 0, GFX_TIER_MINIMUM, "the minimum tier's edge");
    expect(GFX_TIER_MINIMUM_MB + 1, 0, GFX_TIER_LOW, "one MiB over it");
    expect(GFX_TIER_HIGH_MB, 0, GFX_TIER_HIGH, "the high tier's edge");
    expect(GFX_TIER_HIGH_MB + 1, 0, GFX_TIER_ULTRA, "one MiB over it");
    // ---- integrated GPUs: minimum whatever they report; nothing known: no tier
    expect(512, 1, GFX_TIER_MINIMUM, "an integrated GPU with 512 MiB of its own");
    expect(16000, 1, GFX_TIER_MINIMUM, "an integrated GPU with a large share");
    expect(0, 0, -1, "no video memory known");
    // ---- the setting
    setting(NULL, -2), setting("", -2), setting("auto", -2), setting("off", -1);
    setting("minimum", GFX_TIER_MINIMUM), setting("low", GFX_TIER_LOW), setting("balanced", GFX_TIER_BALANCED);
    setting("high", GFX_TIER_HIGH), setting("ultra", GFX_TIER_ULTRA), setting("medium", -3);
    // ---- the values: the 3D resolution and sharpness of the reference table; no tier sets the console mode
    row("SCENE_SHARE", (const char *const[]){"0.5", "0.75", "1", "1", "1"});
    row("DLSS", (const char *const[]){"performance", "quality", "dlaa", "dlaa", "dlaa"});
    row("SHARPNESS", (const char *const[]){"30", "30", "30", "40", "50"});
    row("TEXTURE_DETAIL", (const char *const[]){"low", "low", "reduced", GFX_TIER_HIGH_DETAIL, "full"});
    if (value("OPERATION_MODE", 0)) fails++, printf("FAIL: a tier sets operation_mode (handheld draws a quarter of the frame black)\n");
    printf(fails ? "gfx_tier_check: %d failed\n" : "gfx_tier_check: ok\n", fails);
    return fails != 0;
}
