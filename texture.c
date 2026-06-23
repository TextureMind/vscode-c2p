#include "texture.h"

#include <math_headers.h>
#include <stdlib_headers.h>

typedef struct AGFRect
{
    int32_t m_left;
    int32_t m_top;
    int32_t m_right;
    int32_t m_bottom;
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

static int agf_texture_is_mono8(const AGFImage *p_image)
{
    return p_image != NULL && p_image->m_data != NULL && p_image->m_depth == AGF_IMAGE_DEPTH_8;
}

int agf_texture_generate_cone(AGFImage *p_image)
{
    int32_t x;
    int32_t y;
    int32_t xcenter;
    int32_t ycenter;
    int32_t width;
    int32_t height;
    uint32_t radius_scale;

    if (!agf_texture_is_mono8(p_image)) {
        return 0;
    }

    width = p_image->m_width;
    height = p_image->m_height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512;

    agf_texture_init_sqrt_table();

    for (y = 0; y < height; y++) {
        uint8_t *dst = &p_image->m_data[y * p_image->m_stride];

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

int agf_texture_generate_sphere(AGFImage *p_image)
{
    int32_t x;
    int32_t y;
    int32_t xcenter;
    int32_t ycenter;
    int32_t width;
    int32_t height;
    uint32_t radius_scale;

    if (!agf_texture_is_mono8(p_image)) {
        return 0;
    }

    width = p_image->m_width;
    height = p_image->m_height;
    xcenter = width / 2;
    ycenter = height / 2;
    radius_scale = 512;

    agf_texture_init_sqrt_table();

    for (y = 0; y < height; y++) {
        uint8_t *dst = &p_image->m_data[y * p_image->m_stride];

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

void agf_texture_blit_lump(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos)
{
    AGFRect dst_rect;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    const uint8_t *src_scan;
    uint8_t *dst_scan;

    if (!agf_texture_is_mono8(p_dst) || !agf_texture_is_mono8(p_src)) {
        return;
    }

    dst_rect.m_left = (int32_t)p_xpos;
    dst_rect.m_top = (int32_t)p_ypos;
    dst_rect.m_right = dst_rect.m_left + p_src->m_width;
    dst_rect.m_bottom = dst_rect.m_top + p_src->m_height;

    src_scan = p_src->m_data;

    if (dst_rect.m_left < 0) {
        src_scan -= dst_rect.m_left;
        dst_rect.m_left = 0;
    }

    if (dst_rect.m_top < 0) {
        src_scan -= dst_rect.m_top * p_src->m_stride;
        dst_rect.m_top = 0;
    }

    if (dst_rect.m_right > p_dst->m_width) {
        dst_rect.m_right = p_dst->m_width;
    }

    if (dst_rect.m_bottom > p_dst->m_height) {
        dst_rect.m_bottom = p_dst->m_height;
    }

    width = dst_rect.m_right - dst_rect.m_left;
    height = dst_rect.m_bottom - dst_rect.m_top;
    if (width <= 0 || height <= 0) {
        return;
    }

    dst_scan = &p_dst->m_data[dst_rect.m_left + dst_rect.m_top * p_dst->m_stride];

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

        src_scan += p_src->m_stride;
        dst_scan += p_dst->m_stride;
    }
}

void agf_texture_blit_blob(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos)
{
    AGFRect dst_rect;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    const uint8_t *src_scan;
    uint8_t *dst_scan;

    if (!agf_texture_is_mono8(p_dst) || !agf_texture_is_mono8(p_src)) {
        return;
    }

    dst_rect.m_left = (int32_t)p_xpos;
    dst_rect.m_top = (int32_t)p_ypos;
    dst_rect.m_right = dst_rect.m_left + p_src->m_width;
    dst_rect.m_bottom = dst_rect.m_top + p_src->m_height;

    src_scan = p_src->m_data;

    if (dst_rect.m_left < 0) {
        src_scan -= dst_rect.m_left;
        dst_rect.m_left = 0;
    }

    if (dst_rect.m_top < 0) {
        src_scan -= dst_rect.m_top * p_src->m_stride;
        dst_rect.m_top = 0;
    }

    if (dst_rect.m_right > p_dst->m_width) {
        dst_rect.m_right = p_dst->m_width;
    }

    if (dst_rect.m_bottom > p_dst->m_height) {
        dst_rect.m_bottom = p_dst->m_height;
    }

    width = dst_rect.m_right - dst_rect.m_left;
    height = dst_rect.m_bottom - dst_rect.m_top;
    if (width <= 0 || height <= 0) {
        return;
    }

    dst_scan = &p_dst->m_data[dst_rect.m_left + dst_rect.m_top * p_dst->m_stride];

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

        src_scan += p_src->m_stride;
        dst_scan += p_dst->m_stride;
    }
}

static void agf_texture_blit_tiled_lump(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos)
{
    float width = (float)p_dst->m_width;
    float height = (float)p_dst->m_height;

    agf_texture_blit_lump(p_dst, p_src, p_xpos, p_ypos);
    agf_texture_blit_lump(p_dst, p_src, p_xpos - width, p_ypos + height);
    agf_texture_blit_lump(p_dst, p_src, p_xpos - width, p_ypos);
    agf_texture_blit_lump(p_dst, p_src, p_xpos - width, p_ypos - height);
    agf_texture_blit_lump(p_dst, p_src, p_xpos, p_ypos - height);
    agf_texture_blit_lump(p_dst, p_src, p_xpos + width, p_ypos - height);
    agf_texture_blit_lump(p_dst, p_src, p_xpos + width, p_ypos);
    agf_texture_blit_lump(p_dst, p_src, p_xpos, p_ypos + height);
    agf_texture_blit_lump(p_dst, p_src, p_xpos + width, p_ypos + height);
}

static void agf_texture_blit_tiled_blob(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos)
{
    float width = (float)p_dst->m_width;
    float height = (float)p_dst->m_height;

    agf_texture_blit_blob(p_dst, p_src, p_xpos, p_ypos);
    agf_texture_blit_blob(p_dst, p_src, p_xpos - width, p_ypos + height);
    agf_texture_blit_blob(p_dst, p_src, p_xpos - width, p_ypos);
    agf_texture_blit_blob(p_dst, p_src, p_xpos - width, p_ypos - height);
    agf_texture_blit_blob(p_dst, p_src, p_xpos, p_ypos - height);
    agf_texture_blit_blob(p_dst, p_src, p_xpos + width, p_ypos - height);
    agf_texture_blit_blob(p_dst, p_src, p_xpos + width, p_ypos);
    agf_texture_blit_blob(p_dst, p_src, p_xpos, p_ypos + height);
    agf_texture_blit_blob(p_dst, p_src, p_xpos + width, p_ypos + height);
}

int agf_texture_generate_lumps(AGFImage *p_image, uint16_t p_nparticles)
{
    uint16_t n;
    uint16_t sphere_width;
    uint16_t sphere_height;
    AGFImage *sphere;

    if (!agf_texture_is_mono8(p_image)) {
        return 0;
    }

    sphere_width = (uint16_t)((float)p_image->m_width * 0.2f);
    sphere_height = (uint16_t)((float)p_image->m_height * 0.2f);
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

    memset(p_image->m_data, 0, agf_image_size(p_image));

    for (n = 0; n < p_nparticles; n++) {
        float xpos = (float)((uint32_t)rand() % p_image->m_width);
        float ypos = (float)((uint32_t)rand() % p_image->m_height);
        agf_texture_blit_tiled_lump(p_image, sphere, xpos, ypos);
    }

    agf_image_free(sphere);

    return 1;
}

int agf_texture_generate_blobs(AGFImage *p_image, uint16_t p_nparticles)
{
    uint16_t n;
    uint16_t cone_width;
    uint16_t cone_height;
    AGFImage *cone;

    if (!agf_texture_is_mono8(p_image)) {
        return 0;
    }

    cone_width = (uint16_t)((float)p_image->m_width * 0.6f);
    cone_height = (uint16_t)((float)p_image->m_height * 0.6f);
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

    memset(p_image->m_data, 0, agf_image_size(p_image));

    for (n = 0; n < p_nparticles; n++) {
        float xpos = (float)((uint32_t)rand() % p_image->m_width);
        float ypos = (float)((uint32_t)rand() % p_image->m_height);
        agf_texture_blit_tiled_blob(p_image, cone, xpos, ypos);
    }

    agf_image_free(cone);

    return 1;
}

AGFTextureAnimation *agf_texture_animation_alloc_with_free(AGFTextureAnimationType p_type, uint16_t p_width, uint16_t p_height, uint16_t p_nparticles, AGFFreeFunc p_free_func)
{
    AGFTextureAnimation *animation;
    uint16_t n;
    uint16_t shape_width;
    uint16_t shape_height;

    if (p_width == 0 || p_height == 0 || p_nparticles == 0 || p_free_func == NULL) {
        return NULL;
    }

    animation = (AGFTextureAnimation *)malloc(sizeof(AGFTextureAnimation));
    if (animation == NULL) {
        return NULL;
    }

    animation->m_type = p_type;
    animation->m_width = p_width;
    animation->m_height = p_height;
    animation->m_nparticles = p_nparticles;
    animation->m_xpos = NULL;
    animation->m_ypos = NULL;
    animation->m_xspeed = NULL;
    animation->m_yspeed = NULL;
    animation->m_shape = NULL;
    animation->m_free_func = p_free_func;

    animation->m_xpos = (int32_t *)malloc((uint32_t)p_nparticles * sizeof(int32_t));
    animation->m_ypos = (int32_t *)malloc((uint32_t)p_nparticles * sizeof(int32_t));
    animation->m_xspeed = (int32_t *)malloc((uint32_t)p_nparticles * sizeof(int32_t));
    animation->m_yspeed = (int32_t *)malloc((uint32_t)p_nparticles * sizeof(int32_t));

    if (animation->m_xpos == NULL || animation->m_ypos == NULL || animation->m_xspeed == NULL || animation->m_yspeed == NULL) {
        agf_texture_animation_free(animation);
        return NULL;
    }

    if (p_type == AGF_TEXTURE_ANIMATION_LUMPS) {
        shape_width = (uint16_t)((uint32_t)p_width / 5);
        shape_height = (uint16_t)((uint32_t)p_height / 5);
    } else {
        shape_width = (uint16_t)(((uint32_t)p_width * 3) / 5);
        shape_height = (uint16_t)(((uint32_t)p_height * 3) / 5);
    }

    if (shape_width == 0) {
        shape_width = 1;
    }
    if (shape_height == 0) {
        shape_height = 1;
    }

    animation->m_shape = agf_image_alloc_8(shape_width, shape_height);
    if (animation->m_shape == NULL) {
        agf_texture_animation_free(animation);
        return NULL;
    }

    if (p_type == AGF_TEXTURE_ANIMATION_LUMPS) {
        agf_texture_generate_sphere(animation->m_shape);
    } else {
        agf_texture_generate_cone(animation->m_shape);
    }

    for (n = 0; n < p_nparticles; n++) {
        int32_t angle = rand() % 360;
        int32_t speed = 128 + (rand() % 128);
        double radians = (double)angle * 3.141593 / 180.0;

        animation->m_xpos[n] = (int32_t)((uint32_t)(rand() % p_width) << 8);
        animation->m_ypos[n] = (int32_t)((uint32_t)(rand() % p_height) << 8);
        animation->m_xspeed[n] = (int32_t)(cos(radians) * (double)speed);
        animation->m_yspeed[n] = (int32_t)(sin(radians) * (double)speed);
    }

    return animation;
}

void agf_texture_animation_free(AGFTextureAnimation *p_animation)
{
    if (p_animation != NULL && p_animation->m_free_func != NULL) {
        p_animation->m_free_func(p_animation->m_xpos);
        p_animation->m_free_func(p_animation->m_ypos);
        p_animation->m_free_func(p_animation->m_xspeed);
        p_animation->m_free_func(p_animation->m_yspeed);
        agf_image_free(p_animation->m_shape);
        p_animation->m_free_func(p_animation);
    }
}

int agf_texture_animation_render(AGFTextureAnimation *p_animation, AGFImage *p_image)
{
    uint16_t n;
    int32_t width_fixed;
    int32_t height_fixed;

    if (p_animation == NULL ||
        !agf_texture_is_mono8(p_image) ||
        p_image->m_width != p_animation->m_width ||
        p_image->m_height != p_animation->m_height) {
        return 0;
    }

    width_fixed = (int32_t)p_animation->m_width << 8;
    height_fixed = (int32_t)p_animation->m_height << 8;

    memset(p_image->m_data, 0, agf_image_size(p_image));

    for (n = 0; n < p_animation->m_nparticles; n++) {
        float xpos = (float)(p_animation->m_xpos[n] >> 8);
        float ypos = (float)(p_animation->m_ypos[n] >> 8);

        if (p_animation->m_type == AGF_TEXTURE_ANIMATION_LUMPS) {
            agf_texture_blit_tiled_lump(p_image, p_animation->m_shape, xpos, ypos);
        } else {
            agf_texture_blit_tiled_blob(p_image, p_animation->m_shape, xpos, ypos);
        }

        p_animation->m_xpos[n] += p_animation->m_xspeed[n];
        p_animation->m_ypos[n] += p_animation->m_yspeed[n];

        if (p_animation->m_xpos[n] >= width_fixed) {
            p_animation->m_xpos[n] -= width_fixed;
        } else if (p_animation->m_xpos[n] < 0) {
            p_animation->m_xpos[n] += width_fixed;
        }

        if (p_animation->m_ypos[n] >= height_fixed) {
            p_animation->m_ypos[n] -= height_fixed;
        } else if (p_animation->m_ypos[n] < 0) {
            p_animation->m_ypos[n] += height_fixed;
        }
    }

    return 1;
}
