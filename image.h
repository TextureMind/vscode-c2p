#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#define IMAGE_DEPTH_8 8

typedef struct SImage
{
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t stride;
    uint8_t *data;
} SImage;

SImage *ImageAlloc(uint16_t width, uint16_t height, uint16_t depth);
SImage *ImageAlloc8(uint16_t width, uint16_t height);
void ImageFree(SImage *image);
uint32_t ImageSize(const SImage *image);

#endif
