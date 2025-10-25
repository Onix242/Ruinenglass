#if !defined(RUINENGLASS_RENDERER_OPENGL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#define GL_NUM_EXTENSIONS                 0x821D

#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C

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
//    char *Extensions;

    b32x GL_EXT_texture_sRGB;
    b32x GL_ARB_framebuffer_sRGB;
};

// TODO(chowie): Switch over to "type_" and not underscoring
// everything for less typing and slightly easier automation (if you
// wanted reflection). Change this for GetInputState stub?
// HmH D371 2:06:05 - You could use a #include file with all of them for macro tricks!
// TODO(chowie): HmH D371 2:20:30 - Automate this with a function
// table like in Win32? Maybe change the function names to lowercase
// instead of "type_"?
#define GlobalFunctionStub(Name) global type_##Name *Name

GlobalFunctionStub(wglCreateContextAttribsARB);
GlobalFunctionStub(wglChoosePixelFormatARB);
GlobalFunctionStub(wglSwapIntervalEXT);
GlobalFunctionStub(wglGetExtensionsStringEXT);
GlobalFunctionStub(glGetStringi);

// TODO(chowie): IMPORTANT(chowie): Move all function stubs here!
struct opengl
{
};

// RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/1011-opengl_srgb__assets
// TODO(chowie): Remove srgb check?

#define RUINENGLASS_RENDERER_OPENGL_H
#endif
