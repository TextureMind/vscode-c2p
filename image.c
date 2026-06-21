#include "image.h"

#include <stdlib_headers.h>

uint32_t agf_image_size(const AGFImage *image)
{
    if (image == NULL) {
        return 0;
    }

    return (uint32_t)image->stride * image->height;
}

AGFImage *agf_image_alloc(uint16_t width, uint16_t height, uint16_t depth)
{
    AGFImage *image;

    if (width == 0 || height == 0 || depth != AGF_IMAGE_DEPTH_8) {
        return NULL;
    }

    image = (AGFImage *)malloc(sizeof(AGFImage));
    if (image == NULL) {
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->depth = depth;
    image->stride = width;
    image->data = (uint8_t *)malloc(image->stride * image->height);

    if (image->data == NULL) {
        free(image);
        return NULL;
    }

    memset(image->data, 0, agf_image_size(image));

    return image;
}

AGFImage *agf_image_alloc_8(uint16_t width, uint16_t height)
{
    return agf_image_alloc(width, height, AGF_IMAGE_DEPTH_8);
}

