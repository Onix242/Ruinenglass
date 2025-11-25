/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

inline void
PushBitmapInternal(render_group *RenderGroup, rui_bitmap *Bitmap,
           f32 Height, v3 Offset, v4 Colour = V4(1, 1, 1, 1))
{
    if(Bitmap->Dim.E)
    {
    }
}

inline void
PushBitmap(render_group *RenderGroup, bitmap_id ID,
           f32 Height, v3 Offset, v4 Colour = V4(1, 1, 1, 1))
{
    rui_bitmap *Bitmap = GetBitmap(RenderGroup->GameAssets, ID);
    if(Bitmap)
    {
        PushBitmapInternal(RenderGroup, Bitmap, Height, Offset, Colour);
    }
    else
    {
        LoadBitmap(RenderGroup->GameAssets, ID);
    }
}

inline void
PushCube(render_group *RenderGroup, bitmap_id ID,
         v3 P, f32 Radius, v3 Dim, v4 Colour)
{
    rui_bitmap *Bitmap = GetBitmap(RenderGroup->GameAssets, ID);
    if(Bitmap)
    {
    }
    else
    {
        // NOTE(chowie): Threads to force assets to around next time
    }
}

