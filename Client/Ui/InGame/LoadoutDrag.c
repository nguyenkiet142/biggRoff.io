#include "LoadoutDrag.h"

#include <Client/Game.h>
#include <Client/InputData.h>
#include <Client/Renderer/Renderer.h>
#include <Client/Assets/RenderFunctions.h>
#include <Client/Ui/Ui.h>
#include <Client/Ui/Engine.h>

#include <Shared/Component/PlayerInfo.h>
#include <Shared/Entity.h>
#include <Shared/pb.h>

#include <math.h>

#define MAX_SLOT_REGISTRY (RR_MAX_SLOT_COUNT * 2)

static struct slot_entry
{
    uint8_t pos;
    float abs_x, abs_y;
    float width, height;
} slot_registry[MAX_SLOT_REGISTRY];

static uint8_t slot_registry_count = 0;

void rr_ui_drag_register_slot(uint8_t pos, float abs_x, float abs_y,
                              float width, float height)
{
    if (slot_registry_count < MAX_SLOT_REGISTRY)
    {
        slot_registry[slot_registry_count].pos = pos;
        slot_registry[slot_registry_count].abs_x = abs_x;
        slot_registry[slot_registry_count].abs_y = abs_y;
        slot_registry[slot_registry_count].width = width;
        slot_registry[slot_registry_count].height = height;
        ++slot_registry_count;
    }
}

void rr_ui_drag_clear_slot_registry(void)
{
    slot_registry_count = 0;
}

void rr_ui_drag_start_inventory(struct rr_game *game,
                                uint8_t id, uint8_t rarity,
                                float click_x, float click_y)
{
    struct rr_drag_state *d = &game->drag_state;
    d->active = 1;
    d->source_type = RR_DRAG_SOURCE_INVENTORY;
    d->source_id = id;
    d->source_rarity = rarity;
    d->source_slot = 0;
    d->target_slot = 255;
    d->preview_active = 0;
    d->prev_target_slot = 255;
    d->wobble_time = 0;
    d->drag_time = 0;
    d->preview_transition = 0;
    d->click_off_x = click_x;
    d->click_off_y = click_y;
}

void rr_ui_drag_start_slot(struct rr_game *game, uint8_t slot,
                           float click_x, float click_y)
{
    struct rr_drag_state *d = &game->drag_state;
    d->active = 1;
    d->source_type = RR_DRAG_SOURCE_LOADOUT;
    d->source_slot = slot;
    d->source_id = 0;
    d->source_rarity = 0;
    d->target_slot = 255;
    d->preview_active = 0;
    d->prev_target_slot = 255;
    d->wobble_time = 0;
    d->drag_time = 0;
    d->preview_transition = 0;
    d->click_off_x = click_x;
    d->click_off_y = click_y;
}

void rr_ui_drag_cancel(struct rr_game *game)
{
    struct rr_drag_state *d = &game->drag_state;
    d->active = 0;
    d->source_type = RR_DRAG_SOURCE_NONE;
    d->target_slot = 255;
    d->prev_target_slot = 255;
    d->preview_active = 0;
    d->wobble_time = 0;
}

uint8_t rr_ui_drag_is_over_slot(struct rr_game *game,
                                uint8_t slot_pos,
                                float slot_abs_x, float slot_abs_y,
                                float slot_width, float slot_height)
{
    struct rr_drag_state *d = &game->drag_state;
    if (!d->active) return 0;

    float mx = game->input_data->mouse_x;
    float my = game->input_data->mouse_y;
    float half_w = slot_width * game->renderer->scale / 2;
    float half_h = slot_height * game->renderer->scale / 2;

    return fabsf(mx - slot_abs_x) < half_w &&
           fabsf(my - slot_abs_y) < half_h;
}

static uint8_t rr_ui_drag_find_target_slot(struct rr_game *game)
{
    for (uint8_t i = 0; i < slot_registry_count; ++i)
    {
        struct slot_entry *e = &slot_registry[i];
        float mx = game->input_data->mouse_x;
        float my = game->input_data->mouse_y;
        float half_w = e->width * game->renderer->scale / 2;
        float half_h = e->height * game->renderer->scale / 2;
        if (fabsf(mx - e->abs_x) < half_w && fabsf(my - e->abs_y) < half_h)
            return e->pos;
    }
    return 255;
}

