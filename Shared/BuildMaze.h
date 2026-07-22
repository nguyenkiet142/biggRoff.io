#pragma once

#include <stdint.h>

struct rr_maze_grid;

enum
{
    CW_CENTER = 0, 
    CW_TOP, 
    CW_BOTTOM, 
    CW_LEFT, 
    CW_RIGHT,
    CW_TOP_LEFT, 
    CW_TOP_RIGHT, 
    CW_BOTTOM_LEFT, 
    CW_BOTTOM_RIGHT,
    CW_TOP_LEFT_INNER, 
    CW_TOP_RIGHT_INNER, 
    CW_BOTTOM_LEFT_INNER,
    CW_BOTTOM_RIGHT_INNER, 
    CW_ISOLATED, 
    CW_COUNT
};

int cw_tmpl_iswall(struct rr_maze_grid *maze, uint32_t dim, int tx, int ty);
int cw_pick_variant(struct rr_maze_grid *maze, uint32_t dim, int tx, int ty);
const char (*cw_get_hitquad(uint8_t biome))[4];
void build_custom_maze(uint8_t biome, struct rr_maze_grid *maze, uint32_t maze_dim);
int spawn_is_safe(struct rr_maze_grid *maze, uint32_t dim, int sx, int sy);
