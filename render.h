#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "image.h"

#define AGF_DEPTH_MAX 0xffffffffUL

typedef struct AGFDepthBuffer
{
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint32_t *data;
    AGFFreeFunc free_func;
} AGFDepthBuffer;

typedef struct AGFVertex3i
{
    int16_t x;
    int16_t y;
    uint32_t z;
} AGFVertex3i;

#define agf_depth_buffer_alloc(width, height) \
    agf_depth_buffer_alloc_with_free((width), (height), free)

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t width, uint16_t height, AGFFreeFunc free_func);
void agf_depth_buffer_free(AGFDepthBuffer *buffer);
void agf_depth_buffer_clear(AGFDepthBuffer *buffer, uint32_t depth);
int agf_draw_triangle_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *v0, const AGFVertex3i *v1, const AGFVertex3i *v2, uint8_t color);
int agf_draw_polygon_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *vertices, uint16_t count, uint8_t color);

#endif
