
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <clib/alib_protos.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include "g_misc.h"
#include "image.h"
#include "texture.h"
#include "render.h"

#define OPTION_USE_CLIB2 1
#define OPTION_USE_BSD_SOCKET 0
#define OPTION_USE_USER_GROUP 0

#if OPTION_USE_CLIB2
#include <stdlib_headers.h>
#include <math_headers.h>
#endif

struct c2pStruct
{
  struct BitMap *bmap;
  unsigned short startX, startY, width, height;
  unsigned char *ChunkyBuffer;
};

extern void ChunkyToPlanarAsm(void);

static inline void chunkyToPlanar(struct c2pStruct *arg)
{
    register struct c2pStruct *a0 __asm__("a0") = arg;
    __asm__ volatile (
        "jsr _ChunkyToPlanarAsm"
        :
        : "r"(a0)
        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
    );
}

static inline void chunkyToPlanar2Init(int chunkyx, int chunkyy, int scroffsy)
{
    register volatile int d0 __asm__("d0") = chunkyx;
    register volatile int d1 __asm__("d1") = chunkyy;
    register volatile int d3 __asm__("d3") = scroffsy;
    __asm__ volatile (
        "jsr _c2p1x1_8_c5_gen_init"
        :
        : "r"(d0), "r"(d1), "r"(d3) 
    );
}

static void chunkyToPlanar2(void *input, void *output)
{
    register void *a0 __asm__("a0") = input;
    register void *a1 __asm__("a1") = output;
    __asm__ volatile (
        "jsr _c2p1x1_8_c5_gen"
        :
        : "r"(a0), "r"(a1)
    );
}

__attribute__((always_inline)) inline short mouseLeft()
{
    return !((*(volatile UBYTE*)0xbfe001)&64);
}

__attribute__((always_inline)) inline short mouseRight()
{
    return !((*(volatile UWORD*)0xdff016)&(1<<10));
}

static void waitVbl()
{
	while (1) {
		volatile ULONG vpos =* (volatile ULONG*)0xDFF004;
		vpos &= 0x1ff00;

        if (vpos != (311<<8)) {
			break;
        }
	}

	while (1) {
		volatile ULONG vpos =* (volatile ULONG*)0xDFF004;
		vpos &= 0x1ff00;

        if (vpos == (311<<8)) {
			break;
        }
	}
}

static void agf_project_vertex(const AGFVector3f *p_point, AGFVertex3i *p_vertex, uint16_t p_width, uint16_t p_height, float p_camera_z, float p_focal)
{
    float z3;

    z3 = p_point->m_z + p_camera_z;
    p_vertex->m_x = (int16_t)((float)(p_width / 2) + (p_point->m_x * p_focal) / z3);
    p_vertex->m_y = (int16_t)((float)(p_height / 2) + (p_point->m_y * p_focal) / z3);
    p_vertex->m_z = (uint32_t)(z3 * 256.0f);
}

