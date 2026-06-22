#include "render.h"

#include <stdlib_headers.h>

static int agf_render_target_valid(const AGFImage *image, const AGFDepthBuffer *depth)
{
    return image != NULL && depth != NULL && image->data != NULL && depth->data != NULL &&
           image->depth == AGF_IMAGE_DEPTH_8 && image->width == depth->width && image->height == depth->height;
}

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t width, uint16_t height, AGFFreeFunc free_func)
{
    AGFDepthBuffer *buffer;
    uint32_t buffer_size;

    if (width == 0 || height == 0 || free_func == NULL) {
        return NULL;
    }

    buffer = (AGFDepthBuffer *)malloc(sizeof(AGFDepthBuffer));
    if (buffer == NULL) {
        return NULL;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->stride = width;
    buffer->free_func = free_func;

    buffer_size = (uint32_t)width * (uint32_t)height * sizeof(uint32_t);
    buffer->data = (uint32_t *)malloc(buffer_size);

    if (buffer->data == NULL) {
        free_func(buffer);
        return NULL;
    }

    agf_depth_buffer_clear(buffer, AGF_DEPTH_MAX);

    return buffer;
}

void agf_depth_buffer_free(AGFDepthBuffer *buffer)
{
    if (buffer != NULL && buffer->free_func != NULL) {
        buffer->free_func(buffer->data);
        buffer->free_func(buffer);
    }
}

void agf_depth_buffer_clear(AGFDepthBuffer *buffer, uint32_t depth)
{
    uint32_t i;
    uint32_t count;

    if (buffer == NULL || buffer->data == NULL) {
        return;
    }

    count = (uint32_t)buffer->stride * (uint32_t)buffer->height;
    for (i = 0; i < count; i++) {
        buffer->data[i] = depth;
    }
}

static int32_t agf_edge_function(const AGFVertex3i *a, const AGFVertex3i *b, int32_t x, int32_t y)
{
    return (x - (int32_t)a->x) * ((int32_t)b->y - (int32_t)a->y) -
           (y - (int32_t)a->y) * ((int32_t)b->x - (int32_t)a->x);
}

int agf_draw_triangle_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *v0, const AGFVertex3i *v1, const AGFVertex3i *v2, uint8_t color)
{
    int32_t min_x;
    int32_t min_y;
    int32_t max_x;
    int32_t max_y;
    int32_t area;
    int32_t x;
    int32_t y;
    uint32_t triangle_depth;

    if (!agf_render_target_valid(image, depth) || v0 == NULL || v1 == NULL || v2 == NULL) {
        return 0;
    }

    min_x = v0->x;
    if (v1->x < min_x) min_x = v1->x;
    if (v2->x < min_x) min_x = v2->x;

    max_x = v0->x;
    if (v1->x > max_x) max_x = v1->x;
    if (v2->x > max_x) max_x = v2->x;

    min_y = v0->y;
    if (v1->y < min_y) min_y = v1->y;
    if (v2->y < min_y) min_y = v2->y;

    max_y = v0->y;
    if (v1->y > max_y) max_y = v1->y;
    if (v2->y > max_y) max_y = v2->y;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= image->width) max_x = image->width - 1;
    if (max_y >= image->height) max_y = image->height - 1;

    if (min_x > max_x || min_y > max_y) {
        return 1;
    }

    area = agf_edge_function(v0, v1, v2->x, v2->y);
    if (area == 0) {
        return 1;
    }

    if (area < 0) {
        const AGFVertex3i *tmp = v1;
        v1 = v2;
        v2 = tmp;
        area = -area;
    }

    triangle_depth = (v0->z / 3) + (v1->z / 3) + (v2->z / 3);

    for (y = min_y; y <= max_y; y++) {
        uint8_t *dst = &image->data[y * image->stride];
        uint32_t *zbuf = &depth->data[y * depth->stride];

        for (x = min_x; x <= max_x; x++) {
            int32_t w0 = agf_edge_function(v1, v2, x, y);
            int32_t w1 = agf_edge_function(v2, v0, x, y);
            int32_t w2 = agf_edge_function(v0, v1, x, y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                if (triangle_depth < zbuf[x]) {
                    zbuf[x] = triangle_depth;
                    dst[x] = color;
                }
            }
        }
    }

    return 1;
}

int agf_draw_polygon_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *vertices, uint16_t count, uint8_t color)
{
    uint16_t i;

    if (vertices == NULL || count < 3) {
        return 0;
    }

    for (i = 1; i + 1 < count; i++) {
        if (!agf_draw_triangle_flat(image, depth, &vertices[0], &vertices[i], &vertices[i + 1], color)) {
            return 0;
        }
    }

    return 1;
}
