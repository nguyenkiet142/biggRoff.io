#include <Client/Ui/Ui.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Client/Assets/RenderFunctions.h>
#include <Client/Game.h>
#include <Client/InputData.h>
#include <Client/Renderer/Renderer.h>

#include <Client/Ui/Engine.h>

#include <Shared/SimulationCommon.h>
#include <Shared/Utilities.h>
#include <Shared/pb.h>

#include "LoadoutDrag.h"

struct loadout_slot_data
{
    uint8_t pos;
    uint8_t prev_id;
    uint8_t prev_rarity;
    float secondary_animation;
    float lerp_cd;
    float lerp_hp;
};

static void loadout_get_petal(struct rr_game *game, uint8_t pos,
                              uint8_t *id, uint8_t *rarity)
{
    if (game->simulation_ready)
    {
        if (pos < RR_MAX_SLOT_COUNT)
        {
            *id = game->player_info->slots[pos].id;
            *rarity = game->player_info->slots[pos].rarity;
        }
        else
        {
            *id = game->player_info->secondary_slots[pos - RR_MAX_SLOT_COUNT].id;
            *rarity = game->player_info->secondary_slots[pos - RR_MAX_SLOT_COUNT].rarity;
        }
    }
    else
    {
        *id = game->cache.loadout[pos].id;
        *rarity = game->cache.loadout[pos].rarity;
    }
}

static uint8_t slot_has_petal(struct rr_game *game, uint8_t pos)
{
    if (game->simulation_ready)
    {
        struct rr_component_player_info_petal_slot *slot =
            pos < RR_MAX_SLOT_COUNT
                ? &game->player_info->slots[pos]
                : &game->player_info->secondary_slots[pos - RR_MAX_SLOT_COUNT];
        return slot->id != 0;
    }
    return game->cache.loadout[pos].id != 0;
}

static void loadout_slot_on_event(struct rr_ui_element *this,
                                  struct rr_game *game)
{
    struct loadout_slot_data *data = this->data;
    uint8_t pos = data->pos;

    if ((game->input_data->mouse_buttons_down_this_tick & 1) &&
        game->pressed == this && slot_has_petal(game, pos) &&
        !game->drag_state.active)
    {
        float click_off_x = game->input_data->mouse_x - this->abs_x;
        float click_off_y = game->input_data->mouse_y - this->abs_y;
        rr_ui_drag_start_slot(game, pos, click_off_x, click_off_y);
        return;
    }

    if ((game->input_data->mouse_buttons_up_this_tick & 1) &&
        game->drag_state.active)
    {
        if (rr_ui_drag_is_over_slot(game, pos,
                                     this->abs_x, this->abs_y,
                                     this->width, this->height))
        {
            rr_ui_drag_drop_to_slot(game, pos);
            return;
        }
    }

    if ((game->input_data->mouse_buttons_up_this_tick & 1) &&
        game->pressed == this && !game->drag_state.active &&
        !game->simulation_ready)
    {
        game->cache.loadout[pos].id = 0;
        game->cache.loadout[pos].rarity = 0;
    }

    if (slot_has_petal(game, pos))
    {
        rr_ui_set_tooltip_metadata(
            game->petal_tooltips[data->prev_id][data->prev_rarity], 0,
            pos % RR_MAX_SLOT_COUNT);
        rr_ui_render_tooltip_above(
            this, game->petal_tooltips[data->prev_id][data->prev_rarity], game);
        game->cursor = rr_game_cursor_pointer;
    }
}

static uint8_t loadout_slot_should_show(struct rr_ui_element *this,
                                        struct rr_game *game)
{
    struct loadout_slot_data *data = this->data;

    if (game->simulation_ready)
    {
        struct rr_component_player_info *player_info = game->player_info;
        if (!player_info) return 0;
        return player_info->slot_count > data->pos % RR_MAX_SLOT_COUNT;
    }

    return data->pos % RR_MAX_SLOT_COUNT < game->slots_unlocked;
}

