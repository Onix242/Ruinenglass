/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// NOTE(chowie): Win32/Compiler defines. Must be above windows.h!!!!
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#define NOMINMAX

#include "ruinenglass_platform.h"

#include <windows.h>
#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "ruinenglass_intrinsics.h"
#include "win32_ruinenglass.h"

// TODO(chowie): These are global for now!
global_variable b32 GlobalRunning;
global_variable b32 GlobalShowCursor;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable IAudioClient *GlobalSoundClient;
global_variable IAudioRenderClient *GlobalSoundRenderClient;

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
        // RESOURCE(chowie): https://hero.handmade.network/forums/code-discussion/t/2686-function_pointer_assignment_trick#13360
        // RESOURCE(chowie): https://man7.org/linux/man-pages/man3/dlopen.3.html
        // TODO(chowie): A big advantage is assigning to a struct or
        // an array of function pointers, since you can use a loop.
        *(void **)(&XInputGetState) = GetProcAddress(XInputLibrary, "XInputGetState");
        *(void **)(&XInputSetState) = GetProcAddress(XInputLibrary, "XInputSetState");
        // STUDY(chowie): Compare "XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");"

        // TODO(chowie): Logging/Diagnostic
//        OutputDebugStringA("Xinput1_4 loaded successfully\n");
    }
    else
    {
        OutputDebugStringA("Xinput1_4 failed to load\n");
    }
}

internal void
Win32LoadWASAPI(void)
{
    HMODULE WASAPILibrary = LoadLibraryA("ole32.dll");

    if(WASAPILibrary)
    {
#if 1
        *(void **)(&CoInitializeEx) = GetProcAddress(WASAPILibrary, "CoInitializeEx");
        *(void **)(&CoCreateInstance) = GetProcAddress(WASAPILibrary, "CoCreateInstance");
#else
        for(u32 LibraryIndex = 0;
            LibraryIndex < ArrayCount(GlobalAudioContext);
            ++LibraryIndex)
        {
            *(void **)(&GlobalAudioContext[LibraryIndex]) = GetProcAddress(WASAPILibrary, "CoInitializeEx");
        }
#endif
        // TODO(chowie): Logging/Diagnostic
    }
    else
    {
        OutputDebugStringA("Ole32 failed to load\n");
    }
}

