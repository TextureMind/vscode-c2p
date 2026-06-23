#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

#include "image.h"

typedef enum AGFTextureAnimationType
{
    AGF_TEXTURE_ANIMATION_LUMPS = 0,
    AGF_TEXTURE_ANIMATION_BLOBS
} AGFTextureAnimationType;

typedef struct AGFTextureAnimation
{
    AGFTextureAnimationType m_type;
    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_nparticles;
    int32_t *m_xpos;
    int32_t *m_ypos;
    int32_t *m_xspeed;
    int32_t *m_yspeed;
    AGFImage *m_shape;
    AGFFreeFunc m_free_func;
} AGFTextureAnimation;

void agf_texture_init_sqrt_table(void);
int agf_texture_generate_cone(AGFImage *p_image);
int agf_texture_generate_sphere(AGFImage *p_image);
void agf_texture_blit_lump(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos);
void agf_texture_blit_blob(AGFImage *p_dst, const AGFImage *p_src, float p_xpos, float p_ypos);
int agf_texture_generate_lumps(AGFImage *p_image, uint16_t p_nparticles);
int agf_texture_generate_blobs(AGFImage *p_image, uint16_t p_nparticles);
#define agf_texture_animation_alloc(type, width, height, nparticles) \
    agf_texture_animation_alloc_with_free((type), (width), (height), (nparticles), free)

void agf_texture_animation_free(AGFTextureAnimation *p_animation);

AGFTextureAnimation *agf_texture_animation_alloc_with_free(AGFTextureAnimationType p_type, uint16_t p_width, uint16_t p_height, uint16_t p_nparticles, AGFFreeFunc p_free_func);
int agf_texture_animation_render(AGFTextureAnimation *p_animation, AGFImage *p_image);

#endif
