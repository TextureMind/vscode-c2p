#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#define AGF_IMAGE_DEPTH_8 8

typedef void (*AGFFreeFunc)(void *ptr);

typedef struct AGFImage
{
    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_depth;
    uint16_t m_stride;
    uint8_t *m_data;
    AGFFreeFunc m_free_func;
} AGFImage;

void agf_image_free(AGFImage *image);

#define agf_image_alloc(width, height, depth) \
    agf_image_alloc_with_free((width), (height), (depth), free)

#define agf_image_alloc_8(width, height) \
    agf_image_alloc((width), (height), AGF_IMAGE_DEPTH_8)

uint32_t agf_image_size(const AGFImage *p_image);
AGFImage *agf_image_alloc_with_free(uint16_t p_width, uint16_t p_height, uint16_t p_depth, AGFFreeFunc p_free_func);

#endif