static void agf_build_cube_mesh(AGFMesh3D *p_mesh)
{
    static AGFVector3f points[8] = {
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f}
    };
    static AGFPolyQuad3D hull_quads[6] = {
        {{0, 1, 2, 3}, { 0.0f,  0.0f, -1.0f}},
        {{4, 7, 6, 5}, { 0.0f,  0.0f,  1.0f}},
        {{0, 4, 5, 1}, { 0.0f, -1.0f,  0.0f}},
        {{3, 2, 6, 7}, { 0.0f,  1.0f,  0.0f}},
        {{1, 5, 6, 2}, { 1.0f,  0.0f,  0.0f}},
        {{0, 3, 7, 4}, {-1.0f,  0.0f,  0.0f}}
    };
    static AGFVertexAttribute3f vertex_attributes[24] = {
        {0, { 0.0f,  0.0f, -1.0f}}, {1, { 0.0f,  0.0f, -1.0f}}, {2, { 0.0f,  0.0f, -1.0f}}, {3, { 0.0f,  0.0f, -1.0f}},
        {4, { 0.0f,  0.0f,  1.0f}}, {7, { 0.0f,  0.0f,  1.0f}}, {6, { 0.0f,  0.0f,  1.0f}}, {5, { 0.0f,  0.0f,  1.0f}},
        {0, { 0.0f, -1.0f,  0.0f}}, {4, { 0.0f, -1.0f,  0.0f}}, {5, { 0.0f, -1.0f,  0.0f}}, {1, { 0.0f, -1.0f,  0.0f}},
        {3, { 0.0f,  1.0f,  0.0f}}, {2, { 0.0f,  1.0f,  0.0f}}, {6, { 0.0f,  1.0f,  0.0f}}, {7, { 0.0f,  1.0f,  0.0f}},
        {1, { 1.0f,  0.0f,  0.0f}}, {5, { 1.0f,  0.0f,  0.0f}}, {6, { 1.0f,  0.0f,  0.0f}}, {2, { 1.0f,  0.0f,  0.0f}},
        {0, {-1.0f,  0.0f,  0.0f}}, {3, {-1.0f,  0.0f,  0.0f}}, {7, {-1.0f,  0.0f,  0.0f}}, {4, {-1.0f,  0.0f,  0.0f}}
    };
    static AGFMeshQuad3D mesh_quads[6] = {
        {{ 0,  1,  2,  3}, 0},
        {{ 4,  5,  6,  7}, 1},
        {{ 8,  9, 10, 11}, 2},
        {{12, 13, 14, 15}, 3},
        {{16, 17, 18, 19}, 4},
        {{20, 21, 22, 23}, 5}
    };
    static AGFMeshSlice3D slices[1] = {
        {vertex_attributes, 24, NULL, 0, mesh_quads, 6, NULL}
    };

    p_mesh->m_polygonHull.m_points = points;
    p_mesh->m_polygonHull.m_npoints = 8;
    p_mesh->m_polygonHull.m_triangles = NULL;
    p_mesh->m_polygonHull.m_ntriangles = 0;
    p_mesh->m_polygonHull.m_quads = hull_quads;
    p_mesh->m_polygonHull.m_nquads = 6;
    p_mesh->m_polygonHull.m_free_func = NULL;
    p_mesh->m_slices = slices;
    p_mesh->m_nslices = 1;
    p_mesh->m_free_func = NULL;
}

static void drawRotatingCube(AGFImage *p_image, AGFDepthBuffer *p_depth, float p_angle)
{
    AGFMesh3D mesh;
    AGFMatrix4x4 scale_matrix;
    AGFMatrix4x4 rotation_x;
    AGFMatrix4x4 rotation_y;
    AGFMatrix4x4 rotation_z;
    AGFMatrix4x4 model_matrix;
    AGFMatrix4x4 normal_matrix;
    AGFMatrix4x4 tmp_matrix;
    AGFVector3f transformed_points[8];
    AGFVector3f transformed_normals[24];
    AGFVertex3i projected_points[8];
    AGFDirectionalLight light;
    float scale = 80.0f;
    float camera_z = 330.0f;
    float focal = 190.0f;
    uint32_t i;
    uint32_t slice_index;

    agf_build_cube_mesh(&mesh);

    memset(p_image->m_data, 0, agf_image_size(p_image));
    agf_depth_buffer_clear(p_depth, AGF_DEPTH_MAX);

    light.m_nx = -6000;
    light.m_ny = -9000;
    light.m_nz = -12000;
    light.m_ambient = 42;
    light.m_intensity = 196;

    agf_matrix_scale(&scale_matrix, scale, scale, scale);
    agf_matrix_rotation_y(&rotation_y, p_angle);
    agf_matrix_rotation_x(&rotation_x, p_angle * 0.73f);
    agf_matrix_rotation_z(&rotation_z, p_angle * 0.31f);
    agf_matrix_multiply(&tmp_matrix, &rotation_x, &rotation_y);
    agf_matrix_multiply(&normal_matrix, &rotation_z, &tmp_matrix);
    agf_matrix_multiply(&model_matrix, &normal_matrix, &scale_matrix);

    for (i = 0; i < mesh.m_polygonHull.m_npoints; i++) {
        transformed_points[i] = agf_matrix_transform_point(&model_matrix, mesh.m_polygonHull.m_points[i]);
        agf_project_vertex(&transformed_points[i], &projected_points[i], p_image->m_width, p_image->m_height, camera_z, focal);
    }

    for (slice_index = 0; slice_index < mesh.m_nslices; slice_index++) {
        AGFMeshSlice3D *slice = &mesh.m_slices[slice_index];
        uint32_t attribute_index;
        uint32_t quad_index;

        for (attribute_index = 0; attribute_index < slice->m_nvertexAttributes; attribute_index++) {
            transformed_normals[attribute_index] = agf_matrix_transform_vector(&normal_matrix, slice->m_vertexAttributes[attribute_index].m_normal);
        }

        for (quad_index = 0; quad_index < slice->m_nquads; quad_index++) {
            AGFMeshQuad3D *quad = &slice->m_quads[quad_index];
            AGFVertex3n polygon[4];
            int32_t ax;
            int32_t ay;
            int32_t bx;
            int32_t by;
            int32_t winding;
            uint32_t vertex;

            for (vertex = 0; vertex < 4; vertex++) {
                uint32_t vertex_attribute_index = quad->m_indices[vertex];
                AGFVertexAttribute3f *vertex_attribute = &slice->m_vertexAttributes[vertex_attribute_index];
                uint32_t point_index = vertex_attribute->m_index;

                polygon[vertex].m_x = projected_points[point_index].m_x;
                polygon[vertex].m_y = projected_points[point_index].m_y;
                polygon[vertex].m_z = projected_points[point_index].m_z;
                polygon[vertex].m_nx = (int16_t)(transformed_normals[vertex_attribute_index].m_x * (float)AGF_NORMAL_SCALE);
                polygon[vertex].m_ny = (int16_t)(transformed_normals[vertex_attribute_index].m_y * (float)AGF_NORMAL_SCALE);
                polygon[vertex].m_nz = (int16_t)(transformed_normals[vertex_attribute_index].m_z * (float)AGF_NORMAL_SCALE);
            }

            ax = (int32_t)polygon[1].m_x - polygon[0].m_x;
            ay = (int32_t)polygon[1].m_y - polygon[0].m_y;
            bx = (int32_t)polygon[2].m_x - polygon[0].m_x;
            by = (int32_t)polygon[2].m_y - polygon[0].m_y;
            winding = ax * by - ay * bx;

            if (quad->m_polygonIndex < mesh.m_polygonHull.m_nquads && winding > 0) {
                agf_draw_polygon_lit_flat(p_image, p_depth, polygon, 4, &light);
            }
        }
    }
}

