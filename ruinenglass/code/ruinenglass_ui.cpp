/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// RESOURCE(): https://halt.software/p/rectcut-for-dead-simple-ui-layouts
// RESOURCE(): Linked in article above - https://github.com/nsmryan/rectcut-rs/blob/master/src/lib.rs
// TODO(chowie): Use Inverted infinity rect in math.h?

// From the rect cut article,
// Example 1: Toolbar (buttons on left and right)
// Rect layout = { 0, 0, 180, 16 };
//
// Rect r1 = cut_left(&layout, 16);
// Rect r2 = cut_left(&layout, 16);
// Rect r3 = cut_left(&layout, 16);
//
// Rect r4 = cut_right(&layout, 16);
// Rect r5 = cut_right(&layout, 16);

// Example 2
// - Top bar with icons and title
// Rect top = cut_top(&layout, 16);
//     Rect button_close = cut_right(&top, 16);
//     Rect button_maximize = cut_right(&top, 16);
//     Rect button_minimize = cut_right(&top, 16);
//     Rect title = top;
//
// - Bottom bar.
// Rect bottom = cut_bottom(&layout, 16);
//
// - Left and right panels.
// Rect panel_left = cut_left(&layout, w / 2);
// Rect panel_right = layout;

// NOTE(chowie): Cuts smaller rectangle of input rect
inline rect2
RectCutSide_Left(rect2 *Rect, f32 A)
{
    f32 CutLeft = Rect->Min.x;
    Rect->Min.x = Min(Rect->Max.x, Rect->Min.x + A);

    rect2 Result = RectMinMax(V2(CutLeft, Rect->Min.y), V2(Rect->Min.x, Rect->Max.y));
    return(Result);
}

inline rect2
RectCutSide_Right(rect2 *Rect, f32 A)
{
    f32 CutRight = Rect->Max.x;
    Rect->Max.x = Max(Rect->Min.x, Rect->Max.x - A);

    rect2 Result = RectMinMax(V2(Rect->Max.x, Rect->Min.y), V2(CutRight, Rect->Max.y));
    return(Result);
}

inline rect2
RectCutSide_Top(rect2 *Rect, f32 A)
{
    f32 CutTop = Rect->Min.y;
    Rect->Min.y = Min(Rect->Max.y, Rect->Min.y + A);

    rect2 Result = RectMinMax(V2(Rect->Min.x, CutTop), V2(Rect->Max.x, Rect->Min.y));
    return(Result);
}

inline rect2
RectCutSide_Bottom(rect2 *Rect, f32 A)
{
    f32 CutBottom = Rect->Max.y;
    Rect->Max.y = Max(Rect->Min.y, Rect->Max.y - A);

    rect2 Result = RectMinMax(V2(Rect->Min.x, Rect->Max.y), V2(Rect->Max.x, CutBottom));
    return(Result);
}

inline rect_cut
GetRectCut(rect2 *Rect, rect_cut_side_type Type)
{
    rect_cut Result = {Rect, Type};
    return(Result);
}

inline rect2
RectCutFitToSize(rect_cut *RectCut, f32 A)
{
#define RectCase(Cut, a) case a: {return RectCut##a(Cut, a);}
    switch(RectCut->Type)
    {
        RectCase(RectCut->Rect, Side_Left);
        RectCase(RectCut->Rect, Side_Right);
        RectCase(RectCut->Rect, Side_Top);
        RectCase(RectCut->Rect, Side_Bottom);
        default: InvalidCodePath;
    }
}

// TODO(chowie): Button's size is by the label
// b32x Button(rect_cut Layout, char *Label)
// {
//    f32 Size = MeasureText(Label)
//    rect2 Rect = RectCutFitToSize(Layout, Size)
//    - Interactions
//    - Draw
// }
//
// From caller,
// rect2 Toolbar = {, , , ,};
// Button(GetRectCut(&Toolbar, Side_Left), "Left");
// Button(GetRectCut(&Toolbar, Side_Right), "Right");

// NOTE(chowie): Leaves rect unmodified = good for adding 9-patch UI or decorations?
inline rect2
GetRectCutSide_Left(rect2 *Rect, f32 A)
{
    f32 Left = Min(Rect->Max.x, Rect->Min.x + A);

    rect2 Result = RectMinMax(V2(Rect->Min.x, Rect->Min.y), V2(Left, Rect->Max.y));
    return(Result);
}

inline rect2
GetRectCutSide_Right(rect2 *Rect, f32 A)
{
    f32 Right = Max(Rect->Min.x, Rect->Max.x - A);

    rect2 Result = RectMinMax(V2(Right, Rect->Min.y), V2(Rect->Max.x, Rect->Max.y));
    return(Result);
}

inline rect2
GetRectCutSide_Top(rect2 *Rect, f32 A)
{
    f32 Top = Min(Rect->Max.y, Rect->Min.y + A);

    rect2 Result = RectMinMax(V2(Rect->Min.x, Rect->Min.y), V2(Rect->Max.x, Top));
    return(Result);
}

inline rect2
GetRectCutSide_Bottom(rect2 *Rect, f32 A)
{
    f32 Bottom = Max(Rect->Min.y, Rect->Max.y - A);

    rect2 Result = RectMinMax(V2(Rect->Min.x, Bottom), V2(Rect->Max.x, Rect->Max.y));
    return(Result);
}

// NOTE(chowie): Useful for adding tooltips or other overlay elements!
inline rect2
AddRectCutSide_Left(rect2 *Rect, f32 A)
{
    f32 AddLeft = Rect->Min.x - A;
    rect2 Result = RectMinMax(V2(AddLeft, Rect->Min.y), V2(Rect->Min.x, Rect->Max.y));
    return(Result);
}

inline rect2
AddRectCutSide_Right(rect2 *Rect, f32 A)
{
    f32 AddRight = Rect->Max.x + A;
    rect2 Result = RectMinMax(V2(Rect->Max.x, Rect->Min.y), V2(AddRight, Rect->Max.y));
    return(Result);
}

inline rect2
AddRectCutSide_Top(rect2 *Rect, f32 A)
{
    f32 AddTop = Rect->Min.y - A;
    rect2 Result = RectMinMax(V2(Rect->Min.x, AddTop), V2(Rect->Max.x, Rect->Min.y));
    return(Result);
}

inline rect2
AddRectCutSide_Bottom(rect2 *Rect, f32 A)
{
    f32 AddBottom = Rect->Max.y + A;
    rect2 Result = RectMinMax(V2(Rect->Min.x, Rect->Max.y), V2(Rect->Max.x, AddBottom));
    return(Result);
}

inline rect2
RectCutExtendBorder(rect2 *Rect, v2 Radius)
{
    rect2 Result = AddRadiusTo(*Rect, Radius);
    return(Result);
}

inline rect2
RectCutContractBorder(rect2 *Rect, v2 Radius)
{
    rect2 Result = SubtractRadiusTo(*Rect, Radius);
    return(Result);
}

