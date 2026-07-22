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

#pragma once

#include <stdint.h>

struct rr_maze_grid;
struct rr_renderer;

void rr_triceratops_head_draw(struct rr_renderer *);
void rr_triceratops_body_draw(struct rr_renderer *);
void rr_triceratops_leg1_draw(struct rr_renderer *);
void rr_triceratops_leg2_draw(struct rr_renderer *);
void rr_triceratops_tail_draw(struct rr_renderer *);

void rr_pteranodon_wing1_draw(struct rr_renderer *);
void rr_pteranodon_wing2_draw(struct rr_renderer *);
void rr_pteranodon_body_draw(struct rr_renderer *);

void rr_t_rex_body_draw(struct rr_renderer *);
void rr_t_rex_leg1_draw(struct rr_renderer *);
void rr_t_rex_leg2_draw(struct rr_renderer *);
void rr_t_rex_head_draw(struct rr_renderer *);
void rr_t_rex_tail_draw(struct rr_renderer *);

void rr_dakotaraptor_body_draw(struct rr_renderer *);
void rr_dakotaraptor_wing1_draw(struct rr_renderer *);
void rr_dakotaraptor_wing2_draw(struct rr_renderer *);
void rr_dakotaraptor_head_draw(struct rr_renderer *);
void rr_dakotaraptor_tail_draw(struct rr_renderer *);

void rr_fern_draw(struct rr_renderer *);
void rr_tree_draw(struct rr_renderer *);
void rr_meteor_draw(struct rr_renderer *);
void rr_beehive_draw(struct rr_renderer *);

void rr_pachycephalosaurus_head_draw(struct rr_renderer *);
void rr_pachycephalosaurus_body_draw(struct rr_renderer *);
void rr_pachycephalosaurus_leg1_draw(struct rr_renderer *);
void rr_pachycephalosaurus_leg2_draw(struct rr_renderer *);
void rr_pachycephalosaurus_tail_draw(struct rr_renderer *);

void rr_ornithomimus_body_draw(struct rr_renderer *);
void rr_ornithomimus_wing1_draw(struct rr_renderer *);
void rr_ornithomimus_wing2_draw(struct rr_renderer *);
void rr_ornithomimus_head_draw(struct rr_renderer *);
void rr_ornithomimus_tail_draw(struct rr_renderer *);

void rr_ankylosaurus_head_draw(struct rr_renderer *);
void rr_ankylosaurus_body_draw(struct rr_renderer *);
void rr_ankylosaurus_tail_draw(struct rr_renderer *);

void rr_quetzalcoatlus_body_draw(struct rr_renderer *);
void rr_quetzalcoatlus_head_draw(struct rr_renderer *);
void rr_quetzalcoatlus_wing1_draw(struct rr_renderer *);
void rr_quetzalcoatlus_wing2_draw(struct rr_renderer *);

void rr_edmontosaurus_body_draw(struct rr_renderer *);
void rr_edmontosaurus_leg1_draw(struct rr_renderer *);
void rr_edmontosaurus_leg2_draw(struct rr_renderer *);
void rr_edmontosaurus_head_draw(struct rr_renderer *);
void rr_edmontosaurus_tail_draw(struct rr_renderer *);

void rr_ant_abdomen_draw(struct rr_renderer *);
void rr_ant_head_draw(struct rr_renderer *);
void rr_ant_thorax_draw(struct rr_renderer *);
void rr_ant_leg_draw(struct rr_renderer *);

void rr_hornet_abdomen_draw(struct rr_renderer *);
void rr_hornet_head_draw(struct rr_renderer *);
void rr_hornet_thorax_draw(struct rr_renderer *);
void rr_hornet_leg_draw(struct rr_renderer *);
void rr_hornet_wing_draw(struct rr_renderer *);

void rr_dragonfly_abdomen_draw(struct rr_renderer *);
void rr_dragonfly_head_draw(struct rr_renderer *);
void rr_dragonfly_thorax_draw(struct rr_renderer *);
void rr_dragonfly_wing_draw(struct rr_renderer *);

void rr_honeybee_abdomen_draw(struct rr_renderer *);
void rr_honeybee_thorax_draw(struct rr_renderer *);
void rr_honeybee_head_draw(struct rr_renderer *);
void rr_honeybee_leg_draw(struct rr_renderer *);
void rr_honeybee_wing_draw(struct rr_renderer *);

void rr_spider_abdomen_draw(struct rr_renderer *);
void rr_spider_head_draw(struct rr_renderer *);
void rr_spider_leg_draw(struct rr_renderer *);

void rr_house_centipede_body_draw(struct rr_renderer *);
void rr_house_centipede_head_draw(struct rr_renderer *);
void rr_house_centipede_leg_draw(struct rr_renderer *);
void rr_lanternfly_wing1_draw(struct rr_renderer *);
void rr_lanternfly_abdomen_draw(struct rr_renderer *);
void rr_lanternfly_leg_draw(struct rr_renderer *);