static void loadout_slot_animate(struct rr_ui_element *this,
                                  struct rr_game *game)
{
    if (this->completely_hidden) return;

    struct loadout_slot_data *data = this->data;
    struct rr_renderer *renderer = game->renderer;
    uint8_t pos = data->pos;

    this->width = this->abs_width * (1 - this->animation);
    this->height = this->abs_height * (1 - this->animation);
    rr_renderer_scale(renderer, (1 - this->animation));

    rr_ui_drag_register_slot(pos, this->abs_x, this->abs_y,
                             this->width, this->height);

    uint8_t is_hovering = 0;
    if (game->drag_state.active)
    {
        if (!(game->drag_state.source_type == RR_DRAG_SOURCE_LOADOUT &&
              game->drag_state.source_slot == pos) &&
            rr_ui_drag_is_over_slot(game, pos, this->abs_x, this->abs_y,
                                     this->width, this->height))
            is_hovering = 1;
    }

    rr_renderer_scale(renderer, renderer->scale * this->width / 60);

    uint8_t is_drag_source = (game->drag_state.active &&
                              game->drag_state.source_type == RR_DRAG_SOURCE_LOADOUT &&
                              game->drag_state.source_slot == pos);

    if (is_drag_source)
        rr_renderer_set_global_alpha(renderer, 0.35f);

    if (is_hovering)
    {
        uint8_t rar;
        if (game->drag_state.source_type == RR_DRAG_SOURCE_INVENTORY)
            rar = game->drag_state.source_rarity;
        else
            loadout_get_petal(game, game->drag_state.source_slot, NULL, &rar);
        rr_renderer_draw_background(renderer, rar, 1);
    }
    else
        rr_renderer_draw_background(renderer, rr_rarity_id_max + 1, 1);

    uint8_t id, rarity;
    if (game->simulation_ready)
    {
        struct rr_component_player_info_petal_slot *slot =
            pos < RR_MAX_SLOT_COUNT
                ? &game->player_info->slots[pos]
                : &game->player_info->secondary_slots[pos - RR_MAX_SLOT_COUNT];
        id = slot->id;
        rarity = slot->rarity;

        if (id != 0)
        {
            data->prev_id = id;
            data->prev_rarity = rarity;
        }

        data->secondary_animation = rr_lerp(
            data->secondary_animation,
            id != data->prev_id || rarity != data->prev_rarity || id == 0,
            12 * game->lerp_delta);

        float cd = (game->flower_dead ? 0 : slot->client_cooldown) * (1.0f / 255);
        data->lerp_cd = rr_lerp(data->lerp_cd, cd, 12 * game->lerp_delta);
        float hp = (game->flower_dead ? 255 : slot->client_health) * (1.0f / 255);
        data->lerp_hp = rr_lerp(data->lerp_hp, hp, 12 * game->lerp_delta);
    }
    else
    {
        id = game->cache.loadout[pos].id;
        rarity = game->cache.loadout[pos].rarity;

        if (id != 0)
        {
            data->prev_id = id;
            data->prev_rarity = rarity;
        }
        data->secondary_animation = rr_lerp(
            data->secondary_animation, id == 0, 12 * game->lerp_delta);
    }

    rr_renderer_set_global_alpha(renderer, 1.0f);
}

static void loadout_slot_on_render(struct rr_ui_element *this,
                                    struct rr_game *game)
{
    struct loadout_slot_data *data = this->data;
    struct rr_renderer *renderer = game->renderer;
    uint8_t pos = data->pos;
    struct rr_drag_state *d = &game->drag_state;

    struct rr_renderer_context_state state;
    rr_renderer_context_state_init(renderer, &state);

    uint8_t is_drag_source = d->active &&
        d->source_type == RR_DRAG_SOURCE_LOADOUT &&
        d->source_slot == pos;

    uint8_t is_hovering = d->active && !is_drag_source &&
        rr_ui_drag_is_over_slot(game, pos, this->abs_x, this->abs_y,
                                 this->width, this->height);

