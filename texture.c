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

static uint8_t agf_sqrt_table[65536];
static int agf_sqrt_table_ready = 0;

void agf_texture_init_sqrt_table(void)
{
    uint32_t i;
    uint32_t root = 0;
    uint32_t next_square = 1;

    if (agf_sqrt_table_ready) {
        return;
    }

    for (i = 0; i < 65536; i++) {
        while (root < 255 && i >= next_square) {
            root++;
            next_square = (root + 1) * (root + 1);
        }

        agf_sqrt_table[i] = (uint8_t)root;
    }

    agf_sqrt_table_ready = 1;
}

static uint8_t agf_sqrt_u16(uint32_t value)
{
    if (!agf_sqrt_table_ready) {
        agf_texture_init_sqrt_table();
    }

    if (value > 65535) {
        value = 65535;
    }

    return agf_sqrt_table[value];
}

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
    uint32_t radius_scale;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    width = image->width;
    height = image->height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512;

    agf_texture_init_sqrt_table();

    for (y = 0; y < height; y++) {
        uint8_t *dst = &image->data[y * image->stride];

        for (x = 0; x < width; x++) {
            int32_t dx = x - xcenter;
            int32_t dy = y - ycenter;
            uint32_t dist = (uint32_t)(dx * dx + dy * dy);
            uint32_t col = ((uint32_t)agf_sqrt_u16(dist) * radius_scale) / (uint32_t)width;

            if (col > 255) {
                col = 255;
            }

            *dst++ = (uint8_t)(255 - col);
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
    uint32_t radius_scale;

    if (!agf_texture_is_mono8(image)) {
        return 0;
    }

    width = image->width;
    height = image->height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512;

    agf_texture_init_sqrt_table();

    for (y = 0; y < height; y++) {
        uint8_t *dst = &image->data[y * image->stride];

        for (x = 0; x < width; x++) {
            int32_t dx = x - xcenter;
            int32_t dy = y - ycenter;
            uint32_t dist = (uint32_t)(dx * dx + dy * dy);
            uint32_t col = ((uint32_t)agf_sqrt_u16(dist) * radius_scale) / (uint32_t)width;
            uint32_t src_col;

            if (col >= 256) {
                src_col = 0;
            } else {
                src_col = agf_sqrt_u16(65535 - (col * col));
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

AGFTextureAnimation *agf_texture_animation_alloc(AGFTextureAnimationType type, uint16_t width, uint16_t height, uint16_t nparticles)
{
    AGFTextureAnimation *animation;
    uint16_t n;
    uint16_t shape_width;
    uint16_t shape_height;

    if (width == 0 || height == 0 || nparticles == 0) {
        return NULL;
    }

    animation = (AGFTextureAnimation *)malloc(sizeof(AGFTextureAnimation));
    if (animation == NULL) {
        return NULL;
    }

    animation->type = type;
    animation->width = width;
    animation->height = height;
    animation->nparticles = nparticles;
    animation->xpos = NULL;
    animation->ypos = NULL;
    animation->xspeed = NULL;
    animation->yspeed = NULL;
    animation->shape = NULL;

    animation->xpos = (int32_t *)malloc((uint32_t)nparticles * sizeof(int32_t));
    animation->ypos = (int32_t *)malloc((uint32_t)nparticles * sizeof(int32_t));
    animation->xspeed = (int32_t *)malloc((uint32_t)nparticles * sizeof(int32_t));
    animation->yspeed = (int32_t *)malloc((uint32_t)nparticles * sizeof(int32_t));

    if (animation->xpos == NULL || animation->ypos == NULL || animation->xspeed == NULL || animation->yspeed == NULL) {
        agf_texture_animation_free(animation);
        return NULL;
    }

    if (type == AGF_TEXTURE_ANIMATION_LUMPS) {
        shape_width = (uint16_t)((uint32_t)width / 5);
        shape_height = (uint16_t)((uint32_t)height / 5);
    } else {
        shape_width = (uint16_t)(((uint32_t)width * 3) / 5);
        shape_height = (uint16_t)(((uint32_t)height * 3) / 5);
    }

    if (shape_width == 0) {
        shape_width = 1;
    }
    if (shape_height == 0) {
        shape_height = 1;
    }

    animation->shape = agf_image_alloc_8(shape_width, shape_height);
    if (animation->shape == NULL) {
        agf_texture_animation_free(animation);
        return NULL;
    }

    if (type == AGF_TEXTURE_ANIMATION_LUMPS) {
        agf_texture_generate_sphere(animation->shape);
    } else {
        agf_texture_generate_cone(animation->shape);
    }

    for (n = 0; n < nparticles; n++) {
        int32_t angle = rand() % 360;
        int32_t speed = 128 + (rand() % 128);
        double radians = (double)angle * 3.141593 / 180.0;

        animation->xpos[n] = (int32_t)((uint32_t)(rand() % width) << 8);
        animation->ypos[n] = (int32_t)((uint32_t)(rand() % height) << 8);
        animation->xspeed[n] = (int32_t)(cos(radians) * (double)speed);
        animation->yspeed[n] = (int32_t)(sin(radians) * (double)speed);
    }

    return animation;
}

void agf_texture_animation_free(AGFTextureAnimation *animation)
{
    if (animation != NULL) {
        free(animation->xpos);
        free(animation->ypos);
        free(animation->xspeed);
        free(animation->yspeed);
        agf_image_free(animation->shape);
        free(animation);
    }
}

int agf_texture_animation_render(AGFTextureAnimation *animation, AGFImage *image)
{
    uint16_t n;
    int32_t width_fixed;
    int32_t height_fixed;

    if (animation == NULL || !agf_texture_is_mono8(image) || image->width != animation->width || image->height != animation->height) {
        return 0;
    }

    width_fixed = (int32_t)animation->width << 8;
    height_fixed = (int32_t)animation->height << 8;

    memset(image->data, 0, agf_image_size(image));

    for (n = 0; n < animation->nparticles; n++) {
        float xpos = (float)(animation->xpos[n] >> 8);
        float ypos = (float)(animation->ypos[n] >> 8);

        if (animation->type == AGF_TEXTURE_ANIMATION_LUMPS) {
            agf_texture_blit_tiled_lump(image, animation->shape, xpos, ypos);
        } else {
            agf_texture_blit_tiled_blob(image, animation->shape, xpos, ypos);
        }

        animation->xpos[n] += animation->xspeed[n];
        animation->ypos[n] += animation->yspeed[n];

        if (animation->xpos[n] >= width_fixed) {
            animation->xpos[n] -= width_fixed;
        } else if (animation->xpos[n] < 0) {
            animation->xpos[n] += width_fixed;
        }

        if (animation->ypos[n] >= height_fixed) {
            animation->ypos[n] -= height_fixed;
        } else if (animation->ypos[n] < 0) {
            animation->ypos[n] += height_fixed;
        }
    }

    return 1;
}