// We need to declare the libraries as "externally_visible" since we are using the option -fwhole-program in the Makefile
__attribute__((externally_visible)) struct ExecBase *SysBase;
__attribute__((externally_visible)) struct IntuitionBase *IntuitionBase = NULL;
__attribute__((externally_visible)) struct DosLibrary *DOSBase = NULL;
__attribute__((externally_visible)) struct UtilityBase *UtilityBase = NULL;
__attribute__((externally_visible)) struct GfxBase *GfxBase = NULL;
__attribute__((externally_visible)) struct Library *CxBase = NULL;
__attribute__((externally_visible)) struct Library *IconBase = NULL;

#if OPTION_USE_CLIB2
// Declare all clib2 constructors and destructors
extern int __ctor_stdlib_memory_init(void);
extern int __ctor_stdlib_program_name_init(void);
extern int __ctor_stdio_init(void);
extern int __ctor_stdio_file_init(void);
extern int __ctor_math_init(void);
#if OPTION_USE_BSD_SOCKET
extern int __ctor_socket_init(void);
#endif
extern int __ctor_arg_init(void);
extern int __ctor_rexxvars_init(void);
extern int __ctor_dirent_init(void);
extern int __ctor_locale_init(void);
extern int __ctor_clock_init(void);
extern int __ctor_unistd_init(void);
extern int __ctor_timer_init(void);
#if OPTION_USE_USER_GROUP
extern int __ctor_usergroup_init(void);
#endif

#if OPTION_USE_USER_GROUP
extern int __dtor_usergroup_exit();
#endif
extern int __dtor_timer_exit();
extern int __dtor_unistd_exit();
extern int __dtor_locale_exit();
extern int __dtor_dirent_exit();
extern int __dtor_rexxvars_exit();
#if OPTION_USE_BSD_SOCKET
extern int __dtor_socket_exit();
#endif
extern int __dtor_math_exit();
extern int __dtor_workbench_exit();
extern int __dtor_stdio_exit();
extern int __dtor_stdlib_program_name_exit();
extern int __dtor_stdlib_memory_exit();
extern int __dtor___wildcard_expand_exit();
extern int __dtor_alloca_exit();
extern int __dtor___setenv_exit();
extern int __dtor___chdir_exit();
#endif

