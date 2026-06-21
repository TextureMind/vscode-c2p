#include "image.h"

#include <stdlib_headers.h>

uint32_t ImageSize(const SImage *image)
{
    if (image == NULL) {
        return 0;
    }

    return (uint32_t)image->stride * image->height;
}

SImage *ImageAlloc(uint16_t width, uint16_t height, uint16_t depth)
{
    SImage *image;

    if (width == 0 || height == 0 || depth != IMAGE_DEPTH_8) {
        return NULL;
    }

    image = (SImage *)malloc(sizeof(SImage));
    if (image == NULL) {
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->depth = depth;
    image->stride = width;
    image->data = (uint8_t *)malloc(ImageSize(image));

    if (image->data == NULL) {
        free(image);
        return NULL;
    }

    memset(image->data, 0, ImageSize(image));

    return image;
}

SImage *ImageAlloc8(uint16_t width, uint16_t height)
{
    return ImageAlloc(width, height, IMAGE_DEPTH_8);
}

void ImageFree(SImage *image)
{
    if (image != NULL) {
        free(image->data);
        free(image);
    }
}
