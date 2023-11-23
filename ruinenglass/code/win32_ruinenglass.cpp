/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Talia ... & Michael Chow $
   $Notice: $
   ======================================================================== */

// NOTE(chowie): Win32/Compiler defines. Must be above windows.h!!!!
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN

#include "ruinenglass_platform.h"

#include <windows.h>
#include <xinput.h>
#include "ruinenglass_intrinsics.h"
#include "win32_ruinenglass.h"

// TODO(chowie): These are global for now!
global_variable b32 GlobalRunning;
global_variable b32 GlobalShowCursor;
global_variable win32_offscreen_buffer GlobalBackbuffer;

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        OutputDebugStringA("Xinput9_1_0 loaded\n");
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

    if(!XInputLibrary)
    {
        OutputDebugStringA("Xinput1_3 loaded\n");
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if(XInputLibrary)
    {
        // TODO(chowie): Logging/Diagnostic
//        OutputDebugStringA("Xinput1_4 loaded successfully\n");

        // RESOURCE(chowie): https://hero.handmade.network/forums/code-discussion/t/2686-function_pointer_assignment_trick#13360
        // RESOURCE(chowie): https://man7.org/linux/man-pages/man3/dlopen.3.html
        // TODO(chowie): A big advantage is assigning to a struct or
        // an array of function pointers, since you can use a loop.
        *(void **)(&XInputGetState) = GetProcAddress(XInputLibrary, "XInputGetState");
        *(void **)(&XInputSetState) = GetProcAddress(XInputLibrary, "XInputSetState");

//        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    }
    else
    {
        OutputDebugStringA("Xinput1_4 failed to load\n");
    }
}

// TODO(chowie): See what optimiser does!
internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer,
                    v2 Offset)
{
    // STUDY(chowie): Pixels are 32-bit wide, little-endian
    // architecture.
    // Memory Order:   BB GG RR xx
    // Register Order: xx RR GG BB

    u8 *Row = (u8 *)Buffer->Memory;
    for(s32 Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = 0;
            X < Buffer->Width;
            ++X)
        {
            u8 Red = 0;
            u8 Blue = (u8)(X + Offset.x);
            u8 Green = (u8)(Y + Offset.y);

            // STUDY(chowie): Remember pointer arithmetic advances by
            // 4-bytes, an entire u32!
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
    
        Row += Buffer->Pitch;
    }
}

