/* ════════════════════════════════════════════════════════════════════
 *  CustomWall.c — BẢN FINAL CHUẨN (Gọi Layer + Cut khung)
 *  File này gọi trực tiếp các hàm Art (rr_draw_*_wall_*) từ file Export
 * ════════════════════════════════════════════════════════════════════ */

#include <Client/Renderer/Renderer.h>
#include <Client/Assets/Render.h>   /* ← Khai báo hàm rr_draw_hc_wall_* */
#include <math.h>
#include <Shared/BuildMaze.h>
#include <Shared/StaticData.h>

/* ── HELL CREEK walls (Gọi layer từ HellCreekWall.c) ── */
void cw_hc_draw(int variant, struct rr_renderer *r)
{
    switch (variant)
    {
    case CW_CENTER:             rr_draw_hc_wall_center(r);             break;
    case CW_TOP:                rr_draw_hc_wall_top(r);                break;
    case CW_BOTTOM:             rr_draw_hc_wall_bottom(r);             break;
    case CW_LEFT:               rr_draw_hc_wall_left(r);               break;
    case CW_RIGHT:              rr_draw_hc_wall_right(r);              break;
    case CW_TOP_LEFT:           rr_draw_hc_wall_top_left(r);           break;
    case CW_TOP_RIGHT:          rr_draw_hc_wall_top_right(r);          break;
    case CW_BOTTOM_LEFT:        rr_draw_hc_wall_bottom_left(r);        break;
    case CW_BOTTOM_RIGHT:       rr_draw_hc_wall_bottom_right(r);       break;
    case CW_TOP_LEFT_INNER:     rr_draw_hc_wall_top_left_inner(r);     break;
    case CW_TOP_RIGHT_INNER:    rr_draw_hc_wall_top_right_inner(r);    break;
    case CW_BOTTOM_LEFT_INNER:  rr_draw_hc_wall_bottom_left_inner(r);  break;
    case CW_BOTTOM_RIGHT_INNER: rr_draw_hc_wall_bottom_right_inner(r); break;
    case CW_ISOLATED:           rr_draw_hc_wall_isolated(r);           break;
    }
}

/* ── GARDEN walls (Gọi layer từ GardenWall.c) ── */
void cw_ga_draw(int variant, struct rr_renderer *r)
{
    switch (variant)
    {
    case CW_CENTER:             rr_draw_ga_wall_center(r);             break;
    case CW_TOP:                rr_draw_ga_wall_top(r);                break;
    case CW_BOTTOM:             rr_draw_ga_wall_bottom(r);             break;
    case CW_LEFT:               rr_draw_ga_wall_left(r);               break;
    case CW_RIGHT:              rr_draw_ga_wall_right(r);              break;
    case CW_TOP_LEFT:           rr_draw_ga_wall_top_left(r);           break;
    case CW_TOP_RIGHT:          rr_draw_ga_wall_top_right(r);          break;
    case CW_BOTTOM_LEFT:        rr_draw_ga_wall_bottom_left(r);        break;
    case CW_BOTTOM_RIGHT:       rr_draw_ga_wall_bottom_right(r);       break;
    case CW_TOP_LEFT_INNER:     rr_draw_ga_wall_top_left_inner(r);     break;
    case CW_TOP_RIGHT_INNER:    rr_draw_ga_wall_top_right_inner(r);    break;
    case CW_BOTTOM_LEFT_INNER:  rr_draw_ga_wall_bottom_left_inner(r);  break;
    case CW_BOTTOM_RIGHT_INNER: rr_draw_ga_wall_bottom_right_inner(r); break;
    case CW_ISOLATED:           rr_draw_ga_wall_isolated(r);           break;
    }
}

/* ── OCEAN walls (Placeholder) ── */
void cw_oc_draw(int variant, struct rr_renderer *r)
{
    rr_renderer_set_fill(r, 0xff2a5a6e);
    rr_renderer_fill_rect(r, -128, -128, 256, 256);
}

/* ═══ DISPATCH + RENDER (Đã thêm lệnh CLIP chống tràn layer) ═══ */
int cw_biome_uses_custom(uint8_t biome)
{
    switch (biome)
    {
    case rr_biome_id_hell_creek: return 1;
    case rr_biome_id_garden:     return 1;
    case rr_biome_id_ocean:      return 1;
    default:                     return 0;
    }
}

typedef void (*cw_draw_fn)(int variant, struct rr_renderer *r);
cw_draw_fn cw_get_draw(uint8_t biome)
{
    switch (biome)
    {
    case rr_biome_id_hell_creek: return cw_hc_draw;
    case rr_biome_id_garden:     return cw_ga_draw;
    case rr_biome_id_ocean:      return cw_oc_draw;
    default:                     return NULL;
    }
}

void render_custom_map(struct rr_renderer *r, uint8_t biome,
                       struct rr_maze_grid *grid, uint32_t maze_dim,
                       float grid_size, double leftX, double topY,
                       double rightX, double bottomY)
{
    cw_draw_fn draw = cw_get_draw(biome);
    if (!draw) return;

    uint32_t tdim = maze_dim / 2;
    float cell = grid_size * 2.0f;
    int x0 = (int)floor(leftX / cell);
    int y0 = (int)floor(topY / cell);
    int x1 = (int)ceil(rightX / cell);
    int y1 = (int)ceil(bottomY / cell);

    for (int ty = y0; ty < y1; ++ty)
    {
        for (int tx = x0; tx < x1; ++tx)
        {
            if (tx < 0 || ty < 0 || (uint32_t)tx >= tdim ||
                (uint32_t)ty >= tdim)
                continue;

            int isw = cw_tmpl_iswall(grid, maze_dim, tx, ty);
            int variant = cw_pick_variant(grid, maze_dim, tx, ty);

            struct rr_renderer_context_state st;
            rr_renderer_context_state_init(r, &st);
            rr_renderer_translate(r, tx * cell + cell / 2,
                                  ty * cell + cell / 2);
            rr_renderer_scale(r, cell / 256.0f);

            /* ⭐ CLIP: Cắt khung 256×256 để hình không bị tràn viền */
            rr_renderer_begin_path(r);
            rr_renderer_rect(r, -128, -128, 256, 256);
            rr_renderer_clip(r);

            if (isw)
                draw(variant, r);
            else
            {
                int t = cw_tmpl_iswall(grid, maze_dim, tx, ty - 1);
                int b = cw_tmpl_iswall(grid, maze_dim, tx, ty + 1);
                int l = cw_tmpl_iswall(grid, maze_dim, tx - 1, ty);
                int rr = cw_tmpl_iswall(grid, maze_dim, tx + 1, ty);
                if (t && l)  draw(CW_TOP_LEFT_INNER, r);
                if (t && rr) draw(CW_TOP_RIGHT_INNER, r);
                if (b && l)  draw(CW_BOTTOM_LEFT_INNER, r);
                if (b && rr) draw(CW_BOTTOM_RIGHT_INNER, r);
            }
            rr_renderer_context_state_free(r, &st);
        }
    }
}