    if (is_drag_source)
    {
        goto bars;
    }

    if (is_hovering)
    {
        uint8_t preview_id, preview_rarity;
        if (d->source_type == RR_DRAG_SOURCE_INVENTORY)
        {
            preview_id = d->source_id;
            preview_rarity = d->source_rarity;
        }
        else
        {
            loadout_get_petal(game, d->source_slot,
                              &preview_id, &preview_rarity);
        }
        float ps = 0.7f + 0.3f * d->preview_transition;
        rr_renderer_scale(renderer, ps);
        rr_renderer_draw_background(renderer, preview_rarity, 1);
        rr_renderer_draw_petal_with_name(renderer, preview_id, preview_rarity);
        goto bars;
    }

    rr_renderer_scale(renderer, (1 - data->secondary_animation));
    rr_renderer_draw_background(renderer, data->prev_rarity, 1);
    rr_renderer_draw_petal_with_name(renderer, data->prev_id, data->prev_rarity);

bars:
    rr_renderer_context_state_free(renderer, &state);

    if (game->simulation_ready && data->pos < RR_MAX_SLOT_COUNT)
    {
        float pct = data->lerp_cd * data->lerp_cd * (3 - 2 * data->lerp_cd);
        rr_renderer_set_fill(renderer, 0x40000000);
        rr_renderer_fill_rect(renderer, -30.0f, 30.0f - 60.0f * pct,
                              60.0f, 60.0f * pct);
        rr_renderer_fill_rect(renderer, -30.0f, -30.0f, 60.0f,
                              60.0f * (1 - data->lerp_hp));
    }
}

static struct rr_ui_element *loadout_slot_init(uint8_t pos)
{
    struct rr_ui_element *this = rr_ui_element_init();
    struct loadout_slot_data *data = malloc(sizeof *data);
    data->pos = pos;
    data->prev_id = 0;
    data->prev_rarity = 0;
    data->secondary_animation = 1;
    data->lerp_cd = 0;
    data->lerp_hp = 255;
    this->data = data;
    this->abs_width = this->width = pos < RR_MAX_SLOT_COUNT ? 60 : 50;
    this->abs_height = this->height = pos < RR_MAX_SLOT_COUNT ? 60 : 50;
    this->on_render = loadout_slot_on_render;
    this->animate = loadout_slot_animate;
    this->should_show = loadout_slot_should_show;
    this->on_event = loadout_slot_on_event;
    return this;
}

struct rr_ui_element *rr_ui_title_screen_loadout_button_init(uint8_t pos)
{
    return loadout_slot_init(pos);
}

struct rr_ui_element *rr_ui_loadout_button_init(uint8_t pos)
{
    return loadout_slot_init(pos);
}

static uint8_t secondary_loadout_should_show(struct rr_ui_element *this,
                                              struct rr_game *game)
{
    return this->elements.start[0]->should_show(this->elements.start[0], game);
}

struct rr_ui_element *rr_ui_secondary_loadout_button_init(uint8_t pos)
{
    struct rr_ui_element *slot = loadout_slot_init(pos + RR_MAX_SLOT_COUNT);

    char *text = pos == 0   ? "[1]"
                 : pos == 1 ? "[2]"
                 : pos == 2 ? "[3]"
                 : pos == 3 ? "[4]"
                 : pos == 4 ? "[5]"
                 : pos == 5 ? "[6]"
                 : pos == 6 ? "[7]"
                 : pos == 7 ? "[8]"
                 : pos == 8 ? "[9]"
                 : pos == 9 ? "[0]"
                 : pos == 10 ? "[-]" : "[=]";

    struct rr_ui_element *c = rr_ui_v_container_init(
        rr_ui_container_init(), 0, 10, slot,
        rr_ui_text_init(text, 14, 0xffffffff), NULL);
    c->should_show = secondary_loadout_should_show;
    return c;
}
