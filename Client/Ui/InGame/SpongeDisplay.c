#include <stdio.h>
#include <string.h>

#include <Client/Game.h>
#include <Client/Renderer/Renderer.h>
#include <Client/Ui/Engine.h>
#include <Client/Ui/Ui.h>

#include <Shared/StaticData.h>

static uint8_t prev_show = 0;
static float sponge_display_lerp = 0;

static void sponge_display_animate(struct rr_ui_element *this,
                                    struct rr_game *game)
{
    uint8_t show = this->should_show(this, game);
    if (show && !prev_show)
        this->animation = 0.3f;
    prev_show = show;
    if (this->animation > 0)
        this->animation -= 0.05f;
}

static uint8_t sponge_display_should_show(struct rr_ui_element *this,
                                           struct rr_game *game)
{
    if (!game->simulation_ready || game->cache.hide_ui ||
        !game->player_info || game->player_info->flower_id == RR_NULL_ENTITY)
        return 0;
    for (uint32_t i = 0; i < game->player_info->slot_count; ++i)
        if (game->player_info->slots[i].id == rr_petal_id_sponge)
            return 1;
    return 0;
}

static void sponge_display_on_render(struct rr_ui_element *this,
                                      struct rr_game *game)
{
    int best_idx = -1;
    uint8_t best_rarity = 0;
    for (uint32_t i = 0; i < game->player_info->slot_count; ++i)
    {
        if (game->player_info->slots[i].id == rr_petal_id_sponge &&
            game->player_info->slots[i].rarity >= best_rarity)
        {
            best_rarity = game->player_info->slots[i].rarity;
            best_idx = i;
        }
    }
    if (best_idx < 0) return;

    struct rr_renderer *renderer = game->renderer;
    char num[24];
    float target = (float)game->player_info->slots[best_idx].client_health *
        game->player_info->slots[best_idx].client_health * 100.0f;
    if (target > sponge_display_lerp)
        sponge_display_lerp += (target - sponge_display_lerp) * 0.1f;
    else
        sponge_display_lerp += (target - sponge_display_lerp) * 0.05f;
    float dpt = sponge_display_lerp;
    if (dpt >= 1000000.0f)
    {
        float m = dpt / 1000000.0f;
        if (m >= 10.0f)
            snprintf(num, 24, "%.0fM", m);
        else
            snprintf(num, 24, "%.1fM", m);
    }
    else if (dpt >= 1000.0f)
    {
        float k = dpt / 1000.0f;
        if (k >= 100.0f)
            snprintf(num, 24, "%.0fK", k);
        else
            snprintf(num, 24, "%.1fK", k);
    }
    else
        snprintf(num, 24, "%.0f", dpt);
    float w = (strlen("Sponge:") + strlen(num)) * 6 + 20;
    if (w < 80) w = 80;
    float hw = w / 2;
    rr_renderer_set_global_alpha(renderer, 0.6f);
    rr_renderer_set_fill(renderer, 0xff000000);
    rr_renderer_begin_path(renderer);
    // KUNG: toa do khung (-hw, y, w, h, radius)
    rr_renderer_round_rect(renderer, -50, -63, 100, 15, 2);
    rr_renderer_fill(renderer);
    rr_renderer_set_global_alpha(renderer, 1.0f);
    rr_renderer_set_text_baseline(renderer, 1);
    rr_renderer_set_text_size(renderer, 11);
    rr_renderer_set_line_width(renderer, 11 * 0.12);
    // KUNG: toa do chu "Sponge:" (x, y), align=2 (phai)
    rr_renderer_set_text_align(renderer, 2);
    rr_renderer_set_fill(renderer, 0xffC9E033);
    rr_renderer_set_stroke(renderer, 0xff222222);
    rr_renderer_stroke_text(renderer, "Sponge:", 0, -55);
    rr_renderer_fill_text(renderer, "Sponge:", 0, -55);
    // KUNG: toa do so (x, y), align=0 (trai)
    rr_renderer_set_text_align(renderer, 0);
    rr_renderer_set_fill(renderer, 0xffF34242);
    rr_renderer_set_stroke(renderer, 0xff222222);
    rr_renderer_stroke_text(renderer, num, 0, -55);
    rr_renderer_fill_text(renderer, num, 0, -55);
}

struct rr_ui_element *rr_ui_sponge_display_init(void)
{
    struct rr_ui_element *this = rr_ui_element_init();
    this->should_show = sponge_display_should_show;
    this->on_render = sponge_display_on_render;
    this->animate = sponge_display_animate;
    this->abs_width = 150;
    this->abs_height = 16;
    rr_ui_v_pad(rr_ui_set_justify(this, 0, 1), 100);
    return this;
}
