#if !defined(RUINENGLASS_RENDERER_OPENGL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// RESOURCE: https://registry.khronos.org/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
// or https://registry.khronos.org/OpenGL/extensions/EXT/EXT_framebuffer_sRGB.txt
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_MULTISAMPLE_ARB                0x809D

// NOTE(chowie): Windows-specific
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// RESOURCE: https://guide.handmadehero.org/code/day242/
// TODO(chowie): Texture downloads
// TODO(chowie): Hash table would be better. However, since this is
// only at startup, not that bad.
struct opengl_info
{
    b32x ModernContext;

    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    char *Extensions;

    b32x GL_EXT_texture_sRGB;
    b32x GL_ARB_framebuffer_sRGB;
};

#define RUINENGLASS_RENDERER_OPENGL_H
#endif
