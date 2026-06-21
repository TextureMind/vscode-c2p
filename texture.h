#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

#include "image.h"

int agf_texture_generate_cone(AGFImage *image);
int agf_texture_generate_sphere(AGFImage *image);
void agf_texture_blit_lump(AGFImage *dst, const AGFImage *src, float xpos, float ypos);
void agf_texture_blit_blob(AGFImage *dst, const AGFImage *src, float xpos, float ypos);
int agf_texture_generate_lumps(AGFImage *image, uint16_t nparticles);
int agf_texture_generate_blobs(AGFImage *image, uint16_t nparticles);

#endif
