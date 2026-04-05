#if !defined(RUINENGLASS_UI_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Get inspiration from when I did a simple compressor a program
enum rectcutside_type
{
    Side_Left,
    Side_Right,
    Side_Top,
    Side_Bottom,
};

struct rectcut
{
    rect2 *Rect;
    rectcutside_type Type;
};

// TODO(chowie): Fully implement this!
// COULDDO(chowie): Discriminated unions?
struct layout_element
{
    rectcutside_type Type;
};

// TODO(chowie): Alignment modes?

#define RUINENGLASS_UI_H
#endif
