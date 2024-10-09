/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass_renderer_opengl.h"

internal opengl_info
OpenGLGetInfo(b32x ModernContext)
{
    opengl_info Result = {};

    Result.ModernContext = ModernContext;
    Result.Vendor = (char *)glGetString(GL_VENDOR);
    Result.Renderer = (char *)glGetString(GL_RENDERER);
    Result.Version = (char *)glGetString(GL_VERSION);
    if(Result.ModernContext)
    {
        Result.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    }
    else
    {
        Result.ShadingLanguageVersion = "(none)";
    }

    Result.Extensions = (char *)glGetString(GL_EXTENSIONS);

    char *At = Result.Extensions;
    while(*At)
    {
        while(IsWhitespace(*At)) {++At;}
        char *End = At;
        while(*End && !IsWhitespace(*End)) {++End;}

        umm Count = End - At;

        if(0) {}
        else if(StringsAreEqual(Count, At, "GL_EXT_texture_sRGB")) {Result.GL_EXT_texture_sRGB = true;}
        else if(StringsAreEqual(Count, At, "GL_EXT_framebuffer_sRGB")) {Result.GL_ARB_framebuffer_sRGB = true;}
        else if(StringsAreEqual(Count, At, "GL_ARB_framebuffer_sRGB")) {Result.GL_ARB_framebuffer_sRGB = true;}

        At = End;
    }

    return(Result);
}

// RESOURCE: https://registry.khronos.org/OpenGL/api/GL/glcorearb.h
// RESOURCE: https://registry.khronos.org/OpenGL/extensions/EXT/EXT_sRGB.txt
internal void
OpenGLInit(b32x ModernContext, b32x FramebufferSupportsSRGB)
{
    opengl_info Info = OpenGLGetInfo(ModernContext);

    // NOTE(chowie): If we can go full sRGB on the texture side and
    // the framebuffer side, we can enable it. Otherwise, it is safer
    // for us to pass it straight through.
    OpenGLDefaultInternalTextureFormat = GL_RGBA8;
    // TODO(chowie): Actually check for extensions!
    if(FramebufferSupportsSRGB && Info.GL_EXT_texture_sRGB &&
       Info.GL_ARB_framebuffer_sRGB)
    {
        OpenGLDefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
}

// TODO(chowie): Triangle Strip? Turn OpenGL circle too
// NOTE(chowie): Extra parameters to account for 1-pixel apron
inline void
OpenGLRect(v3 MinP, v3 MaxP, v4 Colour, v2 MinUV = V2(0, 0), v2 MaxUV = V2(1, 1))
{
    glBegin(GL_TRIANGLES);

    glColor4fv(Colour.E);

    // TODO(chowie): Use fv (vector) op! Access with .E
    // NOTE(chowie): Lower triangle
    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex3f(MinP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MinUV.y);
    glVertex3f(MaxP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MaxUV.y);
    glVertex3f(MaxP.x, MaxP.y, MinP.z);

    // NOTE(chowie): Upper triangle
    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex3f(MinP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MaxUV.y);
    glVertex3f(MaxP.x, MaxP.y, MinP.z);

    glTexCoord2f(MinUV.x, MaxUV.y);
    glVertex3f(MinP.x, MaxP.y, MinP.z);

    glEnd();
}

// RESOURCE(inigo quilez): Eerily similar - https://iquilezles.org/articles/sincos/
// RESOURCE(SiegeLord): Eerily similar - https://siegelord.net/circle_draw
// RESOURCE(ratchetfreak): https://hero.handmade.network/forums/code-discussion/t/1018-2d_rotation_help
// TODO(chowie): General rotation for other shapes other than circles
// RESOURCE: https://stackoverflow.com/questions/1569939/rendering-different-triangle-types-and-triangle-fans-using-vertex-buffer-objects
// TODO(chowie): Change GL_TRIANGLE_FAN to glDrawArrays
// RESOURCE: https://stackoverflow.com/questions/8762826/texture-mapping-a-circle-made-using-gl-polygon
// TODO(chowie): Proper texturing of circles? Should the MinUV really
// be 0.5f? GlTexCoord?
// RESOURCE(blatnik): https://blog.bearcats.nl/seamlessly-subdivide-circle/
// IMPORTANT(chowie): Best explanation by ohAitch. To "triangulate a
// circle" is roughly "get some fractional powers of -1", doing the
// fraction once (outside the loop) and then raising _it_ to succesive
// integer powers. One sin and one cos per/vertex -> one sin per circle,
// with a v2 orientation and rotation matrix mult cost.
inline void
OpenGLCircle(v3 CentreP, r32 Radius, u32 TriCount,
             v4 Colour, v2 MinUV = V2(0, 0), v2 MaxUV = V2(1, 1))
{
    v2 OrientationP = V2(0, Radius);
    m2x2 Rot = M2x2RotationByTris((r32)TriCount);

    // NOTE(chowie): Fan vector version default is clockwise.
    // Anticlockwise +cos +sin <=> (0, -1), Clockwise +cos -sin <=> (0, 1)
    glBegin(GL_TRIANGLE_FAN);
    glColor4fv(Colour.E);
//    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex2f(CentreP.x, CentreP.y);

    for(u32 TriangleIndex = 0;
        TriangleIndex <= TriCount;
        ++TriangleIndex)
    {
        glVertex2f(CentreP.x + OrientationP.x, CentreP.y + OrientationP.y);
        // NOTE(chowie): Orientation after glVertex, starts drawing from top
        OrientationP = Rot*OrientationP;
    }
//    glTexCoord2f(MaxUV.x, MaxUV.y);
    glEnd();
}

/*
inline void
OpenGLCircle(v3 CentreP, r32 Radius, r32 Error,
             v4 Colour, v2 MinUV = V2(0.5f, 0.5f), v2 MaxUV = V2(1, 1))
{
    r32 theta = (r32)acos(1 - Error / Radius);
    int n = (int)ceil(fmax(Pi32 / theta, 3));

    glBegin(GL_TRIANGLE_FAN);

    glColor4fv(Colour.E);
    glVertex2f(CentreP.x, CentreP.y);

    for(int i = 0;
        i <= n;
        ++i)
    {
        r32 sx = CentreP.x + Radius * Cos(Tau32 * i / n);
        r32 sy = CentreP.y + Radius * Sin(Tau32 * i / n);
        glVertex2f(sx, sy);
    }
    glEnd();
}
*/

inline void
OpenGLSetScreenSpace(v2u Dim)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // TODO(chowie): Matrix with rotation?
    // STUDY(chowie): Identity Matrix (similar to glOrtho),
    // fix-function pipeline by column vectors (see placement of -1)
    glMatrixMode(GL_PROJECTION);
    r32 a = SafeRatio1(2.0f, (r32)Dim.Width);
    r32 b = SafeRatio1(2.0f, (r32)Dim.Height);
    r32 Proj[] =
    {
         a,  0,  0,  0,
         0,  b,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1,
    };
    glLoadMatrixf(Proj);
}

