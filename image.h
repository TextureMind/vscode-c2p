#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#define AGF_IMAGE_DEPTH_8 8

typedef struct AGFImage
{
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t stride;
    uint8_t *data;
} AGFImage;

#define agf_image_free(image) \
    do { \
        if ((image) != NULL) { \
            free((image)->data); \
            free(image); \
        } \
    } while (0)

uint32_t agf_image_size(const AGFImage *image);
AGFImage *agf_image_alloc(uint16_t width, uint16_t height, uint16_t depth);
AGFImage *agf_image_alloc_8(uint16_t width, uint16_t height);

#endif
