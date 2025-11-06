#if !defined(RUINENGLASS_UI_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Get inspiration from when I did a simple compressor a program
enum rect_cut_side_type
{
    Side_Left,
    Side_Right,
    Side_Top,
    Side_Bottom,
};

struct rect_cut
{
    rect2 *Rect;
    rect_cut_side_type Type;
};

// TODO(chowie): Fully implement this!
// COULDDO(chowie): Discriminated unions?
struct ui_layout_element
{
    rect_cut_side_type Type;
};

#define RUINENGLASS_UI_H
#endif
