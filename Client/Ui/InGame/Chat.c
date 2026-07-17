// Copyright (C) 2024 Paul Johnson
// Copyright (C) 2024-2025 Maxim Nesterov

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <Client/Ui/Ui.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <Client/Game.h>
#include <Client/InputData.h>
#include <Client/Simulation.h>
#include <Client/Socket.h>
#include <Client/Ui/Engine.h>
#include <Shared/Bitset.h>
#include <Shared/pb.h>
#include <Shared/StaticData.h>
#include <Client/DOM.h>

static uint8_t min_level_to_chat = 0;

static void compute_chat_suggestions(struct rr_game *game, int cursor_pos)
{
    game->chat.suggestion_count = 0;
    game->chat.selected_suggestion = 0;
    memset(game->chat.suggestion_has_id, 0, CHAT_MAX_SUGGESTIONS);
    char *text = game->chat.sending;
    if (text[0] != '/' || strlen(text) < 2) return;

    int text_len = strlen(text);
    if (cursor_pos < 0 || cursor_pos > text_len)
        cursor_pos = text_len;

    char *active_end = text + cursor_pos;
    int spaces = 0;
    for (char *p = text + 1; p < active_end; p++)
        if (*p == ' ') spaces++;

    char cmd[32] = {0};
    if (spaces == 0)
    {
        int len = active_end - (text + 1);
        if (len > 31) len = 31;
        strncpy(cmd, text + 1, len);
        cmd[len] = 0;
    }
    else
    {
        char *space = strchr(text + 1, ' ');
        int len;
        if (space >= active_end)
        {
            len = active_end - (text + 1);
        }
        else
        {
            len = space - (text + 1);
        }
        if (len > 31) len = 31;
        strncpy(cmd, text + 1, len);
        cmd[len] = 0;
    }

    static const char *all_cmds[] = {"freebiggift", "login", "logout", "give", "summon",
        "kill", "speed", "fov", "flags", "lv"};
    int cmd_count = 10;

    if (spaces == 0)
    {
        int cmd_len = strlen(cmd);
        for (int i = 0; i < cmd_count && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
        {
            if (strncmp(cmd, all_cmds[i], cmd_len) == 0)
            {
                snprintf(game->chat.suggestions[game->chat.suggestion_count], 64, "%s", all_cmds[i]);
                game->chat.suggestion_count++;
            }
        }
        return;
    }

    char *rest = strchr(text + 1, ' ') + 1;
    int arg_spaces = 0;
    char *word_start = rest;
    for (char *p = rest; p < active_end; p++)
        if (*p == ' ') { arg_spaces++; word_start = p + 1; }

    char partial[32] = {0};
    int plen = active_end - word_start;
    if (plen > 31) plen = 31;
    strncpy(partial, word_start, plen);
    partial[plen] = 0;

    if (strcmp(cmd, "give") == 0)
    {
        if (arg_spaces == 0)
        {
            for (uint8_t i = 1; i < rr_petal_id_max && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                char label[16];
                snprintf(label, 16, "%d", i);
                if (plen == 0 || strncmp(label, partial, plen) == 0 ||
                    strncmp(RR_PETAL_NAMES[i], partial, plen) == 0)
                {
                    uint8_t idx = game->chat.suggestion_count;
                    snprintf(game->chat.suggestions[idx], 64, "%s (%d)", RR_PETAL_NAMES[i], i);
                    game->chat.suggestion_has_id[idx] = 1;
                    game->chat.suggestion_ids[idx] = i;
                    game->chat.suggestion_count++;
                }
            }
        }
        else if (arg_spaces == 1)
        {
            for (uint8_t i = 0; i < rr_rarity_id_max && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                char label[16];
                snprintf(label, 16, "%d", i);
                if (plen == 0 || strncmp(label, partial, plen) == 0)
                {
                    snprintf(game->chat.suggestions[game->chat.suggestion_count], 64, "%d", i);
                    game->chat.suggestion_count++;
                }
            }
        }
        else
        {
            static const char *counts[] = {"1", "10", "100", "999"};
            for (int i = 0; i < 4 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(counts[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], counts[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
    }
    else if (strcmp(cmd, "summon") == 0)
    {
        uint8_t max_mob = rr_mob_id_max;
        if (arg_spaces == 0)
        {
            for (uint8_t i = 0; i < max_mob && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                char label[16];
                snprintf(label, 16, "%d", i);
                if (plen == 0 || strncmp(label, partial, plen) == 0 ||
                    strncmp(RR_MOB_NAMES[i], partial, plen) == 0)
                {
                    uint8_t idx = game->chat.suggestion_count;
                    snprintf(game->chat.suggestions[idx], 64, "%s (%d)", RR_MOB_NAMES[i], i);
                    game->chat.suggestion_has_id[idx] = 1;
                    game->chat.suggestion_ids[idx] = i;
                    game->chat.suggestion_count++;
                }
            }
        }
        else if (arg_spaces == 1)
        {
            for (uint8_t i = 0; i < rr_rarity_id_max && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                char label[16];
                snprintf(label, 16, "%d", i);
                if (plen == 0 || strncmp(label, partial, plen) == 0)
                {
                    snprintf(game->chat.suggestions[game->chat.suggestion_count], 64, "%d", i);
                    game->chat.suggestion_count++;
                }
            }
        }
        else
        {
            static const char *counts[] = {"1", "10", "100"};
            for (int i = 0; i < 3 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(counts[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], counts[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
    }
    else if (strcmp(cmd, "kill") == 0)
    {
        strcpy(game->chat.suggestions[0], "(no arguments)");
        game->chat.suggestion_count = 1;
    }
    else if (strcmp(cmd, "speed") == 0 || strcmp(cmd, "fov") == 0)
    {
        if (arg_spaces == 0)
        {
            static const char *vals[] = {"0", "25", "50", "75", "100"};
            for (int i = 0; i < 5 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(vals[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], vals[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
    }
    else if (strcmp(cmd, "flags") == 0)
    {
        if (arg_spaces == 0)
        {
            static const char *flag_names[] = {"all", "invisible", "invulnerable", "no_aggro",
                                               "no_wall_collision", "no_collision", "no_grid_influence"};
            for (int i = 0; i < 7 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(flag_names[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], flag_names[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
        else if (arg_spaces == 1)
        {
            static const char *tf[] = {"true", "false"};
            for (int i = 0; i < 2 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(tf[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], tf[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
    }
    else if (strcmp(cmd, "lv") == 0)
    {
        if (arg_spaces == 0)
        {
            static const char *lv_vals[] = {"1", "5", "10", "15", "20"};
            for (int i = 0; i < 5 && game->chat.suggestion_count < CHAT_MAX_SUGGESTIONS; i++)
            {
                if (plen == 0 || strncmp(lv_vals[i], partial, plen) == 0)
                {
                    strcpy(game->chat.suggestions[game->chat.suggestion_count], lv_vals[i]);
                    game->chat.suggestion_count++;
                }
            }
        }
    }
}

static void insert_chat_suggestion(struct rr_game *game)
{
    if (game->chat.suggestion_count == 0) return;
    if (game->chat.selected_suggestion >= game->chat.suggestion_count) return;
    uint8_t sel = game->chat.selected_suggestion;
    char insert_val[16] = {0};
    if (game->chat.suggestion_has_id[sel])
        snprintf(insert_val, 16, "%d", game->chat.suggestion_ids[sel]);
    else
        strncpy(insert_val, game->chat.suggestions[sel], 15);

    char new_text[128] = {0};
    char *text = game->chat.sending;
    char *space = strchr(text + 1, ' ');
    if (space == NULL)
    {
        new_text[0] = '/';
        strcpy(new_text + 1, insert_val);
        strcat(new_text, " ");
    }
    else
    {
        size_t plen = space - text + 1;
        strncpy(new_text, text, plen);
        new_text[plen] = 0;
        strcat(new_text, insert_val);
        strcat(new_text, " ");
    }
    strcpy(game->chat.sending, new_text);
    rr_dom_set_text("_0x4523", new_text);
    uint32_t new_cursor = strlen(new_text);
    rr_dom_set_selection_start("_0x4523", new_cursor);
}

static void chat_bar_on_event(struct rr_ui_element *this, struct rr_game *game)
{
    if ((game->input_data->mouse_buttons_up_this_tick & 1) &&
        level_from_xp(game->cache.experience) >= min_level_to_chat)
    {
        game->chat.chat_active = 1;
        game->menu_open = 0;
    }
    if (game->chat.chat_active == 0)
        game->cursor = rr_game_cursor_pointer;
}

static void chat_bar_render(struct rr_ui_element *this, struct rr_game *game)
{
    struct rr_renderer *renderer = game->renderer;
    if (!game->chat.chat_active || game->chat.suggestion_count == 0)
        return;

    if (game->chat.selected_suggestion >= game->chat.suggestion_count)
        game->chat.selected_suggestion = 0;

    float row_h = 20;
    float pad = 2;

    float total_h = game->chat.suggestion_count * row_h;
    float left = -this->abs_width / 2;
    float top = -this->abs_height / 2 - total_h - pad;

    float abs_left = this->abs_x - this->abs_width / 2;
    float abs_top = this->abs_y - this->abs_height / 2 - total_h - pad;

    for (uint8_t i = 0; i < game->chat.suggestion_count; i++)
    {
        uint32_t bg = (i == game->chat.selected_suggestion) ? 0x404040ff : 0xb3232323;

        rr_renderer_set_fill(renderer, bg);
        rr_renderer_begin_path(renderer);
        rr_renderer_rect(renderer, left, top + i * row_h, this->abs_width, row_h);
        rr_renderer_fill(renderer);

        rr_renderer_set_fill(renderer, 0xffffffff);
        rr_renderer_set_text_align(renderer, 0);
        rr_renderer_set_text_baseline(renderer, 1);
        rr_renderer_set_text_size(renderer, 14);
        rr_renderer_fill_text(renderer, game->chat.suggestions[i],
            left + 4, top + i * row_h + row_h / 2);

        float mx = game->input_data->mouse_x / renderer->scale;
        float my = game->input_data->mouse_y / renderer->scale;
        if (mx >= abs_left && mx <= abs_left + this->abs_width &&
            my >= abs_top + i * row_h && my <= abs_top + i * row_h + row_h)
        {
            game->cursor = rr_game_cursor_pointer;
            if (game->input_data->mouse_buttons_up_this_tick & 1)
            {
                game->chat.selected_suggestion = i;
                insert_chat_suggestion(game);
            }
        }
    }
}

static void chat_bar_animate(struct rr_ui_element *this, struct rr_game *game)
{
    rr_ui_default_animate(this, game);
    if (level_from_xp(game->cache.experience) < min_level_to_chat)
        game->chat.chat_active = 0;
    if (game->chat.chat_active)
    {
        this->fill = 0xffffffff;
        this->stroke = 0xff222222;
    }
    else
    {
        this->fill = 0x80000000;
        game->chat.suggestion_count = 0;
    }

    uint8_t error = 0;
    if (game->chat.chat_active)
    {
        int cursor_pos = rr_dom_get_selection_start("_0x4523");

        if (game->chat.sending[0] == '/')
        {
            compute_chat_suggestions(game, cursor_pos);

            if (game->chat.suggestion_count == 0)
                error = 1;

            if (rr_bitset_get_bit(game->input_data->keys_pressed_this_tick, 9))
            {
                if (game->chat.suggestion_count > 0)
                {
                    insert_chat_suggestion(game);
                    game->chat.suggestion_count = 0;
                }
            }
            if (rr_bitset_get_bit(game->input_data->keys_pressed_this_tick, 38))
            {
                if (game->chat.suggestion_count > 0)
                {
                    game->chat.selected_suggestion = (game->chat.selected_suggestion == 0) ?
                        game->chat.suggestion_count - 1 : game->chat.selected_suggestion - 1;
                }
            }
            if (rr_bitset_get_bit(game->input_data->keys_pressed_this_tick, 40))
            {
                if (game->chat.suggestion_count > 0)
                {
                    game->chat.selected_suggestion = (game->chat.selected_suggestion + 1) %
                        game->chat.suggestion_count;
                }
            }
            if (rr_bitset_get_bit(game->input_data->keys_pressed_this_tick, 27))
            {
                game->chat.suggestion_count = 0;
            }
        }
        else
        {
            game->chat.suggestion_count = 0;
        }
    }

    if (error)
        rr_dom_set_style("_0x4523", "color", "#ff4444");
    else
        rr_dom_set_style("_0x4523", "color", "");

    if (rr_bitset_get_bit(game->input_data->keys_pressed_this_tick, 13) &&
        level_from_xp(game->cache.experience) >= min_level_to_chat)
    {
        if (game->chat.chat_active)
        {
            if (game->chat.sending[0] != 0)
            {
                struct proto_bug encoder;
                proto_bug_init(&encoder, RR_OUTGOING_PACKET);
                proto_bug_write_uint8(&encoder, game->socket.quick_verification, "qv");
                proto_bug_write_uint8(&encoder, rr_serverbound_chat, "header");
                proto_bug_write_string(&encoder, game->chat.sending, 64, "chat");
                rr_websocket_send(&game->socket, encoder.current - encoder.start);
            }
            memset(game->chat.sending, 0, sizeof game->chat.sending);
            rr_dom_set_text("_0x4523", "");
            game->chat.suggestion_count = 0;
        }
        else
            game->menu_open = 0;
        game->chat.chat_active ^= 1;
    }
    if (game->menu_open != 0)
        game->chat.chat_active = 0;
    if (game->chat.chat_active)
        rr_dom_focus("_0x4523");
    else
        rr_dom_blur("_0x4523");
};

static uint8_t chat_bar_choose(struct rr_ui_element *this, struct rr_game *game)
{
    if (level_from_xp(game->cache.experience) < min_level_to_chat)
        return 2;
    return !(game->chat.chat_active || game->chat.sending[0]);
}

static uint8_t chat_should_show(struct rr_ui_element *this,
                                struct rr_game *game)
{
    uint8_t r = game->simulation_ready &&
                !game->cache.disable_chat &&
                !game->cache.hide_ui &&
                game->menu_open != rr_game_menu_inventory &&
                game->menu_open != rr_game_menu_gallery &&
                game->menu_open != rr_game_menu_crafting;
    if (!r)
        game->chat.chat_active = 0;
    return r;
}

static struct rr_ui_element *rr_ui_chat_text_init(struct rr_game_chat_message
*message)
{
    uint32_t color = message->type == 1 ? 0xffffdd00 : 0xffffffff;
    return rr_ui_set_justify(
        rr_ui_h_container_init(rr_ui_container_init(), 0, 10,
            rr_ui_text_init(message->text, 16, color),
            NULL
        ),
    -1, -1);
}

struct rr_ui_element *rr_ui_message_box_init(struct rr_game *game)
{
    return rr_ui_v_container_init(rr_ui_container_init(), 0, 10,
        rr_ui_chat_text_init(&game->chat.messages[0]),
        rr_ui_chat_text_init(&game->chat.messages[1]),
        rr_ui_chat_text_init(&game->chat.messages[2]),
        rr_ui_chat_text_init(&game->chat.messages[3]),
        rr_ui_chat_text_init(&game->chat.messages[4]),
        rr_ui_chat_text_init(&game->chat.messages[5]),
        rr_ui_chat_text_init(&game->chat.messages[6]),
        rr_ui_chat_text_init(&game->chat.messages[7]),
        rr_ui_chat_text_init(&game->chat.messages[8]),
        rr_ui_chat_text_init(&game->chat.messages[9]),
        NULL
    );
}

struct rr_ui_element *rr_ui_chat_bar_init(struct rr_game *game)
{
    struct rr_ui_element *input = rr_ui_text_input_init(200, 18, game->chat.sending, 64, "_0x4523");
    struct rr_ui_element *text1 = rr_ui_text_init("Press [Enter] or click here to chat", 14, 0xffffffff);
    char *buf = malloc((sizeof *buf) * 24);
    sprintf(buf, "Reach level %u to chat", min_level_to_chat);
    struct rr_ui_element *text2 = rr_ui_text_init(buf, 14, 0xffffffff);
    struct rr_ui_element *inner = rr_ui_multi_choose_element_init(
        chat_bar_choose,
        rr_ui_flex_container_init(
            rr_ui_text_init("[Global]", 14, 0xffffffff),
            input,
            10
        ),
        text1,
        text2,
        NULL
    );
    inner->prevent_on_event = 1;
    struct rr_ui_element *this = rr_ui_set_background(
        rr_ui_h_container_init(rr_ui_container_init(), 10, 0, inner, NULL),
    0x80000000);
    this->animate = chat_bar_animate;
    this->on_event = chat_bar_on_event;
    this->on_secondary_render = chat_bar_render;
    struct rr_ui_element *messages = rr_ui_message_box_init(game);
    messages->prevent_on_event = 1;
    struct rr_ui_element *chat = rr_ui_v_container_init(rr_ui_container_init(), 10, 10,
        rr_ui_set_justify(messages, -1, -1),
        rr_ui_set_justify(this, -1, -1),
        NULL
    );
    chat->pass_on_event = 1;
    chat->should_show = chat_should_show;
    return chat;
}
