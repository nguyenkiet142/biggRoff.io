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

#include <Client/System/ParticleRender.h>

#include <Client/Game.h>

void
rr_system_particle_render_tick(struct rr_game *game,
                               struct rr_particle_manager *particle_manager,
                               float delta)
{
    for (uint16_t i = 0; i < particle_manager->size; ++i)
    {
        struct rr_simulation_animation *particle =
            &particle_manager->particles[i];
        if (!game->cache.low_performance_mode)
            rr_renderer_render_particle(game->renderer, particle);
        particle->opacity = rr_lerp(particle->opacity, 0,
                                    particle->disappearance * delta);
        if (particle->type != rr_animation_type_lightningbolt)
        {
            rr_vector_scale(&particle->velocity, particle->friction);
            rr_vector_add(&particle->velocity, &particle->acceleration);
            particle->x += particle->velocity.x;
            particle->y += particle->velocity.y;
        }
    }
    for (uint16_t i = particle_manager->size; i > 0; --i)
    {
        if (particle_manager->particles[i - 1].opacity < 0.01)
            rr_particle_delete(particle_manager,
                               &particle_manager->particles[i - 1]);
    }
}