struct win32_window_dim
{
    s32 Width;
    s32 Height;
};
internal win32_window_dim
Win32GetWindowDim(HWND Window)
{
    win32_window_dim Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(chowie): Bulletproof this.
    // Free first, free after, then free first if that fails?

    // NOTE(chowie): Allocating new memory everything for a new buffer
    // is not ideal.
    // Either allocate before creating the a new DIB section, free old
    // DIB. Alternatively, wait and see if we can get the new one first
    // else keep using the old one.

    // NOTE(chowie): Creating first and freeing after means if we
    // needed the memory from the first one in order to get the memory
    // from the second one since it was occupying too much memory.

#if 1
    // TODO(chowie): This is techincally only called once now! REMOVE!
    if(Buffer->Memory)
    {
        // STUDY(chowie): If this was MEM_DECOMMIT. Use VirtualProtect
        // with PAGE_NOACCESS to ensure no one keeps a stale pointer
        // to the page. A "use after free" bug if it was written to!
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
#endif

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = BITMAP_BYTES_PER_PIXEL;

    // NOTE(chowie): When biHeight is negative, Windows treats this
    // bitmap as top-down (not bottom-up), meaning the first 3-bytes
    // of the image are the colour of the top-left pixel, not the
    // bottom-left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // TODO(chowie): Buckle top-down bitmaps down!
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = BITMAP_BYTES_PER_PIXEL*8; // NOTE: 8-bits per 1-byte of colour
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    /*
      RESOURCE(chowie): https://learn.microsoft.com/en-us/windows/win32/medfound/image-stride

      STUDY(chowie): Storing a 2D image in a 1D "block" of memory

      Pitch/Stride = Value to added to a pointer on the first row to
      move it to the next row; byte offset between rows.
    */

    Buffer->Pitch = (Buffer->Width*Buffer->BytesPerPixel);
    // NOTE(chowie): There is not need to clear to black as
    // VirtualAlloc should _only_ output zero'd memory!
    int BitmapMemorySize = (Buffer->Pitch*Buffer->Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
                           int WindowWidth, int WindowHeight)
{
    // TODO(chowie): Aspect ratio correction
    // TODO(chowie): Stretch modes?
    // STUDY(chowie): Palettised colours idea for texture compression?
    // STUDY(chowie): Backbuffer and window should be the same size for now
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ProcessPendingMessages(void)
{
    MSG Message;
    while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VKCode = (u32)Message.wParam;
                // RESOURCE(chowie): https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
                // STUDY(chowie): Instead of returning either
                // (1 << 30) or 0. "!= 0" forces 1 or 0.
#define KeyMessageWasDownBit (1 << 30)
#define KeyMessageWasUpBit (1 << 31)
                b32 WasDown = ((Message.lParam & KeyMessageWasDownBit) != 0);
                b32 IsDown = ((Message.lParam & KeyMessageWasUpBit) == 0);

                // NOTE(chowie): Holding down a key would otherwise
                // display both WasDown and IsDown messages!
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
//                    OutputDebugStringA("W\n");
                    }
                    else if(VKCode == 'A')
                    {
                    }
                    else if(VKCode == 'S')
                    {
                    }
                    else if(VKCode == 'D')
                    {
                    }
                    else if(VKCode == 'Q')
                    {
                    }
                    else if(VKCode == 'E')
                    {
                    }
                    else if(VKCode == VK_UP)
                    {
                    }
                    else if(VKCode == VK_DOWN)
                    {
                    }
                    else if(VKCode == VK_LEFT)
                    {
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        // TODO(chowie): This is for testing only REMOVE!
                        OutputDebugStringA("ESCAPE: ");
                        if(IsDown)
                        {
                            OutputDebugStringA("IsDown ");
                        }
                        if(WasDown)
                        {
                            OutputDebugStringA("WasDown");
                        }
                        OutputDebugStringA("\n");
                    }
                    else if(VKCode == VK_SPACE)
                    {
                    }
                }

                if(IsDown)
                {
#define AltKeyWasDownBit (1 << 29)
                    b32 AltKeyWasDown = (Message.lParam & AltKeyWasDownBit);
                    if((VKCode == VK_F4) && AltKeyWasDown)
                    {
                        GlobalRunning = false;
                    }
                }

            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

// STUDY(chowie): WParam and LParam wants to call us in a single
// function, but it has messages with different params of different
// types. Encumbant on user to cast the values. Anonymous parameters
// takes on whatever meaning it needs to for the message in question.
LRESULT CALLBACK
Win32MainWindowCallback(HWND   Window,
                        UINT   Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    // STUDY(chowie): Windows needs 64-bits to handle the biggest
    // message as a return pointer; most messages don't require them.
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_CLOSE:
        {
            // TODO(chowie): Handle with an 'Are you sure?' message to
            // the user?
            GlobalRunning = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_DESTROY:
        {
            // TODO(chowie): Handle an error - recreate window?
            GlobalRunning = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard came in a non-dispatched message!");
        } break;

        case WM_SETCURSOR:
        {
            if(GlobalShowCursor)
            {
                Result = DefWindowProcA(Window, Message, WParam, LParam);
            }
            else
            {
                SetCursor(0);
            }
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dim Dim = Win32GetWindowDim(Window);
            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dim.Width, Dim.Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            // NOTE(chowie): Handles Windows messages that is
            // mandatory e.g. WM_Paint cannot be half-filled!
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

// NOTE(chowie): wWinMain is the newer version and is not necessary!
int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR     CommandLine,
        int       ShowCode)
{
    WNDCLASSA WindowClass = {};

    // NOTE(chowie): Fixed-sized backbuffer
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW; // NOTE(chowie): Repaint the whole window if resizing window.
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
//    WindowClass.hIcon = ;
//    WindowClass.hbrBackground = ;
    WindowClass.lpszClassName = "RuinenglassWindowClass";

    Win32LoadXInput();

#if RUINENGLASS_INTERNAL
    GlobalShowCursor = true;
#endif

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Ruinenglass",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE, // NOTE(chowie): WS_VISIBLE can be ShowWindow!
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0);
        if(Window)
        {
            GlobalRunning = true;
            v2 Offset = {};
            // NOTE(chowie): Messages don't always field in the
            // dispatch queue, Windows reserves the rights to
            // 'cold-call' sometimes (in Win32MainWindowCallback).
            while(GlobalRunning)
            {
                Win32ProcessPendingMessages();

                // TODO(chowie): Should we poll this more frequently?
                DWORD MaxControllerCount = XUSER_MAX_COUNT;
                for(DWORD ControllerIndex = 0;
                    ControllerIndex < MaxControllerCount;
                    ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // NOTE(chowie): The controller is plugged in
                        // TODO(chowie): See if ControllerState.dwPacketNumber increments too rapidly
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad; // STUDY(chowie): Syntactic convience by snapping the pointer

                        b32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        b32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        b32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        b32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        b32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        b32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        b32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        b32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        b32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        b32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        b32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        b32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        s16 StickX = Pad->sThumbLX;
                        s16 StickY = Pad->sThumbLY;

                        Offset.x += (StickX >> 12);
                        Offset.y += (StickY >> 12);
                    }
                    else
                    {
                        // NOTE(chowie): The controller is unavailable
                    }
                }

#if 0
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 6000;
                Vibration.wRightMotorSpeed = 6000;
                XInputSetState(0, &Vibration);
#endif

                RenderWeirdGradient(&GlobalBackbuffer, Offset);

                HDC DeviceContext = GetDC(Window);
                win32_window_dim Dim = Win32GetWindowDim(Window);
                Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                           Dim.Width, Dim.Height);
                ReleaseDC(Window, DeviceContext);

                ++Offset.x;
            }
        }
        else
        {
            // TODO(chowie): Logging
            OutputDebugStringA("Invalid window created\n");
        }
    }
    else
    {
        OutputDebugStringA("Failed to create window\n");
    }

    // STUDY(chowie): RAII isn't best for performance, things are best
    // when 'Acquired and Released in Aggregate'; Part of a group,
    // handled together in waves. On exit, Windows bulk cleans
    // windows, memory, and handles.

    return(0);
}