void rr_ui_drag_drop_to_slot(struct rr_game *game, uint8_t target_slot)
{
    struct rr_drag_state *d = &game->drag_state;
    if (!d->active) return;

    if (d->source_type == RR_DRAG_SOURCE_INVENTORY)
    {
        uint8_t old_id = game->cache.loadout[target_slot].id;
        uint8_t old_rarity = game->cache.loadout[target_slot].rarity;
        game->cache.loadout[target_slot].id = d->source_id;
        game->cache.loadout[target_slot].rarity = d->source_rarity;

        if (game->simulation_ready && game->player_info)
        {
            uint8_t slot = target_slot % RR_MAX_SLOT_COUNT;
            if (target_slot < RR_MAX_SLOT_COUNT)
            {
                game->player_info->slots[slot].id = d->source_id;
                game->player_info->slots[slot].rarity = d->source_rarity;
            }
            else
            {
                game->player_info->secondary_slots[slot].id = d->source_id;
                game->player_info->secondary_slots[slot].rarity = d->source_rarity;
            }
            if (old_id)
                --game->loadout_counts[old_id][old_rarity];
            ++game->loadout_counts[d->source_id][d->source_rarity];
            rr_game_send_equip_petal(game, target_slot, d->source_id, d->source_rarity);
        }
    }
    else if (d->source_type == RR_DRAG_SOURCE_LOADOUT)
    {
        uint8_t src = d->source_slot;
        uint8_t dst = target_slot;

        if (src == dst)
        {
            rr_ui_drag_cancel(game);
            return;
        }

        struct rr_id_rarity_pair tmp = game->cache.loadout[src];
        game->cache.loadout[src] = game->cache.loadout[dst];
        game->cache.loadout[dst] = tmp;

        if (game->simulation_ready && game->player_info)
        {
            uint8_t inv_a = src >= RR_MAX_SLOT_COUNT ? 1 : 0;
            uint8_t inv_b = dst >= RR_MAX_SLOT_COUNT ? 1 : 0;
            uint8_t slot_a = src % RR_MAX_SLOT_COUNT;
            uint8_t slot_b = dst % RR_MAX_SLOT_COUNT;
            struct rr_component_player_info_petal_slot *a =
                inv_a ? &game->player_info->secondary_slots[slot_a]
                      : &game->player_info->slots[slot_a];
            struct rr_component_player_info_petal_slot *b =
                inv_b ? &game->player_info->secondary_slots[slot_b]
                      : &game->player_info->slots[slot_b];
            struct rr_component_player_info_petal_slot tmp = *a;
            *a = *b;
            *b = tmp;
            rr_game_send_petal_swap_slots(game, inv_a, slot_a, inv_b, slot_b);
        }
    }

    d->preview_active = 0;
    rr_ui_drag_cancel(game);
}

uint8_t rr_ui_drag_drop_to_empty_slot(struct rr_game *game)
{
    struct rr_drag_state *d = &game->drag_state;
    if (!d->active || d->source_type != RR_DRAG_SOURCE_INVENTORY)
        return 0;

    uint8_t total = game->simulation_ready
                        ? (game->player_info ? game->player_info->slot_count
                                             : RR_MAX_SLOT_COUNT)
                        : game->slots_unlocked;

    for (uint8_t i = 0; i < RR_MAX_SLOT_COUNT * 2; ++i)
    {
        uint8_t slot_mod = i % RR_MAX_SLOT_COUNT;
        if (slot_mod < total && game->cache.loadout[i].id == 0)
        {
            game->cache.loadout[i].id = d->source_id;
            game->cache.loadout[i].rarity = d->source_rarity;
            if (game->simulation_ready && game->player_info)
            {
                if (i < RR_MAX_SLOT_COUNT)
                {
                    game->player_info->slots[i].id = d->source_id;
                    game->player_info->slots[i].rarity = d->source_rarity;
                }
                else
                {
                    game->player_info->secondary_slots[i % RR_MAX_SLOT_COUNT].id = d->source_id;
                    game->player_info->secondary_slots[i % RR_MAX_SLOT_COUNT].rarity = d->source_rarity;
                }
                ++game->loadout_counts[d->source_id][d->source_rarity];
                rr_game_send_equip_petal(game, i, d->source_id, d->source_rarity);
            }
            d->preview_active = 0;
            rr_ui_drag_cancel(game);
            return 1;
        }
    }
    return 0;
}

struct drag_ghost_data
{
    uint8_t dummy;
};