void rr_sandstone_outer_draw(struct rr_renderer *);
void rr_sandstone_middle_draw(struct rr_renderer *);
void rr_sandstone_inner_draw(struct rr_renderer *);

void rr_kelp_draw(struct rr_renderer *);
void rr_king_mackarel_body_draw(struct rr_renderer *);
void rr_king_mackarel_fin1_draw(struct rr_renderer *);
void rr_king_mackarel_fin2_draw(struct rr_renderer *);
void rr_king_mackarel_head_draw(struct rr_renderer *);
void rr_king_mackarel_tail_draw(struct rr_renderer *);
void rr_seagull_wing1_draw(struct rr_renderer *);
void rr_seagull_wing2_draw(struct rr_renderer *);
void rr_seagull_body_draw(struct rr_renderer *);
void rr_sea_snail_body_draw(struct rr_renderer *);
void rr_sea_snail_head_draw(struct rr_renderer *);
void rr_pectinodon_body_draw(struct rr_renderer *);
void rr_pectinodon_head_draw(struct rr_renderer *);
void rr_pectinodon_tail_draw(struct rr_renderer *);
void rr_pectinodon_wing1_draw(struct rr_renderer *);
void rr_pectinodon_wing2_draw(struct rr_renderer *);

void rr_hc_tile_1_draw(struct rr_renderer *);
void rr_hc_tile_2_draw(struct rr_renderer *);
void rr_hc_tile_3_draw(struct rr_renderer *);

void rr_ga_tile_1_draw(struct rr_renderer *);
void rr_ga_tile_2_draw(struct rr_renderer *);
void rr_ga_tile_3_draw(struct rr_renderer *);

void rr_oc_tile_1_draw(struct rr_renderer *);
void rr_oc_tile_2_draw(struct rr_renderer *);
void rr_oc_tile_3_draw(struct rr_renderer *);

void rr_prop_fern_draw(struct rr_renderer *);
void rr_prop_moss_draw(struct rr_renderer *);
void rr_prop_water_lettuce_draw(struct rr_renderer *);
void rr_prop_palm_tree_draw(struct rr_renderer *);
void rr_prop_beech_tree_draw(struct rr_renderer *);

// CustomWall.c prototypes
void render_custom_map(struct rr_renderer *r, uint8_t biome,
                       struct rr_maze_grid *grid, uint32_t maze_dim,
                       float grid_size, double leftX, double topY,
                       double rightX, double bottomY);
int  cw_biome_uses_custom(uint8_t biome);
void cw_hc_draw(int variant, struct rr_renderer *r);
void cw_ga_draw(int variant, struct rr_renderer *r);
void cw_oc_draw(int variant, struct rr_renderer *r);

// Hell Creek wall art (HellCreekWall.c)
void rr_draw_hc_wall_center(struct rr_renderer *);
void rr_draw_hc_wall_top(struct rr_renderer *);
void rr_draw_hc_wall_bottom(struct rr_renderer *);
void rr_draw_hc_wall_left(struct rr_renderer *);
void rr_draw_hc_wall_right(struct rr_renderer *);
void rr_draw_hc_wall_top_left(struct rr_renderer *);
void rr_draw_hc_wall_top_right(struct rr_renderer *);
void rr_draw_hc_wall_bottom_left(struct rr_renderer *);
void rr_draw_hc_wall_bottom_right(struct rr_renderer *);
void rr_draw_hc_wall_top_left_inner(struct rr_renderer *);
void rr_draw_hc_wall_top_right_inner(struct rr_renderer *);
void rr_draw_hc_wall_bottom_left_inner(struct rr_renderer *);
void rr_draw_hc_wall_bottom_right_inner(struct rr_renderer *);
void rr_draw_hc_wall_isolated(struct rr_renderer *);

// Garden wall art (GardenWall.c)
void rr_draw_ga_wall_center(struct rr_renderer *);
void rr_draw_ga_wall_top(struct rr_renderer *);
void rr_draw_ga_wall_bottom(struct rr_renderer *);
void rr_draw_ga_wall_left(struct rr_renderer *);
void rr_draw_ga_wall_right(struct rr_renderer *);
void rr_draw_ga_wall_top_left(struct rr_renderer *);
void rr_draw_ga_wall_top_right(struct rr_renderer *);
void rr_draw_ga_wall_bottom_left(struct rr_renderer *);
void rr_draw_ga_wall_bottom_right(struct rr_renderer *);
void rr_draw_ga_wall_top_left_inner(struct rr_renderer *);
void rr_draw_ga_wall_top_right_inner(struct rr_renderer *);
void rr_draw_ga_wall_bottom_left_inner(struct rr_renderer *);
void rr_draw_ga_wall_bottom_right_inner(struct rr_renderer *);
void rr_draw_ga_wall_isolated(struct rr_renderer *);