#if OPTION_USE_CLIB2
// Calls all constructors
static VOID fcntCallCtor(void)
{    
    __ctor_stdlib_memory_init();
    __ctor_stdlib_program_name_init();
    __ctor_stdio_init();
    __ctor_stdio_file_init();
    __ctor_math_init();
#if OPTION_USE_BSD_SOCKET
    __ctor_socket_init();
#endif
    __ctor_arg_init();
    __ctor_rexxvars_init();
    __ctor_dirent_init();
    __ctor_locale_init();
    __ctor_clock_init();
    __ctor_unistd_init();
    __ctor_timer_init();
#if OPTION_USE_USER_GROUP
    __ctor_usergroup_init();
#endif
}

// Calls all destructors
static VOID fcntCallDtor(void)
{    
    __dtor___wildcard_expand_exit();
    __dtor_alloca_exit();
    __dtor___setenv_exit();
    __dtor___chdir_exit();
#if OPTION_USE_USER_GROUP
    __dtor_usergroup_exit();
#endif
    __dtor_timer_exit();
    __dtor_unistd_exit();
    __dtor_locale_exit();
    __dtor_dirent_exit();
    __dtor_rexxvars_exit();
#if OPTION_USE_BSD_SOCKET
    __dtor_socket_exit();
#endif
    __dtor_math_exit();
    __dtor_workbench_exit();
    __dtor_stdio_exit();
    __dtor_stdlib_program_name_exit();
    __dtor_stdlib_memory_exit();
}
#endif

// Does the clean up of all ressources
void cleanUP(void)
{
    if (IntuitionBase != NULL) {
        CloseLibrary((struct Library *)IntuitionBase);
    }

    if (DOSBase != NULL) {
        CloseLibrary((struct Library *)DOSBase);
    }

    if (UtilityBase != NULL) {
        CloseLibrary((struct Library *)UtilityBase);
    }

    if (GfxBase != NULL) {
        CloseLibrary((struct Library *)GfxBase);
    }

    if (CxBase != NULL) {
        CloseLibrary(CxBase);
    }

    if (IconBase != NULL) {
        CloseLibrary(IconBase);
    }
}

int main(int argc, char **argv);

__attribute__((used)) __attribute__((section(".text.unlikely"))) void _start(int argc, char **argv)
{
    SysBase = *((struct ExecBase**)4UL);
    struct Process *proc;
    struct WBStartup *wbmsg = NULL;
    
    // We check if our program has been started by the workbench or by the shell
    proc = (struct Process *)FindTask(NULL);

    if (proc->pr_CLI == NULL) {
        // We have been launched by workbench: we wait for the startup message
        WaitPort(&proc->pr_MsgPort);
        wbmsg = (struct WBStartup *)GetMsg(&proc->pr_MsgPort);
    }

#if OPTION_USE_CLIB2
    __WBenchMsg = wbmsg; // this variable is used in __ctor_arg_init()
    __exit_blocked = FALSE; // this will push exit() and similar functions to longjmp to target set by setjmp()
#endif

    // We open the libraries (required since we are linking with alib; please see https://github.com/jyoberle/alib for details)
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((CONST_STRPTR)"intuition.library",0L);
    DOSBase = (struct DosLibrary *)OpenLibrary((CONST_STRPTR)"dos.library",0L);
    UtilityBase = (struct UtilityBase *)OpenLibrary((CONST_STRPTR)"utility.library",0L);
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);
    CxBase = OpenLibrary("commodities.library",0);
    IconBase = OpenLibrary("icon.library",0);

    if (IntuitionBase != NULL && DOSBase != NULL && UtilityBase != NULL && GfxBase != NULL && CxBase != NULL && IconBase != NULL) {
#if OPTION_USE_CLIB2
        __UtilityBase = (struct Library *)UtilityBase; // we need to add this because of macro DECLARE_UTILITYBASE()

        if(setjmp(__exit_jmp_buf) != 0) {
            goto out; // target for exit
        }

        fcntCallCtor(); // call the constructors

        printf("\n");
#endif

        main(argc,argv);

#if OPTION_USE_CLIB2
        out:
        fcntCallDtor(); // call the destructors
#endif
    }

    // Before leaving, we close all libraries
    cleanUP();

    if (wbmsg != NULL) {
        // If needed, we reply to the workbench message
        Forbid();
        ReplyMsg((struct Message *)wbmsg);
    }
}

