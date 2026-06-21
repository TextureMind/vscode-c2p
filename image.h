#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#define AGF_IMAGE_DEPTH_8 8

typedef void (*AGFFreeFunc)(void *ptr);

typedef struct AGFImage
{
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t stride;
    uint8_t *data;
    AGFFreeFunc free_func;
} AGFImage;

void agf_image_free(AGFImage *image);

#define agf_image_alloc(width, height, depth) \
    agf_image_alloc_with_free((width), (height), (depth), free)

#define agf_image_alloc_8(width, height) \
    agf_image_alloc((width), (height), AGF_IMAGE_DEPTH_8)

uint32_t agf_image_size(const AGFImage *image);
AGFImage *agf_image_alloc_with_free(uint16_t width, uint16_t height, uint16_t depth, AGFFreeFunc free_func);

#endif
