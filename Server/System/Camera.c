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

#include <Server/System/System.h>

#include <stdlib.h>

#include <Server/Client.h>
#include <Server/Simulation.h>
#include <Shared/Entity.h>
#include <Shared/Vector.h>

void rr_system_camera_tick(struct rr_simulation *this)
{
    for (EntityIdx i = 0; i < this->player_info_count; ++i)
    {
        struct rr_component_player_info *player_info =
            rr_simulation_get_player_info(this, this->player_info_vector[i]);
        if (player_info->flower_id != RR_NULL_ENTITY &&
            !is_dead_flower(this, player_info->flower_id))
        {
            rr_component_player_info_set_spectate_target(
                player_info, player_info->flower_id);
            player_info->spectate_ticks = 125;
            player_info->spectating_single_target = 0;
        }
        if (player_info->spectate_target != player_info->flower_id)
        {
            if (!rr_simulation_entity_alive(this, player_info->spectate_target) ||
                dev_cheat_enabled(this, player_info->spectate_target, invisible))
            {
                if (player_info->spectate_ticks > 25)
                    player_info->spectate_ticks = 25;
            }
            else if (is_dead_flower(this, player_info->spectate_target))
            {
                if (player_info->spectate_ticks > 62)
                    player_info->spectate_ticks = 62;
            }
        }
        if (--player_info->spectate_ticks == 0)
        {
            EntityHash target_vector[RR_SQUAD_MEMBER_COUNT];
            uint8_t target_count = 0;
            for (uint32_t i = 0; i < this->player_info_count; ++i)
            {
                struct rr_component_player_info *p_info =
                    rr_simulation_get_player_info(
                        this, this->player_info_vector[i]);
                if (p_info->squad != player_info->squad)
                    continue;
                if (p_info->flower_id == RR_NULL_ENTITY)
                    continue;
                if (dev_cheat_enabled(this, p_info->flower_id, invisible) &&
                    p_info->flower_id != player_info->flower_id)
                    continue;
                if (p_info->flower_id == player_info->spectate_target)
                    continue;
                target_vector[target_count++] = p_info->flower_id;
            }
            if (target_count > 0)
            {
                if (player_info->spectating_single_target)
                {
                    player_info->spectate_ticks = 25;
                    player_info->spectating_single_target = 0;
                }
                else
                {
                    rr_component_player_info_set_spectate_target(
                        player_info, target_vector[rand() % target_count]);
                    if (is_dead_flower(this, player_info->spectate_target))
                        player_info->spectate_ticks = 62;
                    else
                        player_info->spectate_ticks = 125;
                }
            }
            else
            {
                player_info->spectate_ticks = 1;
                player_info->spectating_single_target = 1;
            }
        }
        if (rr_simulation_entity_alive(this, player_info->spectate_target) &&
            (!dev_cheat_enabled(this, player_info->spectate_target, invisible) ||
             player_info->spectate_target == player_info->flower_id))
        {
            struct rr_component_physical *physical =
                rr_simulation_get_physical(this, player_info->spectate_target);
            rr_component_player_info_set_camera_x(player_info, physical->x);
            rr_component_player_info_set_camera_y(player_info, physical->y);
            rr_component_player_info_set_arena(player_info, physical->arena);
        }
    }
}
