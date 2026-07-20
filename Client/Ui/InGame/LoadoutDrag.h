#pragma once

#include <stdint.h>

enum rr_drag_source_type
{
    RR_DRAG_SOURCE_NONE,
    RR_DRAG_SOURCE_INVENTORY,
    RR_DRAG_SOURCE_LOADOUT,
};

struct rr_game;
struct rr_ui_element;

struct rr_drag_state
{
    uint8_t active;
    uint8_t source_type;
    uint8_t source_slot;
    uint8_t source_id;
    uint8_t source_rarity;
    uint8_t target_slot;

    uint8_t preview_active;
    uint8_t prev_target_slot;

    float click_off_x;
    float click_off_y;
    float wobble_time;
    float drag_time;
    float preview_transition;
};

void rr_ui_drag_start_inventory(struct rr_game *game,
                                uint8_t id, uint8_t rarity,
                                float click_x, float click_y);

void rr_ui_drag_start_slot(struct rr_game *game, uint8_t slot,
                           float click_x, float click_y);

void rr_ui_drag_cancel(struct rr_game *game);

uint8_t rr_ui_drag_is_over_slot(struct rr_game *game,
                                uint8_t slot_pos,
                                float slot_abs_x, float slot_abs_y,
                                float slot_width, float slot_height);

void rr_ui_drag_drop_to_slot(struct rr_game *game, uint8_t target_slot);

uint8_t rr_ui_drag_drop_to_empty_slot(struct rr_game *game);

void rr_ui_drag_register_slot(uint8_t pos, float abs_x, float abs_y,
                              float width, float height);

void rr_ui_drag_clear_slot_registry(void);

struct rr_ui_element *rr_ui_drag_ghost_init(void);