// TODO(chowie): This is really not a good way to use WASAPI, make a thread-queue first before multithreading this!
// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/8433-correct_implementation_of_wasapi
// RESOURCE: By Nickav, https://gist.github.com/nickav/8be2ded8a8363d5993b2f4e5aa601bd3
// NOTE(chowie): Thank you Martins for providing introductory code!
// RESOURCE: https://gist.github.com/mmozeiko/38c64bb65855d783645c
internal void
Win32InitWASAPI(s32 SamplesPerSecond, s32 BufferSizeInSamples)
{
    // STUDY(chowie): WASAPI is COM! Stepping over the func looks though a jump table
    // RESOURCE: https://kodi.wiki/view/Windows_audio_APIs

    // TODO(chowie): Abstract the audio api architecture?
    // TODO: Output HRESULT to be able to be inspected in watch window!

    // TODO(chowie): Test that FAILED = !SUCCEEDED
    if(FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY)))
    {
        Assert(!"Error");
    }

    IMMDeviceEnumerator *Enumerator;
    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, IID_PPV_ARGS(&Enumerator))))
    {
        Assert(!"Error");
    }

    IMMDevice *Device;
    if(FAILED(Enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device)))
    {
        Assert(!"Error");
    }

    if(FAILED(Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (LPVOID*)&GlobalSoundClient)))
    {
        Assert(!"Error");
    }

    // NOTE(chowie): I reckon this duplicates the two sound buffers [LEFT RIGHT], compared to DSound.
    WAVEFORMATEXTENSIBLE WaveFormat = {};

    WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    WaveFormat.Format.nChannels = 2; // TODO(chowie): Explore multi-channel audio?
    WaveFormat.Format.nSamplesPerSec = (DWORD)SamplesPerSecond;
    WaveFormat.Format.wBitsPerSample = 16; // NOTE(chowie): Really means how many bits per half a sample
    WaveFormat.Format.nBlockAlign = (WORD)((WaveFormat.Format.nChannels * WaveFormat.Format.wBitsPerSample) / 8); // NOTE(chowie): Interleaved bytes [LEFT RIGHT], "s16 for each channel (2) / 8 (in bytes)", just equals 4!
    WaveFormat.Format.nAvgBytesPerSec = WaveFormat.Format.nSamplesPerSec * WaveFormat.Format.nBlockAlign;
    WaveFormat.Format.cbSize = sizeof(WaveFormat);
    WaveFormat.Samples.wValidBitsPerSample = 16;
    WaveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    REFERENCE_TIME BufferDuration = 10000000ULL * BufferSizeInSamples / SamplesPerSecond; // NOTE(martins): Buffer size in 100 nanoseconds
    if(FAILED(GlobalSoundClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST, BufferDuration, 0, &WaveFormat.Format, 0)))
    {
        Assert(!"Error");
    }

    if(FAILED(GlobalSoundClient->GetService(IID_PPV_ARGS(&GlobalSoundRenderClient))))
    {
        Assert(!"Error");
    }

    UINT32 SoundFrameCount;
    if(FAILED(GlobalSoundClient->GetBufferSize(&SoundFrameCount)))
    {
        Assert(!"Error");
    }

    // NOTE(martins): Check if we got what we requested (better would to pass this value back as real buffer size)
    Assert(BufferSizeInSamples <= (s32)SoundFrameCount);
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, u32 SamplesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    BYTE *SoundBufferData;
    if(SUCCEEDED(GlobalSoundRenderClient->GetBuffer((UINT32)SamplesToWrite, &SoundBufferData)))
    {
        s16 *SourceSample = SourceBuffer->Samples;
        s16 *DestSample = (s16 *)SoundBufferData;
        for(u32 SampleIndex = 0;
            SampleIndex < SamplesToWrite;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex; // TODO(chowie): Record with this!
        }

        GlobalSoundRenderClient->ReleaseBuffer((UINT32)SamplesToWrite, 0);
    }
}

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/1380-day_20_tiny_bit_of_fun_with_the_code
struct wilwa_dial_tone
{
    char *Tag; // TODO(chowie): Display this?

