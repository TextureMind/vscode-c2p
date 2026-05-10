
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <clib/alib_protos.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include "g_misc.h"

#define OPTION_USE_CLIB2 1
#define OPTION_USE_BSD_SOCKET 0
#define OPTION_USE_USER_GROUP 0

#if OPTION_USE_CLIB2
#include <stdlib_headers.h>
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
        __UtilityBase = UtilityBase; // we need to add this because of macro DECLARE_UTILITYBASE()

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
    printf("Thanks for running Floor Rotation with bilinear filtering example\n");
    printf("Programmed by Gianpaolo Ingegneri / TextureMind in 2007 with StormC 3.0\n");
    printf("and reworked with Visual Studio Code (Abyss / Job extension) in 2025\n");
    printf("Original Chunky To Planar algorithm coded in 1994 by Morten Eriksen\n\n");

    struct Screen *scr;

    scr = OpenScreenTags(NULL,
                         SA_Title,"Chunky to planar example",
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

    unsigned char *dstImage;
    dstImage = (unsigned char *)malloc(320 * 256);

    memset(dstImage, 0, 320 * 256);

    unsigned char *srcImage;
    srcImage = (unsigned char *)malloc(256 * 256);

    // Init data for c2p conversion

    struct c2pStruct c2p;

    c2p.startX = 0;
    c2p.startY = 0;
    c2p.width = 320;
    c2p.height = 240;
    c2p.ChunkyBuffer = dstImage;
    c2p.bmap = scr->RastPort.BitMap;

    // Load image from file

    FILE *stream;
    stream = fopen("floortexture.raw", "rb");
    if (stream == NULL) {
        printf("Unable to open floortexture.raw file. Draw gray instead\n");
        memset(srcImage, 128,  256 * 256);
    } else {
        fread(srcImage, 256 * 256, 1, stream);
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

    int width = 320;
    int height = 240;
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
            example = example == 0 ? 1 : 0;
            if (example == 1) {
                memset(dstImage, 0, 320 * 256);
            }

            mouseRightDown = 1;
        } else if (!mouseRight() && mouseRightDown == 1) {
            mouseRightDown = 0;
        }

        if (example == 0) { // Image rotation with bilinear filtering
            scale = 1.0f - ((cos(angle) + 1.0f) / 2.25f);

            sinas = sin(-angle) * 65536 * scale;
            cosas = cos(-angle) * 65536 * scale;
            xc = (width / 2) * 65536 - (100 * (cosas + sinas));
            yc = (height / 2) * 65536 - (100 * (cosas - sinas));

            for (y = 0; y < height; y++) {
                xlong = xc;
                ylong = yc;

                c_scan = &dstImage[y * width];

                for (x = 0; x < width; x++) {
                    tx = xlong >> 16;
                    ty = ylong >> 16;

                    col1 = srcImage[((tx + 0) & 0xFF) + ((ty + 0) & 0xFF) * 256];
                    col2 = srcImage[((tx + 1) & 0xFF) + ((ty + 0) & 0xFF) * 256];
                    col3 = srcImage[((tx + 0) & 0xFF) + ((ty + 1) & 0xFF) * 256];
                    col4 = srcImage[((tx + 1) & 0xFF) + ((ty + 1) & 0xFF) * 256];

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

                c_scan = &dstImage[y * width];

                for (x = 0; x < width; x++) {
                    tx = xlong >> 16;
                    ty = ylong >> 16;

                    col1 = srcImage[((tx + 0) & 0xFF) + ((ty + 0) & 0xFF) * 256];
                    col2 = srcImage[((tx + 1) & 0xFF) + ((ty + 0) & 0xFF) * 256];
                    col3 = srcImage[((tx + 0) & 0xFF) + ((ty + 1) & 0xFF) * 256];
                    col4 = srcImage[((tx + 1) & 0xFF) + ((ty + 1) & 0xFF) * 256];

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

        chunkyToPlanar(&c2p);

        waitVbl();
    }
#else
    // simple blit copy
    for (y = 0; y < 256; y++) {
        memcpy(&dstImage[y * 320], &srcImage[y * 256], 256);
    }

    chunkyToPlanar(&c2p);
#endif

    free(dstImage);
    free(srcImage);

    CloseScreen(scr);
}
