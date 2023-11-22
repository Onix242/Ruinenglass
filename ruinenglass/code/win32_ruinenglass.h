#if !defined(WIN32_RUINENGLASS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Talia ... & Michael Chow $
   $Notice: $
   ======================================================================== */

/*
  RESOURCE(chowie): https://www.reddit.com/r/learnprogramming/comments/12q5bho/can_somebody_explain_back_buffers_and_how_they/

  NOTE(chowie): Reserve an extra memory region of memory representing
  the "offscreen" image. If the code was representing the same region,
  there could be strange artifacts to having the screen partially
  drawn. Graphics are typically double-buffered, perform all drawing
  code against the offscreen buffer. After, tell the hardware or
  software stack to switch buffers. Offscreen -> Onscreen, and vice
  versa. You next draw becomes the other buffer, repeat.
*/
struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
    s32 BytesPerPixel;
};

/*
  RESOURCE(chowie): https://d3s.mff.cuni.cz/legacy/~holub/c_features.html
  STUDY(chowie): Defining an array at the end of a struct e.g.
  BITMAPINFO, allows variable-length arrays accessor-wise. Don't
  recommend.
*/

// NOTE(chowie): XInputGetState
// STUDY(chowie): This avoids execution violations by declaring a
// pointer to function with that signature
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(chowie): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define WIN32_RUINENGLASS_H
#endif