static void drag_ghost_animate(struct rr_ui_element *this, struct rr_game *game)
{
    struct rr_drag_state *d = &game->drag_state;
    if (!d->active) return;

    float mx = game->input_data->mouse_x;
    float my = game->input_data->mouse_y;
    d->wobble_time += game->lerp_delta;
    d->target_slot = rr_ui_drag_find_target_slot(game);

    if (d->target_slot != d->prev_target_slot)
    {
        d->prev_target_slot = d->target_slot;
        d->preview_active = d->target_slot != 255;
    }

    if (game->input_data->mouse_buttons_up_this_tick & 1)
    {
        if (d->target_slot != 255)
        {
            rr_ui_drag_drop_to_slot(game, d->target_slot);
        }
        else if (d->source_type == RR_DRAG_SOURCE_INVENTORY)
        {
            if (!rr_ui_drag_drop_to_empty_slot(game))
                rr_ui_drag_cancel(game);
        }
        else if (d->source_type == RR_DRAG_SOURCE_LOADOUT)
        {
            uint8_t src = d->source_slot;
            if (game->simulation_ready && game->player_info)
            {
                uint8_t inv = src >= RR_MAX_SLOT_COUNT ? 1 : 0;
                uint8_t slot_idx = src % RR_MAX_SLOT_COUNT;
                uint8_t old_id, old_rarity;
                if (inv)
                {
                    old_id = game->player_info->secondary_slots[slot_idx].id;
                    old_rarity = game->player_info->secondary_slots[slot_idx].rarity;
                    game->player_info->secondary_slots[slot_idx].id = 0;
                    game->player_info->secondary_slots[slot_idx].rarity = 0;
                }
                else
                {
                    old_id = game->player_info->slots[slot_idx].id;
                    old_rarity = game->player_info->slots[slot_idx].rarity;
                    game->player_info->slots[slot_idx].id = 0;
                    game->player_info->slots[slot_idx].rarity = 0;
                }
                game->cache.loadout[src].id = 0;
                game->cache.loadout[src].rarity = 0;
                if (old_id)
                    --game->loadout_counts[old_id][old_rarity];
                rr_game_send_unequip_petal(game, src);
            }
            else if (!game->simulation_ready)
            {
                game->cache.loadout[src].id = 0;
                game->cache.loadout[src].rarity = 0;
            }
            rr_ui_drag_cancel(game);
        }
        else
        {
            rr_ui_drag_cancel(game);
        }
        return;
    }

    {
        float target = d->preview_active ? 1.0f : 0.0f;
        float speed = 8.0f;
        d->preview_transition += (target - d->preview_transition) *
            fminf(1.0f, game->lerp_delta * speed);
    }
    d->drag_time += game->lerp_delta;

    {
        struct rr_renderer *renderer = game->renderer;
        struct rr_renderer_context_state state;
        rr_renderer_context_state_init(renderer, &state);

        rr_renderer_translate(renderer, mx - this->abs_x, my - this->abs_y);

        uint8_t id, rarity;
        if (d->source_type == RR_DRAG_SOURCE_INVENTORY)
        {
            id = d->source_id;
            rarity = d->source_rarity;
        }
        else
        {
            if (game->simulation_ready &&
                d->source_slot < RR_MAX_SLOT_COUNT)
            {
                id = game->player_info->slots[d->source_slot].id;
                rarity = game->player_info->slots[d->source_slot].rarity;
            }
            else if (game->simulation_ready)
            {
                id = game->player_info->secondary_slots[
                    d->source_slot - RR_MAX_SLOT_COUNT].id;
                rarity = game->player_info->secondary_slots[
                    d->source_slot - RR_MAX_SLOT_COUNT].rarity;
            }
            else
            {
                id = game->cache.loadout[d->source_slot].id;
                rarity = game->cache.loadout[d->source_slot].rarity;
            }
        }

        if (id != 0)
        {
            float lift_progress = fminf(d->drag_time / 0.25f, 1.0f);
            float lift_scale = 0.85f + 0.15f * lift_progress;
            float ghost_scale = lift_scale +
                (1.0f - lift_scale) * d->preview_transition;
            float ghost_alpha = 1.0f - d->preview_transition;

            rr_renderer_set_global_alpha(renderer, ghost_alpha);
            rr_renderer_rotate(renderer, sinf(d->wobble_time * 8.0f) * 0.1f);
            rr_renderer_scale(renderer, ghost_scale * 55.0f / 60.0f);
            rr_renderer_draw_background(renderer, rarity, 1);
            rr_renderer_draw_petal_with_name(renderer, id, rarity);
            rr_renderer_set_global_alpha(renderer, 1.0f);
        }

        rr_renderer_context_state_free(renderer, &state);
    }
}

static uint8_t drag_ghost_should_show(struct rr_ui_element *this,
                                      struct rr_game *game)
{
    return game->drag_state.active;
}

struct rr_ui_element *rr_ui_drag_ghost_init(void)
{
    struct rr_ui_element *this = rr_ui_element_init();
    struct drag_ghost_data *data = malloc(sizeof *data);
    this->data = data;
    this->abs_width = this->width = 60;
    this->abs_height = this->height = 60;
    this->animate = drag_ghost_animate;
    this->should_show = drag_ghost_should_show;
    this->pass_on_event = 1;
    return this;
}
