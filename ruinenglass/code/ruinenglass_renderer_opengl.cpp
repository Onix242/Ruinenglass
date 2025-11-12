/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

GL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
    // TODO(chowie): Check severity for medium/high messages?
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        char *ErrorMessage = (char *)message;
        Assert(!"OpenGL error encountered");
    }
}

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

    GLint ExtensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
    for(GLint ExtensionIndex = 0;
        ExtensionIndex < ExtensionCount;
        ++ExtensionIndex)
    {
        char *ExtensionName = (char *)glGetStringi(GL_EXTENSIONS, ExtensionIndex);

        // STUDY: If 0 is to make everything an else if, being a bit clever
        if(0) {}
        else if(StringsAreEqual(ExtensionName, "GL_EXT_texture_sRGB")) {Result.GL_EXT_texture_sRGB = true;}
        else if(StringsAreEqual(ExtensionName, "GL_EXT_framebuffer_sRGB")) {Result.GL_ARB_framebuffer_sRGB = true;}
        else if(StringsAreEqual(ExtensionName, "GL_ARB_framebuffer_sRGB")) {Result.GL_ARB_framebuffer_sRGB = true;}
        // TODO: Make sure this works!
    }

    // TODO(chowie): Ref HmH 323, only available OpenGL
    // 3.0+. Otherwise, you'd have to parse the string
    GLint Major = 1;
    GLint Minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &Major);
    glGetIntegerv(GL_MINOR_VERSION, &Minor);
    if((Major > 2) || ((Major == 2) && (Minor >= 1)))
    {
        // NOTE(chowie): Should have srgb textures in 2.1+ automatically
        Result.GL_EXT_texture_sRGB = true;
    }

    return(Result);
}

// RESOURCE: https://registry.khronos.org/OpenGL/api/GL/glcorearb.h
// RESOURCE: https://registry.khronos.org/OpenGL/extensions/EXT/EXT_sRGB.txt
internal void
OpenGLInit(b32x ModernContext, opengl_info Info, b32x FramebufferSupportsSRGB)
{
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
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // NOTE(chowie): This wouldn't work for multi-texturing env!

#if RUINENGLASS_INTERNAL
    if(glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackARB(OpenGLDebugCallback, 0);
    }
#endif
}

// TODO(chowie): Remove this (in HmH 359), use 3D coord (set up depth buffer!)
inline void
OpenGLSetScreenSpace(v2u Dim)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // TODO(chowie): Matrix with rotation?
    // STUDY(chowie): Identity Matrix (similar to glOrtho),
    // fix-function pipeline by column vectors (see placement of -1)
    glMatrixMode(GL_PROJECTION);
    f32 a = SafeRatio1(2.0f, (f32)Dim.Width);
    f32 b = SafeRatio1(2.0f, (f32)Dim.Height);
    f32 Proj[] =
    {
         a,  0,  0,  0,
         0,  b,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1,
    }; // TODO(chowie): Convert "-1, -1,  0,  1," to "0, 0, 0, 1", needs basis points!
    glLoadMatrixf(Proj); // COULDDO: Replace glLoadMatrixf with glLoadTransposedMatrix for row-major matrices. But don't care when get out of fixed function pipeline!
}

// TODO(chowie): Pull this out to Ruinenglass_renderer
// RESOURCE(): https://ktstephano.github.io/rendering/opengl/prog_vtx_pulling
// RESOURCE(): https://voxel.wiki/wiki/vertex-pulling/
// TODO(chowie): Vertex pulling?
// TODO(chowie): Triangle Strip? Turn OpenGL circle too
// TODO(chowie): Support shearing?
// NOTE(chowie): Extra parameters to account for 1-pixel apron
inline void
OpenGLRect(v3 MinP, v3 MaxP, v4 Colour, v2 MinUV = V2(0, 0), v2 MaxUV = V2(1, 1))
{
    glBegin(GL_QUADS);

    glColor4fv(Colour.E);

    // TODO(chowie): Use fv (vector) op! Access with .E
    // NOTE(chowie): Lower triangle
    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex3f(MinP.x, MinP.y, 0.0f);

    glTexCoord2f(MaxUV.x, MinUV.y);
    glVertex3f(MaxP.x, MinP.y, 0.0f);

    glTexCoord2f(MaxUV.x, MaxUV.y);
    glVertex3f(MaxP.x, MaxP.y, 0.0f);

    // NOTE(chowie): Upper triangle
    glTexCoord2f(MinUV.x, MaxUV.y);
    glVertex3f(MinP.x, MaxP.y, 0.0f);

    glEnd();
}

