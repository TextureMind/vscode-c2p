#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "image.h"

#define AGF_DEPTH_MAX 0xffffffffUL
#define AGF_NORMAL_SCALE 16384

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

typedef struct AGFVertex3n
{
    int16_t x;
    int16_t y;
    uint32_t z;
    int16_t nx;
    int16_t ny;
    int16_t nz;
} AGFVertex3n;

typedef struct AGFDirectionalLight
{
    int16_t nx;
    int16_t ny;
    int16_t nz;
    uint8_t ambient;
    uint8_t intensity;
} AGFDirectionalLight;

#define agf_depth_buffer_alloc(width, height) \
    agf_depth_buffer_alloc_with_free((width), (height), free)

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t width, uint16_t height, AGFFreeFunc free_func);
void agf_depth_buffer_free(AGFDepthBuffer *buffer);
void agf_depth_buffer_clear(AGFDepthBuffer *buffer, uint32_t depth);
int agf_draw_triangle_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *v0, const AGFVertex3i *v1, const AGFVertex3i *v2, uint8_t color);
int agf_draw_polygon_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3i *vertices, uint16_t count, uint8_t color);
uint8_t agf_light_flat_color(const AGFDirectionalLight *light, const AGFVertex3n *vertices, uint16_t count);
int agf_draw_polygon_lit_flat(AGFImage *image, AGFDepthBuffer *depth, const AGFVertex3n *vertices, uint16_t count, const AGFDirectionalLight *light);

#endif
