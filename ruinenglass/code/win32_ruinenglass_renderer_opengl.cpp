/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hshareContext,
                                                    const int *attribList);
global wgl_create_context_attribs_arb *wglCreateContextAttribsARB;

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
                                                const int *piAttribIList,
                                                const FLOAT *pfAttribFList,
                                                UINT nMaxFormats,
                                                int *piFormats,
                                                UINT *nNumFormats);
global wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;

typedef BOOL WINAPI wgl_swap_interval_ext(int interval); // NOTE: Marking with a WINAPI might matter for 32-bit code, not 64-bit!
global wgl_swap_interval_ext *wglSwapIntervalEXT;

typedef const char *WINAPI wgl_get_extensions_string_ext(void);
global wgl_get_extensions_string_ext *wglGetExtensionsStringEXT;

global b32x OpenGLSupportsSRGBFramebuffer;

internal void
Win32SetPixelFormat(HDC WindowDC)
{
    int SuggestedPixelFormatIndex = 0;
    GLuint ExtendedPick = 0;
    if(wglChoosePixelFormatARB)
    {
        // TODO(chowie): This needs to happen after we create the
        // initial OpenGL context, but how do we do that given that
        // the DC needs to be in the correct format first? Do we just
        // wglMakeCurrent back to zero, _then_ reset the pixel format?
        int IntAttribList[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // RESOURCE: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
                WGL_COLOR_BITS_ARB,     24,
                WGL_DEPTH_BITS_ARB,     24, // TODO(chowie): Use depth?
                WGL_STENCIL_BITS_ARB,   8, // TODO(chowie): Use stencil?
                WGL_SAMPLE_BUFFERS_ARB, 1, // RESOURCE: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
                WGL_SAMPLES_ARB,        4, // TODO(chowie): Set 2x, 4x etc. MSAA?
                0,
            };

        if(!OpenGLSupportsSRGBFramebuffer)
        {
            IntAttribList[10] = 0;
        }

        wglChoosePixelFormatARB(WindowDC, IntAttribList, 0, 1,
                                &SuggestedPixelFormatIndex, &ExtendedPick);
    }

    if(!ExtendedPick)
    {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.cColorBits = 24;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.cDepthBits = 24;
        DesiredPixelFormat.cStencilBits = 8;

        SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    }

    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
}

internal void
Win32LoadWGLExtensions(void)
{
    WNDCLASSA WindowClass = {};
    WindowClass.lpfnWndProc = DefWindowProcA;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = "WGLLoader";

    // NOTE(chowie): Create a dummy window to load the pixel format.
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(0, WindowClass.lpszClassName, "RuinenglassOpenGL",
                            0,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            0, 0, WindowClass.hInstance, 0);

        HDC WindowDC = GetDC(Window);
        Win32SetPixelFormat(WindowDC);

        // RESOURCE: https://computergraphics.stackexchange.com/questions/8311/how-do-i-create-a-win32-window-with-a-vulkan-context
        // STUDY(chowie): OpenGL context ties together the concept of extension management, memory management, draw commands and surface presentation. In Vulkan those are all managed through independent interfaces.
        HGLRC OpenGLRC = wglCreateContext(WindowDC);
        if(wglMakeCurrent(WindowDC, OpenGLRC))
        {
            wglChoosePixelFormatARB =
                (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB =
                (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");
            // NOTE(chowie): wglGetProcAddress is not technically what
            // you do, check for the existance of extensions by
            // checking a string to the drivers. But you still need to
            // make the function call after the fact. Casey thinks
            // this is better though, in theory you could get
            // something different. Swap interval is the only way to
            // use this function.
            wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");

            if(wglGetExtensionsStringEXT)
            {
                char *Extensions = (char *)wglGetExtensionsStringEXT();
                char *At = Extensions;
                while(*At)
                {
                    while(IsWhitespace(*At)) {++At;}
                    char *End = At;
                    while(*End && !IsWhitespace(*End)) {++End;}

                    umm Count = End - At;

                    if(0) {}
                    else if(StringsAreEqual(Count, At, "WGL_EXT_framebuffer_sRGB")) {OpenGLSupportsSRGBFramebuffer = true;}
                    else if(StringsAreEqual(Count, At, "WGL_ARB_framebuffer_sRGB")) {OpenGLSupportsSRGBFramebuffer = true;}

                    At = End;
                }
            }

            wglMakeCurrent(0, 0);
        }

        wglDeleteContext(OpenGLRC);
        ReleaseDC(Window, WindowDC);
        DestroyWindow(Window);
    }
}

// RESOURCE(martins): https://git.handmade.network/hmn/gitlab_snippets/src/branch/master/mmozeiko/win32_opengl.c
// NOTE(chowie): Modern OpenGL version.
global int
Win32OpenGLAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
#if HANDMADE_INTERNAL
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, // NOTE(chowie): Enable this for testing WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB (cannot use this flag with deprecated OpenGL calls)
#endif
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0,
};

// STUDY(chowie): Each thread has implicit OpenGLRC. Sets up
// RenderingContext (RC) for your thread.
internal HGLRC
Win32InitOpenGL(HDC WindowDC, b32x EnableVsync)
{
    Win32LoadWGLExtensions();

    b32x ModernContext = true;
    HGLRC OpenGLRC = 0;
    Win32SetPixelFormat(WindowDC);
    if(wglCreateContextAttribsARB)
    {
        // NOTE(chowie): SetPixelFormat should be here? But it won't
        // set for without modern context!
        OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, Win32OpenGLAttribs);
    }

    if(!OpenGLRC)
    {
        ModernContext = false;
        OpenGLRC = wglCreateContext(WindowDC); // NOTE(chowie): Must be _after_ SetPixelFormat
    }

    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
        OpenGLInit(ModernContext, OpenGLSupportsSRGBFramebuffer);
        if(wglSwapIntervalEXT)
        {
            wglSwapIntervalEXT(EnableVsync ? true : false); // TODO(chowie): How long this is for a 120hz display vs 60hz display?
        }
    }

    return(OpenGLRC);
}
