#include "render.h"

#include <stdlib_headers.h>

static int agf_render_target_valid(const AGFImage *p_image, const AGFDepthBuffer *p_depth)
{
    return p_image != NULL && p_depth != NULL && p_image->m_data != NULL && p_depth->m_data != NULL &&
           p_image->m_depth == AGF_IMAGE_DEPTH_8 && p_image->m_width == p_depth->m_width && p_image->m_height == p_depth->m_height;
}

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t p_width, uint16_t p_height, AGFFreeFunc p_free_func)
{
    AGFDepthBuffer *buffer;
    uint32_t buffer_size;

    if (p_width == 0 || p_height == 0 || p_free_func == NULL) {
        return NULL;
    }

    buffer = (AGFDepthBuffer *)malloc(sizeof(AGFDepthBuffer));
    if (buffer == NULL) {
        return NULL;
    }

    buffer->m_width = p_width;
    buffer->m_height = p_height;
    buffer->m_stride = p_width;
    buffer->m_free_func = p_free_func;

    buffer_size = (uint32_t)p_width * (uint32_t)p_height * sizeof(uint32_t);
    buffer->m_data = (uint32_t *)malloc(buffer_size);

    if (buffer->m_data == NULL) {
        p_free_func(buffer);
        return NULL;
    }

    agf_depth_buffer_clear(buffer, AGF_DEPTH_MAX);

    return buffer;
}

void agf_depth_buffer_free(AGFDepthBuffer *p_buffer)
{
    if (p_buffer != NULL && p_buffer->m_free_func != NULL) {
        p_buffer->m_free_func(p_buffer->m_data);
        p_buffer->m_free_func(p_buffer);
    }
}

void agf_depth_buffer_clear(AGFDepthBuffer *p_buffer, uint32_t p_depth)
{
    uint32_t i;
    uint32_t count;

    if (p_buffer == NULL || p_buffer->m_data == NULL) {
        return;
    }

    count = (uint32_t)p_buffer->m_stride * (uint32_t)p_buffer->m_height;
    for (i = 0; i < count; i++) {
        p_buffer->m_data[i] = p_depth;
    }
}

static int32_t agf_edge_function(const AGFVertex3i *p_a, const AGFVertex3i *p_b, int32_t p_x, int32_t p_y)
{
    return (p_x - (int32_t)p_a->m_x) * ((int32_t)p_b->m_y - (int32_t)p_a->m_y) -
           (p_y - (int32_t)p_a->m_y) * ((int32_t)p_b->m_x - (int32_t)p_a->m_x);
}

int agf_draw_triangle_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_v0, const AGFVertex3i *p_v1, const AGFVertex3i *p_v2, uint8_t p_color)
{
    int32_t min_x;
    int32_t min_y;
    int32_t max_x;
    int32_t max_y;
    int32_t area;
    int32_t x;
    int32_t y;
    uint32_t triangle_depth;

    if (!agf_render_target_valid(p_image, p_depth) || p_v0 == NULL || p_v1 == NULL || p_v2 == NULL) {
        return 0;
    }

    min_x = p_v0->m_x;
    if (p_v1->m_x < min_x) min_x = p_v1->m_x;
    if (p_v2->m_x < min_x) min_x = p_v2->m_x;

    max_x = p_v0->m_x;
    if (p_v1->m_x > max_x) max_x = p_v1->m_x;
    if (p_v2->m_x > max_x) max_x = p_v2->m_x;

    min_y = p_v0->m_y;
    if (p_v1->m_y < min_y) min_y = p_v1->m_y;
    if (p_v2->m_y < min_y) min_y = p_v2->m_y;

    max_y = p_v0->m_y;
    if (p_v1->m_y > max_y) max_y = p_v1->m_y;
    if (p_v2->m_y > max_y) max_y = p_v2->m_y;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= p_image->m_width) max_x = p_image->m_width - 1;
    if (max_y >= p_image->m_height) max_y = p_image->m_height - 1;

    if (min_x > max_x || min_y > max_y) {
        return 1;
    }

    area = agf_edge_function(p_v0, p_v1, p_v2->m_x, p_v2->m_y);
    if (area == 0) {
        return 1;
    }

    if (area < 0) {
        const AGFVertex3i *tmp = p_v1;
        p_v1 = p_v2;
        p_v2 = tmp;
        area = -area;
    }

    triangle_depth = (p_v0->m_z / 3) + (p_v1->m_z / 3) + (p_v2->m_z / 3);

    for (y = min_y; y <= max_y; y++) {
        uint8_t *dst = &p_image->m_data[y * p_image->m_stride];
        uint32_t *zbuf = &p_depth->m_data[y * p_depth->m_stride];

        for (x = min_x; x <= max_x; x++) {
            int32_t w0 = agf_edge_function(p_v1, p_v2, x, y);
            int32_t w1 = agf_edge_function(p_v2, p_v0, x, y);
            int32_t w2 = agf_edge_function(p_v0, p_v1, x, y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                if (triangle_depth < zbuf[x]) {
                    zbuf[x] = triangle_depth;
                    dst[x] = p_color;
                }
            }
        }
    }

    return 1;
}

int agf_draw_polygon_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_vertices, uint16_t p_count, uint8_t p_color)
{
    uint16_t i;

    if (p_vertices == NULL || p_count < 3) {
        return 0;
    }

    for (i = 1; i + 1 < p_count; i++) {
        if (!agf_draw_triangle_flat(p_image, p_depth, &p_vertices[0], &p_vertices[i], &p_vertices[i + 1], p_color)) {
            return 0;
        }
    }

    return 1;
}

uint8_t agf_light_flat_color(const AGFDirectionalLight *p_light, const AGFVertex3n *p_vertices, uint16_t p_count)
{
    int32_t nx = 0;
    int32_t ny = 0;
    int32_t nz = 0;
    int32_t dot;
    int32_t color;
    uint16_t i;

    if (p_light == NULL || p_vertices == NULL || p_count == 0) {
        return 0;
    }

    for (i = 0; i < p_count; i++) {
        nx += p_vertices[i].m_nx;
        ny += p_vertices[i].m_ny;
        nz += p_vertices[i].m_nz;
    }

    nx /= p_count;
    ny /= p_count;
    nz /= p_count;

    dot = (nx * p_light->m_nx + ny * p_light->m_ny + nz * p_light->m_nz) / AGF_NORMAL_SCALE;
    if (dot < 0) {
        dot = 0;
    } else if (dot > AGF_NORMAL_SCALE) {
        dot = AGF_NORMAL_SCALE;
    }

    color = p_light->m_ambient + (int32_t)(((uint32_t)p_light->m_intensity * (uint32_t)dot) / AGF_NORMAL_SCALE);
    if (color > 255) {
        color = 255;
    }

    return (uint8_t)color;
}

int agf_draw_polygon_lit_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3n *p_vertices, uint16_t p_count, const AGFDirectionalLight *p_light)
{
    AGFVertex3i flat_vertices[8];
    uint16_t i;

    if (p_vertices == NULL || p_count < 3 || p_count > 8) {
        return 0;
    }

    for (i = 0; i < p_count; i++) {
        flat_vertices[i].m_x = p_vertices[i].m_x;
        flat_vertices[i].m_y = p_vertices[i].m_y;
        flat_vertices[i].m_z = p_vertices[i].m_z;
    }

    return agf_draw_polygon_flat(p_image, p_depth, flat_vertices, p_count, agf_light_flat_color(p_light, p_vertices, p_count));
}
