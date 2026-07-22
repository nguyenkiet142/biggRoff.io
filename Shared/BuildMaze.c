/* ════════════════════════════════════════════════════════════════════
 *  BuildMaze.c — SHARED maze-building + hit-quad logic (FINAL VERSION)
 * ════════════════════════════════════════════════════════════════════ */

#include <stddef.h>
#include <stdint.h>
#include <Shared/BuildMaze.h>
#include <Shared/StaticData.h>

/* ── HIT_QUAD: [TL,TR,BL,BR] 1 = solid quadrant ──
 *  Đã fix lỗi đảo LEFT/RIGHT. Cứ copy bảng từ Web Editor đè lên đây.         */
static const char CW_HC_HITQUAD[CW_COUNT][4] = {
    {1,1,1,1},{0,0,1,1},{1,1,0,0},{0,1,0,1},{1,0,1,0},
    {1,1,1,0},{1,1,0,1},{1,0,1,1},{0,1,1,1},
    {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1},
    {1,1,1,1},
};
static const char CW_GA_HITQUAD[CW_COUNT][4] = {
    {1,1,1,1},{0,0,1,1},{1,1,0,0},{0,1,0,1},{1,0,1,0},
    {1,1,1,0},{1,1,0,1},{1,0,1,1},{0,1,1,1},
    {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1},
    {1,1,1,1},
};
static const char CW_OC_HITQUAD[CW_COUNT][4] = {
    {1,1,1,1},{0,0,1,1},{1,1,0,0},{0,1,0,1},{1,0,1,0},
    {1,1,1,0},{1,1,0,1},{1,0,1,1},{0,1,1,1},
    {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1},
    {1,1,1,1},
};

const char (*cw_get_hitquad(uint8_t biome))[4]
{
    switch (biome)
    {
    case rr_biome_id_hell_creek: return CW_HC_HITQUAD;
    case rr_biome_id_garden:     return CW_GA_HITQUAD;
    case rr_biome_id_ocean:      return CW_OC_HITQUAD;
    default:                     return NULL;
    }
}

int cw_tmpl_iswall(struct rr_maze_grid *maze, uint32_t dim, int tx, int ty)
{
    if (tx < 0 || ty < 0 || (uint32_t)tx * 2 >= dim || (uint32_t)ty * 2 >= dim)
        return 1;
    return maze[(ty * 2) * dim + (tx * 2)].difficulty == 0;
}

int cw_pick_variant(struct rr_maze_grid *maze, uint32_t dim, int tx, int ty)
{
    int isw = cw_tmpl_iswall(maze, dim, tx, ty);
    int t = cw_tmpl_iswall(maze, dim, tx, ty - 1);
    int b = cw_tmpl_iswall(maze, dim, tx, ty + 1);
    int l = cw_tmpl_iswall(maze, dim, tx - 1, ty);
    int rr = cw_tmpl_iswall(maze, dim, tx + 1, ty);

    /* FIX: ISOLATED wall (không nối tile nào) → trả CW_ISOLATED ngay */
    if (isw && !t && !b && !l && !rr)
        return CW_ISOLATED;

    int tl = cw_tmpl_iswall(maze, dim, tx - 1, ty - 1);
    int tr = cw_tmpl_iswall(maze, dim, tx + 1, ty - 1);
    int bl = cw_tmpl_iswall(maze, dim, tx - 1, ty + 1);
    int br = cw_tmpl_iswall(maze, dim, tx + 1, ty + 1);

    int qTL, qTR, qBL, qBR;
    if (isw)
    {
        qTL = (t && l) || tl; qTR = (t && rr) || tr;
        qBL = (b && l) || bl; qBR = (b && rr) || br;
    }
    else
    {
        qTL = (t && l); qTR = (t && rr);
        qBL = (b && l); qBR = (b && rr);
    }

    int pat = qTL * 8 + qTR * 4 + qBL * 2 + qBR;
    switch (pat)
    {
    case 15: return CW_CENTER;          /* wall đặc → CENTER */
    case 3:  return CW_TOP;
    case 12: return CW_BOTTOM;
    case 5:  return CW_LEFT;
    case 10: return CW_RIGHT;
    case 14: return CW_TOP_LEFT;
    case 13: return CW_TOP_RIGHT;
    case 11: return CW_BOTTOM_LEFT;
    case 7:  return CW_BOTTOM_RIGHT;
    case 8:  return CW_TOP_LEFT_INNER;
    case 4:  return CW_TOP_RIGHT_INNER;
    case 2:  return CW_BOTTOM_LEFT_INNER;
    case 1:  return CW_BOTTOM_RIGHT_INNER;
    default: return CW_CENTER;
    }
}

void build_custom_maze(uint8_t biome, struct rr_maze_grid *maze,
                       uint32_t maze_dim)
{
    const char (*hq)[4] = cw_get_hitquad(biome);
    if (!hq) return;

    uint32_t tdim = maze_dim / 2;
    for (int ty = 0; ty < (int)tdim; ++ty)
    {
        for (int tx = 0; tx < (int)tdim; ++tx)
        {
            int isw = cw_tmpl_iswall(maze, maze_dim, tx, ty);
            int t = cw_tmpl_iswall(maze, maze_dim, tx, ty - 1);
            int b = cw_tmpl_iswall(maze, maze_dim, tx, ty + 1);
            int l = cw_tmpl_iswall(maze, maze_dim, tx - 1, ty);
            int rr = cw_tmpl_iswall(maze, maze_dim, tx + 1, ty);

            if (isw)
            {
                int variant = cw_pick_variant(maze, maze_dim, tx, ty);
                const char *q = hq[variant];
                maze[(ty * 2) * maze_dim + (tx * 2)].value         = q[0] ? 0 : 1;
                maze[(ty * 2) * maze_dim + (tx * 2 + 1)].value     = q[1] ? 0 : 1;
                maze[(ty * 2 + 1) * maze_dim + (tx * 2)].value     = q[2] ? 0 : 1;
                maze[(ty * 2 + 1) * maze_dim + (tx * 2 + 1)].value = q[3] ? 0 : 1;
            }
            else
            {
                /* FIX: FLOOR cell → tính trực tiếp */
                maze[(ty * 2) * maze_dim + (tx * 2)].value         = (t && l) ? 0 : 1;
                maze[(ty * 2) * maze_dim + (tx * 2 + 1)].value     = (t && rr) ? 0 : 1;
                maze[(ty * 2 + 1) * maze_dim + (tx * 2)].value     = (b && l) ? 0 : 1;
                maze[(ty * 2 + 1) * maze_dim + (tx * 2 + 1)].value = (b && rr) ? 0 : 1;
            }
        }
    }
}

int spawn_is_safe(struct rr_maze_grid *maze, uint32_t dim, int sx, int sy)
{
    if (sx < 0 || sy < 0 || (uint32_t)sx >= dim || (uint32_t)sy >= dim)
        return 0;
    return maze[sy * dim + sx].value != 0;
}