// TODO(chowie): Remove!
inline void
OpenGLBitmap(void *Memory, v2u Dim, s32 Pitch,
             v2u WindowDim, GLuint BlitTexture)
{
    Assert(Pitch == ((s32)Dim.Width*4));
    glViewport(0, 0, Dim.Width, Dim.Height);

    glDisable(GL_SCISSOR_TEST);
    glBindTexture(GL_TEXTURE_2D, BlitTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, Dim.Width, Dim.Height, 0,
                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, Memory);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();


    OpenGLSetScreenSpace(Dim);

    v3 MinP = V3(0, 0, 0);
    v3 MaxP = V3((f32)WindowDim.Width, (f32)WindowDim.Height, 0.0f);
    v4 Colour = V4(1, 1, 1, 0);
    OpenGLRect(MinP, MaxP, Colour);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// RESOURCE(): https://www.humus.name/index.php?page=Comments&ID=228&start=24
// RESOURCE(): https://stackoverflow.com/questions/75496846/circle-triangulation
// Useless in triangulation.

// RESOURCE(aolo2): https://discord.com/channels/239737791225790464/1307441055641374720/1307441055641374720
// NOTE(chowie): Quad with disc sdf vs geometry; geometry wins as they
// blend nicely with a single stroke
// NOTE(chowie): LOD is needed if you have the curse of infinite zoom
// NOTE(chowie): Furthest LOD can just use a single triangle for caps
// TODO(chowie): Bucket into logarithmic increments 3,6,12,24...-gons,
// batch with different LOD. When you're drawing you usually use the
// same-sized brush. If not necessary, batch line segments in one draw call.
// TODO(chowie): For a polyline (multi-segment stroke/path), use
// semi-circle with a rotate (or one point of look-ahead) per instance
// as the caps to look convex. Can be used for joints at the angle
// bisector. 1/3 save
// lod: 0, c =    1, s =    1  ->  0.000
// lod: 1, c =    4, s =    3  ->  0.250
// lod: 2, c =   10, s =    7  ->  0.300
// lod: 3, c =   22, s =   15  ->  0.318
// lod: 4, c =   46, s =   31  ->  0.326
// lod: 5, c =   94, s =   63  ->  0.330
// lod: 6, c =  190, s =  127  ->  0.332
// lod: 7, c =  382, s =  255  ->  0.332
// lod: 8, c =  766, s =  511  ->  0.333
// lod: 9, c = 1534, s = 1023  ->  0.333
// In general, for (t0, e0) initial (triangle, subdividable edge) counts
// P(n) = t0 + e0*(2^n - 1) triangle count for lod=n
// RESOURCE: https://stackoverflow.com/questions/1569939/rendering-different-triangle-types-and-triangle-fans-using-vertex-buffer-objects
// TODO(chowie): Change GL_TRIANGLE_FAN to glDrawArrays
// RESOURCE: https://stackoverflow.com/questions/8762826/texture-mapping-a-circle-made-using-gl-polygon
// TODO(chowie): Proper texturing of circles?
// RESOURCE(blatnik): https://blog.bearcats.nl/seamlessly-subdivide-circle/
// IMPORTANT(chowie): Best explanation by ohAitch. To "triangulate a
// circle" is roughly "get some fractional powers of -1", doing the
// fraction once (outside the loop) and then raising _it_ to succesive
// integer powers. One sin & one cos per/vertex -> now no sin/cos per
// circle, with a v2 orientation and rotation matrix mult cost.
// RESOURCE(inigo quilez): Eerily similar - https://iquilezles.org/articles/sincos/
// RESOURCE(SiegeLord): Eerily similar - https://siegelord.net/circle_draw
inline void
OpenGLCircle(v3 CentreP, f32 Radius, u32 TriCount,
             v4 Colour, f32 Circumference, v2 MinUV = V2(0, 0), v2 MaxUV = V2(1, 1))
{
    m2x2 Rot = M2x2RotationByTris((f32)TriCount, Circumference);
    v2 OrientationP = V2(0, Radius);

    // NOTE(chowie): TriFan vector version default is clockwise currently.
    glBegin(GL_TRIANGLE_FAN);
    glColor4fv(Colour.E);

//    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex2f(CentreP.x, CentreP.y);
    for(u32 TriangleIndex = 0;
        TriangleIndex <= TriCount;
        ++TriangleIndex)
    {
        // TODO(chowie): Why is anticlockwise more imprecise??
        // Anticlockwise +cos +sin <=> (0, -1), Clockwise +cos -sin <=> (0, 1)
        glVertex2f(CentreP.x - OrientationP.x, CentreP.y + OrientationP.y);
        // NOTE(chowie): Orientation after glVertex, starts drawing from top
        OrientationP = Rot*OrientationP;
    }
//    glTexCoord2f(MaxUV.x, MaxUV.y);

    glEnd();
}

// RESOURCE(): https://ktstephano.github.io/rendering/opengl/dsa
// TODO(chowie): Replace bind/unbind with DSA? Assumes OpenGL 4.6
// RESOURCE(): https://ktstephano.github.io/rendering/opengl/ssbos
// TODO(chowie): Shader SSBO?
// RESOURCE(): https://ktstephano.github.io/rendering/opengl/bindless
// TODO(chowie): Bindless textures
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
    for(umm BaseAddress = 0;
        BaseAddress < Commands->CommandsArena.Used;
        )
    {
        render_group_entry_header *Header = (render_group_entry_header *)(Commands->CommandsArena.Base + BaseAddress);
        BaseAddress += sizeof(*Header);

        void *Data = (u8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
            case RenderGroupEntryType_render_entry_clear:
            {
                render_entry_clear *Entry = (render_entry_clear *)Data;

                glClearColor(Entry->Colour.r, Entry->Colour.g, Entry->Colour.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                BaseAddress += sizeof(render_entry_clear);
            } break;

            case RenderGroupEntryType_render_entry_rect:
            {
                render_entry_rect *Entry = (render_entry_rect *)Data;

                // TODO(chowie): What do I do with a v3?
                glDisable(GL_TEXTURE_2D);
                OpenGLRect(Entry->P, Entry->P + V3(Entry->Dim, 0), Entry->Colour);
                glEnable(GL_TEXTURE_2D);

                BaseAddress += sizeof(render_entry_rect);
            } break;

            case RenderGroupEntryType_render_entry_bitmap: // TODO(chowie): Temporary, replace for a generic quad later
            {
                render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
                Assert(Entry->Bitmap)

                glBindTexture(GL_TEXTURE_2D, (GLuint)U32FromPointer(Entry->Bitmap->TextureHandle));

                if(Entry->Bitmap->Dim.E)
                {
                    f32 OneTexelU = 1.0f / (f32)Entry->Bitmap->Dim.Width;
                    f32 OneTexelV = 1.0f / (f32)Entry->Bitmap->Dim.Height;
                    v2 MinUV = V2(OneTexelU, OneTexelV);
                    v2 MaxUV = V2(1.0f - OneTexelU, 1.0f - OneTexelV);
                    v2 BitmapDim = V2((f32)Entry->Bitmap->Dim.x, (f32)Entry->Bitmap->Dim.y);

                    OpenGLRect(Entry->P, Entry->P + V3(BitmapDim, 0), Entry->PremulColour, MinUV, MaxUV);
                }
            };

            case RenderGroupEntryType_render_entry_circle:
            {
                render_entry_circle *Entry = (render_entry_circle *)Data;

                glDisable(GL_TEXTURE_2D);
                OpenGLCircle(Entry->P, Entry->Radius, Entry->Tris, Entry->Colour, Entry->Circumference);
                glEnable(GL_TEXTURE_2D);

                BaseAddress += sizeof(render_entry_circle);
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
        f32 OneTexelU = 1.0f / (f32)Entry->Bitmap->Width;
        f32 OneTexelV = 1.0f / (f32)Entry->Bitmap->Height;
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
