#include "texture.h"

#include <math_headers.h>
#include <stdlib_headers.h>

typedef struct AGFRect
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} AGFRect;

static int agf_texture_is_mono8(const AGFImage *image)
{
    return image != NULL && image->data != NULL && image->depth == AGF_IMAGE_DEPTH_8;
}

int agf_texture_generate_cone(AGFImage *image)
{
    int32_t x;
    int32_t y;
    int32_t xcenter;
    int32_t ycenter;
    int32_t width;
    int32_t height;
    float radius_scale;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    width = image->width;
    height = image->height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512.0f / (float)width;

    for (y = 0; y < height; y++) {
        uint8_t *dst = &image->data[y * image->stride];

        for (x = 0; x < width; x++) {
            int32_t dx = x - xcenter;
            int32_t dy = y - ycenter;
            float col = sqrt((float)(dx * dx) + (float)(dy * dy)) * radius_scale;
            uint32_t src_col;

            if (col > 255.0f) {
                col = 255.0f;
            }

            src_col = 255 - (uint32_t)col;
            *dst++ = (uint8_t)src_col;
        }
    }

    return 1;
}

int agf_texture_generate_sphere(AGFImage *image)
{
    int32_t x;
    int32_t y;
    int32_t xcenter;
    int32_t ycenter;
    int32_t width;
    int32_t height;
    float radius_scale;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    width = image->width;
    height = image->height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512.0f / (float)width;

    for (y = 0; y < height; y++) {
        uint8_t *dst = &image->data[y * image->stride];

        for (x = 0; x < width; x++) {
            int32_t dx = x - xcenter;
            int32_t dy = y - ycenter;
            float col = sqrt((float)(dx * dx) + (float)(dy * dy)) * radius_scale;
            uint32_t src_col;

            if (col > 256.0f) {
                col = 256.0f;
            }

            src_col = (uint32_t)sqrt(fabs(65536.0f - (col * col)));
            if (src_col > 255) {
                src_col = 255;
            }

            *dst++ = (uint8_t)src_col;
        }
    }

    return 1;
}

void agf_texture_blit_lump(AGFImage *dst, const AGFImage *src, float xpos, float ypos)
{
    AGFRect dst_rect;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    const uint8_t *src_scan;
    uint8_t *dst_scan;

    if (!agf_texture_is_mono8(dst) || !agf_texture_is_mono8(src)) {
        return;
    }

    dst_rect.left = (int32_t)xpos;
    dst_rect.top = (int32_t)ypos;
    dst_rect.right = dst_rect.left + src->width;
    dst_rect.bottom = dst_rect.top + src->height;

    src_scan = src->data;

    if (dst_rect.left < 0) {
        src_scan -= dst_rect.left;
        dst_rect.left = 0;
    }

    if (dst_rect.top < 0) {
        src_scan -= dst_rect.top * src->stride;
        dst_rect.top = 0;
    }

    if (dst_rect.right > dst->width) {
        dst_rect.right = dst->width;
    }

    if (dst_rect.bottom > dst->height) {
        dst_rect.bottom = dst->height;
    }

    width = dst_rect.right - dst_rect.left;
    height = dst_rect.bottom - dst_rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    dst_scan = &dst->data[dst_rect.left + dst_rect.top * dst->stride];

    for (y = 0; y < height; y++) {
        const uint8_t *src_data = src_scan;
        uint8_t *dst_data = dst_scan;

        for (x = 0; x < width; x++) {
            uint8_t scol = *src_data;
            uint8_t dcol = *dst_data;

            if (scol > dcol) {
                *dst_data = scol;
            }

            dst_data++;
            src_data++;
        }

        src_scan += src->stride;
        dst_scan += dst->stride;
    }
}

void agf_texture_blit_blob(AGFImage *dst, const AGFImage *src, float xpos, float ypos)
{
    AGFRect dst_rect;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    const uint8_t *src_scan;
    uint8_t *dst_scan;

    if (!agf_texture_is_mono8(dst) || !agf_texture_is_mono8(src)) {
        return;
    }

    dst_rect.left = (int32_t)xpos;
    dst_rect.top = (int32_t)ypos;
    dst_rect.right = dst_rect.left + src->width;
    dst_rect.bottom = dst_rect.top + src->height;

    src_scan = src->data;

    if (dst_rect.left < 0) {
        src_scan -= dst_rect.left;
        dst_rect.left = 0;
    }

    if (dst_rect.top < 0) {
        src_scan -= dst_rect.top * src->stride;
        dst_rect.top = 0;
    }

    if (dst_rect.right > dst->width) {
        dst_rect.right = dst->width;
    }

    if (dst_rect.bottom > dst->height) {
        dst_rect.bottom = dst->height;
    }

    width = dst_rect.right - dst_rect.left;
    height = dst_rect.bottom - dst_rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    dst_scan = &dst->data[dst_rect.left + dst_rect.top * dst->stride];

    for (y = 0; y < height; y++) {
        const uint8_t *src_data = src_scan;
        uint8_t *dst_data = dst_scan;

        for (x = 0; x < width; x++) {
            int32_t dcol = *dst_data + *src_data;

            if (dcol > 255) {
                dcol = 510 - dcol;
            }

            *dst_data = (uint8_t)dcol;

            dst_data++;
            src_data++;
        }

        src_scan += src->stride;
        dst_scan += dst->stride;
    }
}

