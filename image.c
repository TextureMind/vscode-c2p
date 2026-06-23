#include "image.h"

#include <stdlib_headers.h>

uint32_t agf_image_size(const AGFImage *p_image)
{
    if (p_image == NULL) {
        return 0;
    }

    return (uint32_t)p_image->m_stride * p_image->m_height;
}

AGFImage *agf_image_alloc_with_free(uint16_t p_width, uint16_t p_height, uint16_t p_depth, AGFFreeFunc p_free_func)
{
    AGFImage *image;

    if (p_width == 0 || p_height == 0 || p_depth != AGF_IMAGE_DEPTH_8 || p_free_func == NULL) {
        return NULL;
    }

    image = (AGFImage *)malloc(sizeof(AGFImage));
    if (image == NULL) {
        return NULL;
    }

    image->m_width = p_width;
    image->m_height = p_height;
    image->m_depth = p_depth;
    image->m_stride = p_width;
    image->m_free_func = p_free_func;
    image->m_data = (uint8_t *)malloc(image->m_stride * image->m_height);

    if (image->m_data == NULL) {
        p_free_func(image);
        return NULL;
    }

    memset(image->m_data, 0, agf_image_size(image));

    return image;
}


void agf_image_free(AGFImage *p_image)
{
    if (p_image != NULL && p_image->m_free_func != NULL) {
        p_image->m_free_func(p_image->m_data);
        p_image->m_free_func(p_image);
    }
}