internal void
OpenGLRenderCommands(game_render_commands *Commands, v2u WindowDim)
{
    glViewport(0, 0, WindowDim.Width, WindowDim.Height);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE_ARB);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND); // STUDY(chowie): Blends still remains in fixed-function pipeline
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    OpenGLSetScreenSpace(Commands->Dim);

//    u32 ClipRect = 0xFFFFFFFF; // STUDY: Set ClipRect to something that cannot be true, so we set it everytime, instead of force setting to 0
    for(u8 *HeaderAt = Commands->PushBufferBase;
        HeaderAt < Commands->PushBufferDataAt;
        )
    {
        render_group_entry_header *Header = (render_group_entry_header *)HeaderAt;
        HeaderAt += sizeof(render_group_entry_header);
        void *Data = (u8 *)Header + sizeof(*Header);

        switch(Header->Type)
        {
            case RenderGroupEntryType_render_entry_clear:
            {
                HeaderAt += sizeof(render_entry_clear);
                render_entry_clear *Entry = (render_entry_clear *)Data;

                glClearColor(Entry->Colour.r, Entry->Colour.g, Entry->Colour.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_rect:
            {
                HeaderAt += sizeof(render_entry_rect);
                render_entry_rect *Entry = (render_entry_rect *)Data;

                // TODO(chowie): What do I do with a v3?
                glDisable(GL_TEXTURE_2D);
                OpenGLRect(Entry->P, Entry->P + V3(Entry->Dim, 0), Entry->Colour);
                glEnable(GL_TEXTURE_2D);
            } break;

            case RenderGroupEntryType_render_entry_circle:
            {
                HeaderAt += sizeof(render_entry_circle);
                render_entry_circle *Entry = (render_entry_circle *)Data;

                glDisable(GL_TEXTURE_2D);
                OpenGLCircle(Entry->P, Entry->Radius, Entry->Tris, Entry->Colour);
                glEnable(GL_TEXTURE_2D);
            } break;

            InvalidDefaultCase;
        }
    }
}

// TODO(chowie): Textures (with multithreading) & scissor
/*
case RenderGroupEntryType_render_entry_bitmap:
{
    render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
    Assert(Entry->Bitmap);

    // NOTE: Guard for 0-width bitmap for some reason to
    // prevent crashing; rather not render anything.
    if(Entry->Bitmap->Width && Entry->Bitmap->Height)
    {
        v2 XAxis = {1, 0};
        v2 YAxis = {0, 1};
        v2 MinP = Entry->P;
        v2 MaxP = MinP + Entry->Size.x*XAxis + Entry->Size.y*YAxis;

        // TODO: Hold the frame if we are not ready with the texture?
        // TODO(chowie): Check that type cast to umm works fine?
        glBindTexture(GL_TEXTURE_2D, (GLuint)U32FromPointer(Entry->Bitmap->TextureHandle));
        r32 OneTexelU = 1.0f / (r32)Entry->Bitmap->Width;
        r32 OneTexelV = 1.0f / (r32)Entry->Bitmap->Height;
        v2 MinUV = V2(OneTexelU, OneTexelV);
        v2 MaxUV = V2(1.0f - OneTexelU, 1.0f - OneTexelV);
        OpenGLRectangle(Entry->P, MaxP, Entry->Colour, MinUV, MaxUV);
    }
} break;

if(ClipRectIndex != Header->ClipRectIndex)
{
    ClipRectIndex = Header->ClipRectIndex;
    Assert(ClipRectIndex < Commands->ClipRectCount);

    // RESOURCE: https://docs.gl/gl2/glScissor
    render_entry_cliprect *Clip = Commands->ClipRects + ClipRectIndex;
    glScissor(Clip->Rect.MinX, Clip->Rect.MinY,
              Clip->Rect.MaxX - Clip->Rect.MinX,
              Clip->Rect.MaxY - Clip->Rect.MinY);
}
*/
