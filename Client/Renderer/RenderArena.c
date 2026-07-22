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

#include <Client/Renderer/ComponentRender.h>

#include <math.h>
#include <stdbool.h>

#include <Client/Assets/Render.h>
#include <Client/Assets/RenderFunctions.h>
#include <Client/Game.h>
#include <Client/InputData.h>
#include <Client/Renderer/Renderer.h>
#include <Client/Simulation.h>
#include <Shared/Crypto.h>

void render_custom_map(struct rr_renderer *r, uint8_t biome,
                       struct rr_maze_grid *grid, uint32_t maze_dim,
                       float grid_size, double leftX, double topY,
                       double rightX, double bottomY);
int cw_biome_uses_custom(uint8_t biome);

static float map_feature_random_unit(uint64_t value)
{
    return (float)(value & 0xffff) / 65535.0f;
}

static void render_map_feature(struct rr_renderer *renderer, uint8_t biome,
                               uint64_t seed, float x, float y,
                               float grid_size)
{
    uint8_t feature = seed & 0xff;
    float scale = 1.0f;
    bool uses_672_canvas = false;

    if (biome == rr_biome_id_garden)
    {
        if (feature < 10)
        {
            scale = grid_size / 1200.0f;
            uses_672_canvas = true;
        }
        else if (feature < 132)
            scale = grid_size / 1500.0f;
        else
            scale = grid_size / 1350.0f;
    }
    else if (biome == rr_biome_id_ocean)
    {
        if (feature < 18)
        {
            scale = grid_size / 1200.0f;
            uses_672_canvas = true;
        }
        else
            scale = grid_size / 1550.0f;
    }
    else
        return;

    /* Prop nằm trong local space của ô map; seed giữ vị trí cố định giữa các frame. */
    uint64_t offset_seed = rr_get_hash(seed);
    float offset_x = (map_feature_random_unit(offset_seed) - 0.5f) * grid_size * 0.45f;
    float offset_y =
        (map_feature_random_unit(offset_seed >> 16) - 0.5f) * grid_size * 0.45f;

    struct rr_renderer_context_state state;
    rr_renderer_context_state_init(renderer, &state);
    rr_renderer_translate(renderer, x + offset_x, y + offset_y);
    rr_renderer_rotate(renderer,
                       (map_feature_random_unit(offset_seed >> 32) - 0.5f) * 0.30f);
    rr_renderer_scale(renderer, scale);

    /* Beech/Palm được export trên canvas 672×672, các prop còn lại đã căn giữa (0, 0). */
    if (uses_672_canvas)
        rr_renderer_translate(renderer, -336.0f, -336.0f);

    if (biome == rr_biome_id_garden)
    {
        if (feature < 10)
            rr_prop_beech_tree_draw(renderer);
        else if (feature < 132)
            rr_prop_fern_draw(renderer);
        else
            rr_prop_moss_draw(renderer);
    }
    else if (feature < 18)
        rr_prop_palm_tree_draw(renderer);
    else
        rr_prop_water_lettuce_draw(renderer);

    rr_renderer_context_state_free(renderer, &state);
}

static void render_map_features(struct rr_renderer *renderer, uint8_t biome,
                                uint32_t maze_dim, float maze_grid_size,
                                double left_x, double top_y, double right_x,
                                double bottom_y)
{
    const float cluster_size = 2048.0f;
    const float prop_scale_size = 512.0f;

    if (biome != rr_biome_id_garden && biome != rr_biome_id_ocean)
        return;

    int32_t first_x = (int32_t)floor(left_x / cluster_size);
    int32_t first_y = (int32_t)floor(top_y / cluster_size);
    int32_t last_x = (int32_t)ceil(right_x / cluster_size);
    int32_t last_y = (int32_t)ceil(bottom_y / cluster_size);
    float map_size = maze_dim * maze_grid_size;

    for (int32_t y = first_y; y < last_y; ++y)
    {
        if (y < 0 || y * cluster_size >= map_size)
            continue;

        for (int32_t x = first_x; x < last_x; ++x)
        {
            if (x < 0 || x * cluster_size >= map_size)
                continue;

            uint64_t position = ((uint64_t)(uint32_t)y << 32) | (uint32_t)x;
            uint64_t cluster_seed =
                rr_get_hash(position ^ ((uint64_t)biome << 56));
            uint32_t count = biome == rr_biome_id_garden
                                 ? 3 + (cluster_seed & 3)
                                 : 2 + (cluster_seed & 1);

            for (uint32_t i = 0; i < count; ++i)
            {
                uint64_t seed = rr_get_hash(cluster_seed +
                                             (uint64_t)(i + 1) *
                                                 0x9e3779b97f4a7c15ULL);
                float x_pos = x * cluster_size +
                              map_feature_random_unit(seed) * cluster_size;
                float y_pos = y * cluster_size +
                              map_feature_random_unit(seed >> 16) * cluster_size;

                if (x_pos >= map_size || y_pos >= map_size)
                    continue;

                render_map_feature(renderer, biome, seed, x_pos, y_pos,
                                   prop_scale_size);
            }
        }
    }
}

