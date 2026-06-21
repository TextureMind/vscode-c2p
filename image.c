#include "image.h"

#include <stdlib_headers.h>

uint32_t agf_image_size(const AGFImage *image)
{
    if (image == NULL) {
        return 0;
    }

    return (uint32_t)image->stride * image->height;
}

AGFImage *agf_image_alloc_with_free(uint16_t width, uint16_t height, uint16_t depth, AGFFreeFunc free_func)
{
    AGFImage *image;

    if (width == 0 || height == 0 || depth != AGF_IMAGE_DEPTH_8 || free_func == NULL) {
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
    image->free_func = free_func;
    image->data = (uint8_t *)malloc(image->stride * image->height);

    if (image->data == NULL) {
        free_func(image);
        return NULL;
    }

    memset(image->data, 0, agf_image_size(image));

    return image;
}


void agf_image_free(AGFImage *image)
{
    if (image != NULL && image->free_func != NULL) {
        image->free_func(image->data);
        image->free_func(image);
    }
}
