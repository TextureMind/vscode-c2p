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
    AGFTextureAnimationType type;
    uint16_t width;
    uint16_t height;
    uint16_t nparticles;
    int32_t *xpos;
    int32_t *ypos;
    int32_t *xspeed;
    int32_t *yspeed;
    AGFImage *shape;
} AGFTextureAnimation;

void agf_texture_init_sqrt_table(void);
int agf_texture_generate_cone(AGFImage *image);
int agf_texture_generate_sphere(AGFImage *image);
void agf_texture_blit_lump(AGFImage *dst, const AGFImage *src, float xpos, float ypos);
void agf_texture_blit_blob(AGFImage *dst, const AGFImage *src, float xpos, float ypos);
int agf_texture_generate_lumps(AGFImage *image, uint16_t nparticles);
int agf_texture_generate_blobs(AGFImage *image, uint16_t nparticles);
AGFTextureAnimation *agf_texture_animation_alloc(AGFTextureAnimationType type, uint16_t width, uint16_t height, uint16_t nparticles);
void agf_texture_animation_free(AGFTextureAnimation *animation);
int agf_texture_animation_render(AGFTextureAnimation *animation, AGFImage *image);

#endif