static void agf_texture_blit_tiled_lump(AGFImage *dst, const AGFImage *src, float xpos, float ypos)
{
    float width = (float)dst->width;
    float height = (float)dst->height;

    agf_texture_blit_lump(dst, src, xpos, ypos);
    agf_texture_blit_lump(dst, src, xpos - width, ypos + height);
    agf_texture_blit_lump(dst, src, xpos - width, ypos);
    agf_texture_blit_lump(dst, src, xpos - width, ypos - height);
    agf_texture_blit_lump(dst, src, xpos, ypos - height);
    agf_texture_blit_lump(dst, src, xpos + width, ypos - height);
    agf_texture_blit_lump(dst, src, xpos + width, ypos);
    agf_texture_blit_lump(dst, src, xpos, ypos + height);
    agf_texture_blit_lump(dst, src, xpos + width, ypos + height);
}

static void agf_texture_blit_tiled_blob(AGFImage *dst, const AGFImage *src, float xpos, float ypos)
{
    float width = (float)dst->width;
    float height = (float)dst->height;

    agf_texture_blit_blob(dst, src, xpos, ypos);
    agf_texture_blit_blob(dst, src, xpos - width, ypos + height);
    agf_texture_blit_blob(dst, src, xpos - width, ypos);
    agf_texture_blit_blob(dst, src, xpos - width, ypos - height);
    agf_texture_blit_blob(dst, src, xpos, ypos - height);
    agf_texture_blit_blob(dst, src, xpos + width, ypos - height);
    agf_texture_blit_blob(dst, src, xpos + width, ypos);
    agf_texture_blit_blob(dst, src, xpos, ypos + height);
    agf_texture_blit_blob(dst, src, xpos + width, ypos + height);
}

int agf_texture_generate_lumps(AGFImage *image, uint16_t nparticles)
{
    uint16_t n;
    uint16_t sphere_width;
    uint16_t sphere_height;
    AGFImage *sphere;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    sphere_width = (uint16_t)((float)image->width * 0.2f);
    sphere_height = (uint16_t)((float)image->height * 0.2f);
    if (sphere_width == 0) {
        sphere_width = 1;
    }
    if (sphere_height == 0) {
        sphere_height = 1;
    }

    sphere = agf_image_alloc_8(sphere_width, sphere_height);
    if (sphere == NULL) {
        return 0;
    }

    if (!agf_texture_generate_sphere(sphere)) {
        agf_image_free(sphere);
        return 0;
    }

    memset(image->data, 0, agf_image_size(image));

    for (n = 0; n < nparticles; n++) {
        float xpos = (float)((uint32_t)rand() % image->width);
        float ypos = (float)((uint32_t)rand() % image->height);
        agf_texture_blit_tiled_lump(image, sphere, xpos, ypos);
    }

    agf_image_free(sphere);
    return 1;
}

int agf_texture_generate_blobs(AGFImage *image, uint16_t nparticles)
{
    uint16_t n;
    uint16_t cone_width;
    uint16_t cone_height;
    AGFImage *cone;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    cone_width = (uint16_t)((float)image->width * 0.6f);
    cone_height = (uint16_t)((float)image->height * 0.6f);
    if (cone_width == 0) {
        cone_width = 1;
    }
    if (cone_height == 0) {
        cone_height = 1;
    }

    cone = agf_image_alloc_8(cone_width, cone_height);
    if (cone == NULL) {
        return 0;
    }

    if (!agf_texture_generate_cone(cone)) {
        agf_image_free(cone);
        return 0;
    }

    memset(image->data, 0, agf_image_size(image));

    for (n = 0; n < nparticles; n++) {
        float xpos = (float)((uint32_t)rand() % image->width);
        float ypos = (float)((uint32_t)rand() % image->height);
        agf_texture_blit_tiled_blob(image, cone, xpos, ypos);
    }

    agf_image_free(cone);
    return 1;
}
