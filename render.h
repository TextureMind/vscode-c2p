#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "image.h"

#define AGF_DEPTH_MAX 0xffffffffUL
#define AGF_NORMAL_SCALE 16384

typedef struct AGFDepthBuffer
{
    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_stride;
    uint32_t *m_data;
    AGFFreeFunc m_free_func;
} AGFDepthBuffer;

typedef struct AGFVertex3i
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
} AGFVertex3i;

typedef struct AGFVertex3n
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
    int16_t m_nx;
    int16_t m_ny;
    int16_t m_nz;
} AGFVertex3n;

typedef struct AGFDirectionalLight
{
    int16_t m_nx;
    int16_t m_ny;
    int16_t m_nz;
    uint8_t m_ambient;
    uint8_t m_intensity;
} AGFDirectionalLight;

#define agf_depth_buffer_alloc(width, height) \
    agf_depth_buffer_alloc_with_free((width), (height), free)

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t p_width, uint16_t p_height, AGFFreeFunc p_free_func);
void agf_depth_buffer_free(AGFDepthBuffer *p_buffer);
void agf_depth_buffer_clear(AGFDepthBuffer *p_buffer, uint32_t p_depth);
int agf_draw_triangle_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_v0, const AGFVertex3i *p_v1, const AGFVertex3i *p_v2, uint8_t p_color);
int agf_draw_polygon_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_vertices, uint16_t p_count, uint8_t p_color);
uint8_t agf_light_flat_color(const AGFDirectionalLight *p_light, const AGFVertex3n *p_vertices, uint16_t p_count);
int agf_draw_polygon_lit_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3n *p_vertices, uint16_t p_count, const AGFDirectionalLight *p_light);

#endif