    s32 Period;
    s32 ToneHz; // NOTE(chowie): For testing 200-500hz is a good range!
    r32 Value; // NOTE(chowie): Sine value
};
struct test_output_sound
{
    s16 ToneVolume;
    FIELD_ARRAY(wilwa_dial_tone,
    {
        wilwa_dial_tone Wave1;
        wilwa_dial_tone Wave2;
    });
};
internal void
TestOutputWilwaDialTone(game_sound_output_buffer *SoundBuffer)
{
    local_persist r32 tSine1; // TODO(chowie): Move this out to game_state!
    local_persist r32 tSine2;

    test_output_sound WilwaDialTone = {};
    WilwaDialTone.ToneVolume = 3000;

    wilwa_dial_tone *Wave1 = &WilwaDialTone.Wave1;
    Wave1->Tag = "Wave 1";
    Wave1->ToneHz = 350;
    Wave1->Period = SoundBuffer->SamplesPerSecond / Wave1->ToneHz;
    wilwa_dial_tone *Wave2 = &WilwaDialTone.Wave2;
    Wave2->Tag = "Wave 2";
    Wave2->ToneHz = 440;
    Wave2->Period = SoundBuffer->SamplesPerSecond / Wave2->ToneHz;

    s16 *SampleOut = SoundBuffer->Samples;
    for(s32 SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        Wave1->Value = Sin(tSine1);
        Wave2->Value = Sin(tSine2);

        r32 TotalValue = 0;
        for(s32 ToneIndex = 0;
            ToneIndex < ArrayCount(WilwaDialTone.E); // STUDY(chowie): I'm really proud to be able to convert a range-based for loop into a regular one!
            ++ToneIndex)
        {
            wilwa_dial_tone *Sound = WilwaDialTone.E + ToneIndex;
            TotalValue += Sound->Value;
        }
        s16 SampleValue = (s16)(TotalValue * WilwaDialTone.ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine1 += Tau32*1.0f / (r32)Wave1->Period; // NOTE(chowie): 2*Pi is how many wave periods elapsed since we started
        if(tSine1 > Tau32)
        {
            tSine1 -= Tau32; // NOTE(chowie): Normalising it to its period
        }
        tSine2 += Tau32*1.0f / (r32)Wave2->Period;
        if(tSine2 > Tau32)
        {
            tSine2 -= Tau32;
        }
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
    // TODO(chowie): This is technically only called once now! REMOVE!
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
      RESOURCE: https://learn.microsoft.com/en-us/windows/win32/medfound/image-stride

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
                            OutputDebugStringA("IsDown");
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
    Win32LoadWASAPI();

#if RUINENGLASS_INTERNAL
    GlobalShowCursor = true;
#endif

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(0, WindowClass.lpszClassName, "Ruinenglass",
                            WS_OVERLAPPEDWINDOW|WS_VISIBLE, // NOTE(chowie): WS_VISIBLE can be ShowWindow!
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            0, 0, Instance, 0);
        if(Window)
        {
            v2 Offset = {};

            // STUDY(chowie): Audio latency is determined not by the size of
            // the buffer, but by how far ahead of the PlayCursor you
            // write. The optimal amount of latency is the amount that will
            // cause this frame's audio to coincide with the display of this
            // frame's image.
            int FramesOfAudioLatency = 1;
            int MonitorRefreshHz = 60;
            HDC RefreshDC = GetDC(Window);
            int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            ReleaseDC(Window, RefreshDC);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            r32 GameUpdateHz = (r32)(MonitorRefreshHz / 2.0f);
            r32 TargetSecondsPerFrame = 1.0f / (r32)GameUpdateHz;

            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000; // TODO(chowie): Set to 60 seconds?
            SoundOutput.BytesPerSample = sizeof(s16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample; // TODO(chowie): Allocate a 2 sec buffer?
            SoundOutput.LatencySampleCount = FramesOfAudioLatency*(int)(SoundOutput.SamplesPerSecond / GameUpdateHz);
            Win32InitWASAPI(SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);

            GlobalRunning = true;

            // STUDY(chowie): Remember that modulo op transforms an absolute size to relative!
            // TODO(chowie): Pool with bitmap VirtualAlloc
            s16 *Samples = (s16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            if(Samples)
            {
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

                    // RESOURCE: https://hero.handmade.network/forums/code-discussion/t/102-day_19_-_audio_latency
                    // NOTE(chowie): Computes how much sound to write and where.
#if 1
                    // NOTE(chowie): For calculating samples to write,
                    // this is more accurate. We want x samples (1
                    // frames worth + latency).
                    int SamplesToWrite = 0;
                    UINT32 SoundPaddingSize;
                    if(SUCCEEDED(GlobalSoundClient->GetCurrentPadding(&SoundPaddingSize)))
                    {
                        // NOTE(chowie): Number of samples that are
                        // ready and have not been read into the
                        // buffer. Therefore, number of samples to
                        // write should be x - padding.
                        int MaxSampleCount = (int)(SoundOutput.SecondaryBufferSize - SoundPaddingSize);
                        SamplesToWrite = (int)(SoundOutput.LatencySampleCount - SoundPaddingSize);
                        if(SamplesToWrite < 0)
                        {
                            SamplesToWrite = 0;
                        }
                        Assert(SamplesToWrite <= MaxSampleCount);
                    }
#else
                    int SamplesToWrite = 0;
                    UINT32 SoundPaddingSize;
                    if (SUCCEEDED(GlobalSoundClient->GetCurrentPadding(&SoundPaddingSize)))
                    {
                        SamplesToWrite = (int)(SoundOutput.SecondaryBufferSize - SoundPaddingSize);
                        if (SamplesToWrite > SoundOutput.LatencySampleCount)
                        {
                            SamplesToWrite = SoundOutput.LatencySampleCount;
                        }
                    }
#endif
                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                    SoundBuffer.SampleCount = SamplesToWrite;
                    SoundBuffer.Samples = Samples;

                    TestOutputWilwaDialTone(&SoundBuffer); // TODO(chowie): Move this out!

                    Win32FillSoundBuffer(&SoundOutput, SamplesToWrite, &SoundBuffer);
                    GlobalSoundClient->Start();
                    // NOTE(chowie): Starts buffer the first time we
                    // fill data in it, rather than filling the audio
                    // buffer at init as it would introduce latency
                    // (we cannot overwrite previous samples). Call
                    // after audio engine has been initially loaded.

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