int main(int argc, char **argv) 
{
    printf("Thanks for running Amiga Graphics Framework example\n");
    printf("Programmed by Gianpaolo Ingegneri / TextureMind. Started from a small example\n");
    printf("created in 2007 with StormC 3.0 and continued in 2026 with Visual Studio Code\n");
    printf("(Abyss / Job extension). C2P algorithm coded in 1998 by Kalms\n\n");

    struct Screen *scr;

    scr = OpenScreenTags(NULL,
                         SA_Title,(ULONG)"Chunky to planar example",
                         SA_DisplayID, 0x00000000,
                         SA_Depth, 8,
    //                   SA_ShowTitle, FALSE,
    //                   SA_Draggable, TRUE,
                         SA_FullPalette, TRUE,
                         SA_Exclusive, TRUE,
                         TAG_DONE);

    if (scr == NULL) {
        printf("Unable to open full screen with 320x256x8bit (AGA) resolution\n");
        return 0;
    }

    int x, y, n = 0;

    // Change palette on screen to gray scale colors

    for (y = 0; y < 256; y++) {
        SetRGB32( &scr->ViewPort, y, (y << 24), (y << 24), (y << 24));
    }

    AGFImage *screenImage = agf_image_alloc_8(320, 240);
    AGFImage *textureImage = agf_image_alloc_8(256, 256);
    AGFDepthBuffer *depthBuffer = agf_depth_buffer_alloc(320, 240);
    AGFTextureAnimation *textureAnimation = NULL;

    if (screenImage == NULL || textureImage == NULL || depthBuffer == NULL) {
        printf("Unable to allocate image buffers\n");
        agf_image_free(screenImage);
        agf_image_free(textureImage);
        agf_depth_buffer_free(depthBuffer);
        CloseScreen(scr);
        return 0;
    }

    // Init data for c2p conversion

#define USE_C2P_KALMS 

#ifndef USE_C2P_KALMS
    struct c2pStruct c2p;

    c2p.startX = 0;
    c2p.startY = 0;
    c2p.width = screenImage->m_width;
    c2p.height = screenImage->m_height;
    c2p.ChunkyBuffer = screenImage->m_data;
    c2p.bmap = scr->RastPort.BitMap;
#endif
    // Load image from file

    FILE *stream;
    stream = fopen("floortexture.raw", "rb");
    if (stream == NULL) {
        printf("Unable to open floortexture.raw file. Generate animated blobs texture instead\n");
        textureAnimation = agf_texture_animation_alloc(AGF_TEXTURE_ANIMATION_BLOBS, textureImage->m_width, textureImage->m_height, 8);
        if (textureAnimation == NULL) {
            agf_texture_generate_blobs(textureImage, 8);
        } else {
            agf_texture_animation_render(textureAnimation, textureImage);
        }
    } else {
        fread(textureImage->m_data, agf_image_size(textureImage), 1, stream);
        fclose(stream);
    }

#define TEXTURE_MAPPING

#ifdef TEXTURE_MAPPING
    float angle, scale;          

    int sinas, cosas, xc, yc, xlong, ylong;
    int col1, col2, col3, col4, c1, c2;
    int sx1, sy1, sx2, sy2, sx3, sy3, u, v, tx, ty, tx2, ty2, xscale;
    float cx, cy, cz, sx, sy, stx, hx, hy;
    float sinAngle, cosAngle;

    unsigned char *c_scan;

    int width = screenImage->m_width;
    int height = screenImage->m_height;
    float center = 256;
    float py = 256;

    int y2 = 0;

    int example = 1;
    int mouseRightDown = 0;

    while (!mouseLeft()) {
        angle = ((float)(y2++) * 1.5f / 180.0f) * 3.14f;

        sinAngle = sin(angle);
        cosAngle = cos(angle);

        if (mouseRight() && mouseRightDown == 0) {
            example = (example + 1) % 3;
            if (example == 1) {
                memset(screenImage->m_data, 0, agf_image_size(screenImage));
            }

            mouseRightDown = 1;
        } else if (!mouseRight() && mouseRightDown == 1) {
            mouseRightDown = 0;
        }

        if (textureAnimation != NULL) {
            agf_texture_animation_render(textureAnimation, textureImage);
        }

        if (example == 2) { // Rotating depth buffered cube
            drawRotatingCube(screenImage, depthBuffer, angle);
        } else if (example == 0) { // Image rotation with bilinear filtering
            scale = 1.0f - ((cos(angle) + 1.0f) / 2.25f);

            sinas = sin(-angle) * 65536 * scale;
            cosas = cos(-angle) * 65536 * scale;
            xc = (width / 2) * 65536 - (100 * (cosas + sinas));
            yc = (height / 2) * 65536 - (100 * (cosas - sinas));

            for (y = 0; y < height; y++) {
                xlong = xc;
                ylong = yc;

                c_scan = &screenImage->m_data[y * screenImage->m_stride];

                for (x = 0; x < width; x++) {
                    tx = xlong >> 16;
                    ty = ylong >> 16;

                    col1 = textureImage->m_data[((tx + 0) & 0xFF) + ((ty + 0) & 0xFF) * textureImage->m_stride];
                    col2 = textureImage->m_data[((tx + 1) & 0xFF) + ((ty + 0) & 0xFF) * textureImage->m_stride];
                    col3 = textureImage->m_data[((tx + 0) & 0xFF) + ((ty + 1) & 0xFF) * textureImage->m_stride];
                    col4 = textureImage->m_data[((tx + 1) & 0xFF) + ((ty + 1) & 0xFF) * textureImage->m_stride];

                    u = xlong & 0x0000FFFF;
                    v = ylong & 0x0000FFFF;

                    c1 = (((col2 - col1) * u) >> 16) + col1;
                    c2 = (((col4 - col3) * u) >> 16) + col3;
                    col1 = (((c2 - c1) * v) >> 16) + c1;

                    *c_scan++ = col1;

                    xlong += cosas;
                    ylong -= sinas;
                }

                xc += sinas;
                yc += cosas;
            }
        } else { // Floor rotation with bilinear filtering
            for (y = 100; y < height; y++) {
                cz = ((py * 256.0f) / ((float)(y + 50) - ((float)height / 2.0f))) - center;
                cx = (-((float)width / 2.0f) * (cz + center)) / 256.0f;
                cy = cz;

                cx = cx - 0;
                cy = cy - 192;

                hx = cx * cosAngle - cy * sinAngle;
                hy = cx * sinAngle + cy * cosAngle;

                hx /= 4;
                hy /= 4;

                hx = hx + 0;
                hy = hy + 192;

                stx = (cz + center) / 256.0f;
                sx = stx / 4 * cosAngle;
                sy = stx / 4 * sinAngle;

                sx1 = (long)(sx * 65536.0f);
                sy1 = (long)(sy * 65536.0f);
                xlong = (long)(hx * 65536.0f);
                ylong = (long)(hy * 65536.0f);

                c_scan = &screenImage->m_data[y * screenImage->m_stride];

                for (x = 0; x < width; x++) {
                    tx = xlong >> 16;
                    ty = ylong >> 16;

                    col1 = textureImage->m_data[((tx + 0) & 0xFF) + ((ty + 0) & 0xFF) * textureImage->m_stride];
                    col2 = textureImage->m_data[((tx + 1) & 0xFF) + ((ty + 0) & 0xFF) * textureImage->m_stride];
                    col3 = textureImage->m_data[((tx + 0) & 0xFF) + ((ty + 1) & 0xFF) * textureImage->m_stride];
                    col4 = textureImage->m_data[((tx + 1) & 0xFF) + ((ty + 1) & 0xFF) * textureImage->m_stride];

                    u = xlong & 0x0000FFFF;
                    v = ylong & 0x0000FFFF;

                    c1 = (((col2 - col1) * u) >> 16) + col1;
                    c2 = (((col4 - col3) * u) >> 16) + col3;
                    col1 = (((c2 - c1) * v) >> 16) + c1;

                    *c_scan++ = (col1 * y) >> 8;

                    xlong += sx1;
                    ylong += sy1;
                }
            }
        }

#ifdef USE_C2P_KALMS
        chunkyToPlanar2Init(screenImage->m_width, screenImage->m_height, 0);
        chunkyToPlanar2(screenImage->m_data, scr->RastPort.BitMap->Planes[0]);
#else
        chunkyToPlanar(&c2p);
#endif
        waitVbl();
    }
#else
    // simple blit copy
    for (y = 0; y < screenImage->height; y++) {
        memcpy(&screenImage->data[y * screenImage->stride], &textureImage->data[y * textureImage->stride], textureImage->width);
    }

#ifdef USE_C2P_KALMS
        chunkyToPlanar2Init(screenImage->width, screenImage->height, 0);
        chunkyToPlanar2(screenImage->data, scr->RastPort.BitMap->Planes[0]);
#else
        chunkyToPlanar(&c2p);
#endif
#endif

    agf_texture_animation_free(textureAnimation);
    agf_depth_buffer_free(depthBuffer);
    agf_image_free(screenImage);
    agf_image_free(textureImage);

    CloseScreen(scr);
}
