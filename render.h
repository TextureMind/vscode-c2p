#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "image.h"

#define AGF_DEPTH_MAX 0xffffffffUL
#define AGF_NORMAL_SCALE 16384

typedef struct AGFDepthBuffer
{
    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_stride;
    uint32_t *m_data;
    AGFFreeFunc m_free_func;
} AGFDepthBuffer;

typedef struct AGFVertex3f
{
    float m_x;
    float m_y;
    float m_z;
} AGFVertex3f;

typedef AGFVertex3f AGFVector3f;

typedef struct AGFMatrix4x4
{
    float m_values[4][4];
} AGFMatrix4x4;

typedef struct AGFVertex3i
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
} AGFVertex3i;

typedef struct AGFVertex3n
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
    int16_t m_nx;
    int16_t m_ny;
    int16_t m_nz;
} AGFVertex3n;

typedef struct AGFVertex3c
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
    uint8_t m_color;
} AGFVertex3c;

typedef struct AGFVertex3t
{
    int16_t m_x;
    int16_t m_y;
    uint32_t m_z;
    int32_t m_u;
    int32_t m_v;
} AGFVertex3t;

typedef struct AGFDirectionalLight
{
    int16_t m_nx;
    int16_t m_ny;
    int16_t m_nz;
    uint8_t m_ambient;
    uint8_t m_intensity;
} AGFDirectionalLight;

typedef struct AGFPolyTriangle3D
{
    uint32_t m_indices[3];
    AGFVector3f m_normal;
} AGFPolyTriangle3D;

typedef struct AGFPolyQuad3D
{
    uint32_t m_indices[4];
    AGFVector3f m_normal;
} AGFPolyQuad3D;

typedef struct AGFPolygonHull3D
{
    AGFVector3f *m_points;
    uint32_t m_npoints;
    AGFPolyTriangle3D *m_triangles;
    uint32_t m_ntriangles;
    AGFPolyQuad3D *m_quads;
    uint32_t m_nquads;
    AGFFreeFunc m_free_func;
} AGFPolygonHull3D;

typedef struct AGFVertexAttribute3f
{
    uint32_t m_index;
    AGFVector3f m_normal;
} AGFVertexAttribute3f;

typedef struct AGFMeshTriangle3D
{
    uint32_t m_indices[3];
    uint32_t m_polygonIndex;
} AGFMeshTriangle3D;

typedef struct AGFMeshQuad3D
{
    uint32_t m_indices[4];
    uint32_t m_polygonIndex;
} AGFMeshQuad3D;

typedef struct AGFMeshSlice3D
{
    AGFVertexAttribute3f *m_vertexAttributes;
    uint32_t m_nvertexAttributes;
    AGFMeshTriangle3D *m_triangles;
    uint32_t m_ntriangles;
    AGFMeshQuad3D *m_quads;
    uint32_t m_nquads;
    AGFFreeFunc m_free_func;
} AGFMeshSlice3D;

typedef struct AGFMesh3D
{
    AGFPolygonHull3D m_polygonHull;
    AGFMeshSlice3D *m_slices;
    uint32_t m_nslices;
    AGFFreeFunc m_free_func;
} AGFMesh3D;

#define agf_depth_buffer_alloc(width, height) \
    agf_depth_buffer_alloc_with_free((width), (height), free)

void agf_matrix_identity(AGFMatrix4x4 *p_matrix);
void agf_matrix_translation(AGFMatrix4x4 *p_matrix, float p_x, float p_y, float p_z);
void agf_matrix_scale(AGFMatrix4x4 *p_matrix, float p_x, float p_y, float p_z);
void agf_matrix_rotation_x(AGFMatrix4x4 *p_matrix, float p_angle);
void agf_matrix_rotation_y(AGFMatrix4x4 *p_matrix, float p_angle);
void agf_matrix_rotation_z(AGFMatrix4x4 *p_matrix, float p_angle);
void agf_matrix_perspective(AGFMatrix4x4 *p_matrix, float p_focal, float p_aspect, float p_near, float p_far);
void agf_matrix_multiply(AGFMatrix4x4 *p_out, const AGFMatrix4x4 *p_a, const AGFMatrix4x4 *p_b);
AGFVector3f agf_matrix_transform_point(const AGFMatrix4x4 *p_matrix, AGFVector3f p_point);
AGFVector3f agf_matrix_transform_vector(const AGFMatrix4x4 *p_matrix, AGFVector3f p_vector);

AGFDepthBuffer *agf_depth_buffer_alloc_with_free(uint16_t p_width, uint16_t p_height, AGFFreeFunc p_free_func);
void agf_depth_buffer_free(AGFDepthBuffer *p_buffer);
void agf_depth_buffer_clear(AGFDepthBuffer *p_buffer, uint32_t p_depth);
int agf_draw_triangle_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_v0, const AGFVertex3i *p_v1, const AGFVertex3i *p_v2, uint8_t p_color);
int agf_draw_polygon_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3i *p_vertices, uint16_t p_count, uint8_t p_color);
int agf_draw_triangle_gouraud(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3c *p_v0, const AGFVertex3c *p_v1, const AGFVertex3c *p_v2);
int agf_draw_polygon_gouraud(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3c *p_vertices, uint16_t p_count);
int agf_draw_triangle_textured(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFImage *p_texture, const AGFVertex3t *p_v0, const AGFVertex3t *p_v1, const AGFVertex3t *p_v2);
int agf_draw_polygon_textured(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFImage *p_texture, const AGFVertex3t *p_vertices, uint16_t p_count);
int agf_draw_triangle_textured_fixed(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFImage *p_texture, const AGFVertex3t *p_v0, const AGFVertex3t *p_v1, const AGFVertex3t *p_v2);
int agf_draw_polygon_textured_fixed(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFImage *p_texture, const AGFVertex3t *p_vertices, uint16_t p_count);
uint8_t agf_light_vertex_color(const AGFDirectionalLight *p_light, int16_t p_nx, int16_t p_ny, int16_t p_nz);
uint8_t agf_light_flat_color(const AGFDirectionalLight *p_light, const AGFVertex3n *p_vertices, uint16_t p_count);
int agf_draw_polygon_lit_flat(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3n *p_vertices, uint16_t p_count, const AGFDirectionalLight *p_light);
int agf_draw_polygon_lit_gouraud(AGFImage *p_image, AGFDepthBuffer *p_depth, const AGFVertex3n *p_vertices, uint16_t p_count, const AGFDirectionalLight *p_light);

#endif