void render_background(struct rr_component_player_info *player_info,
                       struct rr_game *this)
{
    if (player_info->arena == 0)
        return;
    struct rr_renderer *renderer = this->renderer;
    double extra = 100;
    double scale = player_info->lerp_camera_fov * renderer->scale;
    double leftX =
        player_info->lerp_camera_x - renderer->width / (2 * scale) - extra;
    double rightX =
        player_info->lerp_camera_x + renderer->width / (2 * scale) + extra;
    double topY =
        player_info->lerp_camera_y - renderer->height / (2 * scale) - extra;
    double bottomY =
        player_info->lerp_camera_y + renderer->height / (2 * scale) + extra;

#define GRID_SIZE (512.0f)
    double newLeftX = floorf(leftX / GRID_SIZE) * GRID_SIZE;
    double newTopY = floorf(topY / GRID_SIZE) * GRID_SIZE;
    for (; newLeftX < rightX; newLeftX += GRID_SIZE)
    {
        for (double currY = newTopY; currY < bottomY; currY += GRID_SIZE)
        {
            uint32_t tile_index =
                rr_get_hash((uint32_t)(((newLeftX + 8192) / GRID_SIZE + 1) *
                                       ((currY + 8192) / GRID_SIZE + 2))) %
                3;
            struct rr_renderer_context_state state;
            rr_renderer_context_state_init(renderer, &state);
            rr_renderer_translate(renderer, newLeftX + GRID_SIZE / 2,
                                  currY + GRID_SIZE / 2);
            rr_renderer_scale(renderer, (GRID_SIZE + 2) / 256);
            if (this->selected_biome == rr_biome_id_hell_creek)
                rr_renderer_draw_tile_hell_creek(renderer, tile_index);
            else if (this->selected_biome == rr_biome_id_ocean)
                rr_renderer_draw_tile_ocean(renderer, tile_index);
            else
                rr_renderer_draw_tile_garden(renderer, tile_index);
            rr_renderer_context_state_free(renderer, &state);
        }
    }
#undef GRID_SIZE

    struct rr_component_arena *arena =
        rr_simulation_get_arena(this->simulation, player_info->arena);
    float grid_size = RR_MAZES[arena->biome].grid_size;
    uint32_t maze_dim = RR_MAZES[arena->biome].maze_dim;
    struct rr_maze_grid *grid = RR_MAZES[arena->biome].maze;

    /* Vẽ decor sau nền, trước tường maze để cây/cỏ không che lối đi hay tường. */
    render_map_features(renderer, arena->biome, maze_dim, grid_size, leftX,
                        topY, rightX, bottomY);

    if (cw_biome_uses_custom(arena->biome))
    {
        render_custom_map(renderer, arena->biome, grid, maze_dim, grid_size,
                          leftX, topY, rightX, bottomY);
    }
    else
    {
        rr_renderer_set_fill(renderer, 0xff000000);
        rr_renderer_set_global_alpha(renderer, 0.5f);
        int32_t nx = floorf(leftX / grid_size);
        int32_t ny = floorf(topY / grid_size);
        for (; nx < rightX / grid_size; ++nx)
            for (int32_t currY = ny; currY < bottomY / grid_size; ++currY)
            {
                rr_renderer_set_fill(renderer, 0xff000000);
                uint8_t tile =
                    (nx < 0 || currY < 0 || nx >= (int32_t)maze_dim ||
                     currY >= (int32_t)maze_dim)
                        ? 0
                        : grid[currY * maze_dim + nx].value;
                if (tile != 1)
                {
                    rr_renderer_begin_path(renderer);
                    if (tile == 0)
                        rr_renderer_fill_rect(renderer, nx * grid_size,
                                              currY * grid_size, grid_size,
                                              grid_size);
                    else
                    {
                        uint8_t left = (tile >> 1) & 1;
                        uint8_t top = tile & 1;
                        uint8_t inverse = 1 - ((tile >> 3) & 1);
                        rr_renderer_move_to(renderer,
                                            (nx + inverse ^ left) * grid_size,
                                            (currY + inverse ^ top) * grid_size);
                        float start_angle = 0;
                        if (top == 0 && left == 1)
                            start_angle = M_PI / 2;
                        else if (top == 1 && left == 1)
                            start_angle = M_PI;
                        else if (top == 1 && left == 0)
                            start_angle = M_PI * 3 / 2;
                        rr_renderer_partial_arc(renderer,
                                                (nx + left) * grid_size,
                                                (currY + top) * grid_size,
                                                grid_size, start_angle,
                                                start_angle + M_PI / 2, 0);
                        rr_renderer_fill(renderer);
                    }
                }
            }
        rr_renderer_set_global_alpha(renderer, 1.0f);
    }

    if (this->cache.show_coordinates)
    {
        int32_t nx = floorf(leftX / grid_size);
        int32_t ny = floorf(topY / grid_size);
        for (; nx < rightX / grid_size; ++nx)
            for (int32_t currY = ny; currY < bottomY / grid_size; ++currY)
            {
                if (nx % 2 && currY % 2)
                {
                    char pos[10];
                    sprintf(pos, "%d %d", (nx - 1) / 2, (currY - 1) / 2);
                    rr_renderer_set_text_size(renderer, 64);
                    rr_renderer_set_text_align(renderer, 1);
                    rr_renderer_set_text_baseline(renderer, 1);
                    rr_renderer_set_fill(renderer, 0xffffffff);
                    rr_renderer_fill_text(renderer, pos, nx * grid_size,
                                          currY * grid_size);
                }
            }
    }
}

void rr_component_arena_render(EntityIdx entity, struct rr_game *this,
                               struct rr_simulation *simulation)
{
    struct rr_renderer_context_state state2;
    struct rr_component_player_info *player_info = this->player_info;

    struct rr_component_arena *arena =
        rr_simulation_get_arena(this->simulation, player_info->arena);
    rr_renderer_context_state_init(this->renderer, &state2);
    render_background(player_info, this);

    rr_renderer_context_state_free(this->renderer, &state2);
}
