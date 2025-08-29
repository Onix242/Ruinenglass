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
#define COBJMACROS
#define NOMINMAX

#include "ruinenglass_platform.h"
#include "ruinenglass_shared.h"

// IMPORTANT(chowie): I want to make sure #include <stdio.h> is gone for sn_printf
#include <windows.h>
#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <gl/gl.h>

#include "win32_ruinenglass.h"

// TODO(chowie): These are global for now!
global platform_api Platform; // TODO(chowie): Remove? For moving code between renderer and win32
global b32x GlobalRunning;
global b32x GlobalPause;
global b32x GlobalShowCursor;
global win32_offscreen_buffer GlobalBackbuffer;

global IAudioClient *GlobalSoundClient;
global IAudioRenderClient *GlobalSoundRenderClient;
global WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
global u64 GlobalPerfCountFrequency; // TODO(chowie): Time with this?

global GLuint OpenGLDefaultInternalTextureFormat;

#include "ruinenglass_renderer.cpp"
#include "ruinenglass_renderer_opengl.cpp"
#include "win32_ruinenglass_renderer_opengl.cpp"

// NOTE(chowie): This is the only round trip allowed atm.
// STUDY(chowie): Alternatively, Allocating/reserve and Read memory
// could be platform dynamically, but would like know how much memory
// can be used.
// TODO(chowie): Proper File I/O reading all files of the same
// type. Writing to a queue to be processed async.
// TODO(chowie): Memory mapped files for performance (on the backing
// store) reads/writes?
#if RUINENGLASS_INTERNAL

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

// NOTE(chowie): Just for testing. You don't want to do lots of
// small Virtuallocs, typically only for 64K - 4k pages. If you cared,
// you'd use HeapAlloc.
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateU64(FileSize.QuadPart); // NOTE(chowie): Not interested in loading large files for debug
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                // TODO(chowie): Explore memmapfile?
                // TODO(chowie): Overlapped I/O, or I/O ports?
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && // STUDY(chowie): Short circuit FileSize checks
                   (FileSize32 == BytesRead)) // NOTE(chowie): Guard truncated in-mid reads. Can specify larger values in reads, and would get less bytes.
                {
                    // NOTE(chowie): File read successfully!
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    // TODO(chowie): Logging!
                    // NOTE(chowie): Same as an allocation failure,
                    // otherwise would return garbage memory.
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0; // NOTE(chowie): Remove garbage
                }
            }
            else
            {
                // TODO(chowie): Logging!
            }
        }
        else
        {
            // TODO(chowie): Logging!
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(chowie): Logging!
    }

    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    b32x Result = false;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(chowie): File read successfully
            Result = (BytesWritten == MemorySize); // STUDY(chowie): Relational operators
        }
        else
        {
            // TODO(chowie): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(chowie): Logging
    }

    return(Result);
}

#endif

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return(Result);
}

PLATFORM_DEALLOCATE_MEMORY(Win32DeallocateMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

// TODO(chowie): Fix this with OpenGL!
internal void
Win32PreventDPIScaling(void)
{
    HMODULE WinUserLibrary = LoadLibraryA("user32.dll"); // TODO(chowie): Can I not load user32.dll twice?
    *(void **)(&SetProcessDpiAwarenessContext) = GetProcAddress(WinUserLibrary, "SetProcessDPIAwarenessContext");
    if(SetProcessDpiAwarenessContext)
    {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        *(void **)(&SetProcessDpiAware) = GetProcAddress(WinUserLibrary, "SetProcessDPIAware");
        if(SetProcessDpiAware)
        {
            SetProcessDpiAware();
        }
    }
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    DWORD SizeOfFileName = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          umm DestCount, char *Dest)
{
    // TODO(chowie): d7sam concat? Needs to be a range though.
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

inline FILETIME
Win32GetLastWriteTime(char *FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(FileName, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

internal void
Win32UnloadCode(win32_loaded_code *Loaded)
{
    if(Loaded->DLL)
    {
        // TODO(chowie): Should I never free lib, we may still be
        // pointing to strings that are still inside?
        FreeLibrary(Loaded->DLL);
        Loaded->DLL = 0;
    }

    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
}

internal void
Win32LoadCode(win32_loaded_code *Loaded)
{
    char *SourceDLLName = Loaded->DLLFullPath;
    char *TempDLLName = Loaded->TempFullPath;
    char *LockFileName = Loaded->LockFullPath;

    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        // NOTE(chowie): Allows locking a file not the file the compiler is outputting to!
        Loaded->DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

        CopyFile(SourceDLLName, TempDLLName, FALSE);

        Loaded->DLL = LoadLibraryA(TempDLLName);
        if(Loaded->DLL)
        {
            Loaded->IsValid = true;
            for(u32 FunctionIndex = 0;
                FunctionIndex < Loaded->FunctionCount;
                ++FunctionIndex)
            {
                void *Function = GetProcAddress(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if(Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    // TODO(chowie): Display error message here?
                    Loaded->IsValid = false;
                }
            }
        }
    }

    if(!Loaded->IsValid)
    {
        Win32UnloadCode(Loaded);
        // TODO(chowie): Display error message here?
    }
}

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        OutputDebugStringA("Xinput9_1_0 loaded\n");
    }

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        OutputDebugStringA("Xinput1_3 loaded\n");
    }

    if(XInputLibrary)
    {
        *(void **)(&XInputGetState) = GetProcAddress(XInputLibrary, "XInputGetState");
        *(void **)(&XInputSetState) = GetProcAddress(XInputLibrary, "XInputSetState");
        // STUDY(chowie): Compare "XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");"

        // TODO(chowie): Logging/Diagnostic
//        OutputDebugStringA("Xinput1_4 loaded successfully\n");
    }
    else
    {
        OutputDebugStringA("Xinput1_4 failed to load. Controllers will not work!\n");
    }
}

// RESOURCE: https://kodi.wiki/view/Windows_audio_APIs
// STUDY(chowie): WASAPI is COM! Stepping over the func looks though a jump table
internal void
Win32LoadWASAPI(win32_loaded_sound_code *Loaded)
{
    HMODULE WASAPILibrary = LoadLibraryA("ole32.dll");
    if(WASAPILibrary)
    {
        for(u32 FunctionIndex = 0;
            FunctionIndex < ArrayCount(Win32SoundFunctionTableNames);
            ++FunctionIndex)
        {
            // TODO(chowie): Check for failure?
            Loaded->Functions[FunctionIndex] = GetProcAddress(WASAPILibrary, Loaded->FunctionNames[FunctionIndex]);
        }

        // TODO(chowie): Logging/Diagnostic
    }
    else
    {
        OutputDebugStringA("Ole32 failed to load. Sound will not play!\n");
    }
}

// TODO(chowie): This is really not a good way to use WASAPI, make a
// thread-queue first before multithreading this! Make a guard thread
// that flushes on framedrop? I hear audio cracking - need threads!
// RESOURCE(nickav): https://hero.handmade.network/forums/code-discussion/t/8433-correct_implementation_of_wasapi
// RESOURCE(nickav): https://gist.github.com/nickav/8be2ded8a8363d5993b2f4e5aa601bd3

// RESOURCE(martins): https://gist.github.com/mmozeiko/38c64bb65855d783645c
// NOTE(chowie): Thank you Martins for providing introductory code!
internal void
Win32InitWASAPI(win32_sound_function_table Table, s32 SamplesPerSecond, s32 BufferSizeInSamples)
{
    // TODO(chowie): Abstract the audio api architecture?
    // TODO(chowie): Output HRESULT to be able to be inspected in watch window!
    // TODO(chowie): Test that FAILED = !SUCCEEDED
    if(FAILED(Table.CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY)))
    {
        Assert(!"Error");
    }

    IMMDeviceEnumerator *Enumerator;
    if(FAILED(Table.CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, IID_PPV_ARGS(&Enumerator))))
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

    // STUDY(chowie): I reckon this duplicates the two sound buffers [LEFT RIGHT], compared to DSound.
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

    // STUDY(by martins): Check if we got what we requested (better would to pass this value back as real buffer size)
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

internal b32x
Win32ProcessNextWorkQueueEntry(platform_work_queue *Queue)
{
    b32x WeShouldSleep = false;

    // NOTE(chowie): Circular FIFO queue
    u32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    u32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);

    // NOTE(chowie): Work to do
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        // RESOURCE(cohen): https://hero.handmade.network/forums/code-discussion/t/3640-question_on___possible_problem_with_multithreading_queue
        // NOTE(chowie): Prevents worker thread pickup on newer entry,
        // discarding oldest.
        platform_work_queue_entry Entry = Queue->Entries[OriginalNextEntryToRead];
        u32 IncIndex = AtomicCompareExchangeU32(&Queue->NextEntryToRead, NewNextEntryToRead, OriginalNextEntryToRead);
        if(IncIndex == OriginalNextEntryToRead)
        {
            Entry.Callback(Queue, Entry.Data);
            AtomicIncrementU32(&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }

    return(WeShouldSleep);
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    win32_thread_startup *Startup = (win32_thread_startup *)lpParameter;
    platform_work_queue *Queue = Startup->Queue;

    // STUDY(chowie): Busy-wait loop
    // TODO(chowie): Could extend loop to other queues when necessary?
    for(;;)
    {
        if(Win32ProcessNextWorkQueueEntry(Queue))
        {
            // NOTE(HmH 124): Announces suspension of thread to OS;
            // says "if semaphore > 0". Decrement semaphore count by 1
            // (on wakeup necessary threads to process data, not on
            // entry). Semaphore count flickers fast, sticks close to
            // 0 most of the time for threads to increment.
            // STUDY(chowie): Wait in this case, asks how many threads
            // are open, regardless of threadcount.
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
}

// TODO(chowie): For MPMC, switch to using InterlockCompareExchange on
// EntryCount eventually so any thread can add?
internal
PLATFORM_ADD_WORK_QUEUE_ENTRY(Win32AddWorkQueueEntry)
{
    // NOTE(chowie): Circular FIFO queue
    u32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    // NOTE(chowie): Circular queue hasn't wrapped before writing
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);

    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;

    CompletePrevWritesBeforeFutureWrites;

    Queue->NextEntryToWrite = NewNextEntryToWrite;

    // NOTE(chowie): Increment semaphore count by 1, return previous
    // one. Releases threads waiting on semaphore to continue working.
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal
PLATFORM_COMPLETE_ALL_WORK_QUEUE(Win32CompleteAllWorkQueue)
{
    // STUDY(chowie): Example of a spinlock
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32ProcessNextWorkQueueEntry(Queue);
    }

    // TODO(chowie): Temp to reset queue when all is done
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

internal void
Win32MakeWorkQueue(platform_work_queue *Queue, u32 ThreadCount, win32_thread_startup *Startups)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;

    u32 InitialCount = 0;
    Queue->SemaphoreHandle =
        CreateSemaphoreExA(0, InitialCount, ThreadCount,
                           0, 0, SEMAPHORE_ALL_ACCESS);

    for(u32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        win32_thread_startup *Startup = Startups + ThreadIndex;
        Startups->Queue = Queue;

        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc,
                                           Startup, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

inline u64
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result.QuadPart);
}

inline b32x
Win32SupportsRdtscp(void)
{
    b32x Result = false;

    int CPUInfo[4];
    __cpuid(CPUInfo, 0x80000001);

#define RdtscpInstructionSupported BitSet(27)
    if((CPUInfo[3] & RdtscpInstructionSupported) != 0)
    {
        Result = true;
    }

    return(Result);
}

inline u64
Win32ReadCPUTimer(b32x RdtscpSupported)
{
    u64 Result = 0;

    if(RdtscpSupported)
    {
        u32 Register = 0;
        Result = __rdtscp(&Register);
    }
    else
    {
        Result = __rdtsc();
    }

    return(Result);
}

// NOTE(chowie): Measured by count per/sec.
inline u64
Win32GetPerfCountFreq(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceFrequency(&Result);
    return(Result.QuadPart);
}

inline f32
Win32GetSecondsElapsed(u64 LastCounter, u64 EndCounter)
{
    f32 Result = ((f32)(EndCounter - LastCounter) /
                  GlobalPerfCountFrequency);
    return(Result);
}

internal v2u
Win32GetWindowDim(HWND Window)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    v2u Result =
    {
        (u32)(ClientRect.right - ClientRect.left),
        (u32)(ClientRect.bottom - ClientRect.top),
    };

    return(Result);
}

#define BITMAP_BYTES_PER_PIXEL 4
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, v2u WindowDim)
{
    // NOTE(chowie): Allocating new memory everything for a new buffer
    // is not ideal.
    // Either allocate before creating the a new DIB section, free old
    // DIB. Alternatively, wait and see if we can get the new one first
    // else keep using the old one.

    // NOTE(chowie): Creating first and freeing after means if we
    // needed the memory from the first one in order to get the memory
    // from the second one since it was occupying too much memory.

#if 0
    // TODO(chowie): Bulletproof this.
    // Free first, free after, then free first if that fails?
    // TODO(chowie): This is technically only called once now! REMOVE!
    if(Buffer->Memory)
    {
        // STUDY(chowie): If this was MEM_DECOMMIT. Use VirtualProtect
        // with PAGE_NOACCESS to ensure no one keeps a stale pointer
        // to the page. A "use after free" bug if it was written to!
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
#endif

    Buffer->Dim = WindowDim;

    // NOTE(chowie): When biHeight is negative, Windows treats this
    // bitmap as top-down (not bottom-up / Y is up render targets),
    // meaning the first 3-bytes of the image are the colour of the
    // top-left pixel, not the bottom-left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Dim.Width;
    Buffer->Info.bmiHeader.biHeight = Buffer->Dim.Height; // NOTE(chowie): Removed the minus, TODO(chowie): Buckle top-down bitmaps down!
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = BITMAP_BYTES_PER_PIXEL*8; // NOTE(chowie): 8-bits per 1-byte of colour
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    /*
      RESOURCE: https://learn.microsoft.com/en-us/windows/win32/medfound/image-stride

      STUDY(chowie): Storing a 2D image in a 1D "block" of memory

      Pitch/Stride = Value to added to a pointer on the first row to
      move it to the next row; byte offset between rows.
    */
    Buffer->Pitch = Align16(Buffer->Dim.Width*BITMAP_BYTES_PER_PIXEL);
    u32 BitmapMemorySize = (Buffer->Pitch*Buffer->Dim.Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferInWindow(game_render_commands *RenderCommands, HDC WindowDC,
                           v2u WindowDim)
{
    // TODO(chowie): Aspect ratio correction? Stretch modes?
    // TODO(chowie): Fix OpenGL resizing
    OpenGLRenderCommands(RenderCommands, WindowDim);
    SwapBuffers(WindowDC);
}

// NOTE(chowie): This follows Raymond Chen's prescription for fullscreen toggling
// RESOURCE(raymond chen): https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
internal void
ToggleFullscreen(HWND Window)
{
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        // STUDY(chowie): sizeof initialises the first element of
        // struct with MonitorInfo, leaving the rest as 0's.
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void
Win32GetInputFileLocation(win32_state *State, u32 SlotIndex,
                          u32 DestCount, char *Dest)
{
    Assert(SlotIndex == 1);
    Win32BuildEXEPathFileName(State, "loop_edit.hmi", DestCount, Dest);
}

// TODO(chowie): Replace CopyMemory(), with our Copy(). Remember to
// swap source and dest around! Win32 is the other way around.
// TODO(chowie): Can instead just loop over the permanent storage as
// the transient storage will reconstruct itself!
// TODO(chowie): This could live on layer above the platform?
// TODO(chowie): Strobing memory-mapped file?
// TODO(chowie): Alleviate the stalling for large input reads?
internal void
Win32BeginInputRecording(win32_state *State, u32 Index)
{
    win32_replay_buffer *Buffer = Win32GetReplayBuffer(State);
    if(!Buffer->IsInitialised)
    {
        Win32GetInputFileLocation(State, Index, sizeof(Buffer->FileName), Buffer->FileName);
        Buffer->MappedFile = CreateFileA(Buffer->FileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
        Assert(Buffer->MappedFile != INVALID_HANDLE_VALUE);

        // NOTE(chowie): Memory mapping doesn't allow extending the
        // size of the file, you must recreate the file mapping with a
        // larger size, then map a new view of the file.
        LARGE_INTEGER MaxSize;
        MaxSize.QuadPart = State->TotalSize + REPLAY_BUFFER_RESIZE_BYTES; // NOTE(chowie): Avoids the mandatory resizing for the first inputs
        Buffer->MemoryMap = CreateFileMappingA(Buffer->MappedFile, 0, PAGE_READWRITE,
                                               MaxSize.HighPart, MaxSize.LowPart, 0);
        Assert(Buffer->MemoryMap != INVALID_HANDLE_VALUE);

        Buffer->MemoryBlock = MapViewOfFile(Buffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, MaxSize.QuadPart);
        Assert(Buffer->MemoryBlock);

        Buffer->FileSize = MaxSize.QuadPart;
        Buffer->IsInitialised = true;
    }
    else
    {
        // TODO(chowie): Logging!
    }

    if(Buffer->MemoryBlock)
    {
        State->InputRecordingIndex = Index;
        CopyMemory(Buffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
        State->CurrentRecordSize = State->TotalSize;
        State->CurrentBuffer = Buffer;
    }
}

internal void
Win32EndInputRecording(win32_state *State)
{
    State->InputRecordingIndex = 0;
    State->CurrentBuffer->WrittenSize = State->CurrentRecordSize;
    State->CurrentBuffer = 0;
}

// STUDY(chowie): This is awfully similar to HmH Day 279 0h46m00s, in PackEntity()
inline b32x
Win32RecordingInputIsFull(win32_state *State, u32 BytesToWrite)
{
    b32x Result = ((State->CurrentRecordSize + BytesToWrite) >= State->CurrentBuffer->FileSize); // STUDY: Try to always succeed in recording
    return(Result);
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)
{
    if(State->CurrentBuffer)
    {
        u32 BytesToWrite = sizeof(*NewInput);
        if(Win32RecordingInputIsFull(State, BytesToWrite))
        {
            State->CurrentBuffer->FileSize += REPLAY_BUFFER_RESIZE_BYTES;
            Assert(State->CurrentBuffer->FileSize >= (State->CurrentRecordSize + BytesToWrite));

            // RESOURCE(raymond chen): https://devblogs.microsoft.com/oldnewthing/20031007-00/?p=42263
            // NOTE(chowie): Raymond Chen's shared memory technique below.
            void *OldMemory = State->CurrentBuffer->MemoryBlock;
            UnmapViewOfFile(OldMemory);

            // NOTE(chowie): This is not necessary, and goes counter
            // to what MSDN says, but we'll do it anyway!
            CloseHandle(State->CurrentBuffer->MemoryMap);

            LARGE_INTEGER MaxSize;
            MaxSize.QuadPart = State->CurrentBuffer->FileSize;
            State->CurrentBuffer->MemoryMap = CreateFileMappingA(State->CurrentBuffer->MappedFile, 0, PAGE_READWRITE,
                                                                 MaxSize.HighPart, MaxSize.LowPart, 0);
            Assert(State->CurrentBuffer->MemoryMap != 0);

            State->CurrentBuffer->MemoryBlock = MapViewOfFile(State->CurrentBuffer->MemoryMap, FILE_MAP_ALL_ACCESS,
                                                              0, 0, State->CurrentBuffer->FileSize);
            Assert(State->CurrentBuffer->MemoryBlock);
        }

        Assert(!Win32RecordingInputIsFull(State, BytesToWrite));
        void *WriteP = (void *)((u8 *)State->CurrentBuffer->MemoryBlock + State->CurrentRecordSize); // TODO(chowie): I don't think I need a void * cast see OpenGLRenderCommands void *Data
        CopyMemory(WriteP, (void *)NewInput, BytesToWrite);
        State->CurrentRecordSize += BytesToWrite; // STUDY(chowie): Like pushing onto an arena
    }
}

internal void
Win32BeginInputPlayback(win32_state *State, u32 Index)
{
    win32_replay_buffer *Buffer = Win32GetReplayBuffer(State);
    if(Buffer->IsInitialised)
    {
        if(Buffer->MemoryBlock)
        {
            State->InputPlayingIndex = Index;
            CopyMemory(State->GameMemoryBlock, Buffer->MemoryBlock, State->TotalSize);
            State->CurrentRecordSize = State->TotalSize;
            State->CurrentBuffer = Buffer;
        }
    }
    else
    {
        // TODO(chowie): Logging
    }
}

internal void
Win32EndInputPlayback(win32_state *State)
{
    State->InputPlayingIndex = 0;
    State->CurrentBuffer = 0;
}

inline b32x
Win32PlaybackInputIsFull(win32_state *State)
{
    b32x Result = (State->CurrentRecordSize >= State->CurrentBuffer->WrittenSize);
    return(Result);
}

internal void
Win32PlaybackInput(win32_state *State, game_input *NewInput)
{
    if(State->CurrentBuffer)
    {
        u32 BytesToRead = sizeof(*NewInput);
        if(Win32PlaybackInputIsFull(State))
        {
            // NOTE(chowie): We've hit the end of the stream, restart.
            u32 Index = State->InputPlayingIndex;
            Win32EndInputPlayback(State);
            Win32BeginInputPlayback(State, Index);
        }

        // NOTE(chowie): There's still input.
        Assert(!Win32PlaybackInputIsFull(State));
        void *ReadP = (void *)((u8 *)State->CurrentBuffer->MemoryBlock + State->CurrentRecordSize); // TODO(chowie): I don't think I need a void * cast
        CopyMemory((void *)NewInput, ReadP, BytesToRead);
        State->CurrentRecordSize += BytesToRead;
    }
}

// TODO(chowie): Better controller support?
internal void
Win32SetControllerVibration(void)
{
    XINPUT_VIBRATION Vibration;
    Vibration.wLeftMotorSpeed = 6000;
    Vibration.wRightMotorSpeed = 6000;
    XInputSetState(0, &Vibration);
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    // TODO(chowie): If the polling rate in increased, this may have
    // to change. Buffered XInput? Threading?
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0; // NOTE(chowie): OldState->EndedDown = WasIsDownPreviously
}

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/469-deadzone_for_controller_is_not_square,_but_cross
// STUDY(chowie): I wonder if the shape of the hardware noise profile
// is actually rectangular. A "cross" is a "region where at least one
// stick axis does nothing". A square is a "region where the stick
// does nothing". Maybe on the average case a radial deadzone better?
internal f32
Win32ProcessStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    f32 Result = 0;

    if(Value < -DeadZoneThreshold)
    {
        Result = (f32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (f32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(Result);
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, b32x IsDown)
{
    // NOTE(chowie): Because of switching windows / apps, industrial strength will be later
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

// NOTE(chowie): Conceptional purity of being in the same stack,
// nothing in particular. Rather than global variables.
internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController)
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
                // RESOURCE: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
                // STUDY(chowie): Instead of returning either
                // (1 << 30) or 0. "!= 0" forces 1 or 0.
#define KeyMessageWasDownBit BitSet(30)
#define KeyMessageWasUpBit BitSet(31)
#define AltKeyWasDownBit BitSet(29)
#define KeyStateBit BitSet(15)
                b32x WasDown = ((Message.lParam & KeyMessageWasDownBit) != 0);
                b32x IsDown = ((Message.lParam & KeyMessageWasUpBit) == 0);
                b32x AltKeyWasDown = (Message.lParam & AltKeyWasDownBit);
                b32x ShiftKeyWasDown = (GetKeyState(VK_SHIFT) & KeyStateBit);

                // NOTE(chowie): Holding down a key would otherwise
                // display both WasDown and IsDown messages!
                if(WasDown != IsDown)
                {
                    switch(VKCode)
                    {
                        case 'W':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                        } break;
                        case 'A':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                        } break;
                        case 'S':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                        } break;
                        case 'D':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                        } break;
                        case 'Q':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                        } break;
                        case 'E':
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                        } break;
                        case VK_UP:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                        } break;
                        case VK_LEFT:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                        } break;
                        case VK_DOWN:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                        } break;
                        case VK_RIGHT:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                        } break;
                        case VK_ESCAPE:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                        } break;
                        case VK_SPACE:
                        {
                            Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                        } break;

                        // NOTE(chowie): Below are pressed once keys!
                        case VK_F1:
                        case VK_F2:
                        case VK_F3:
                        case VK_F4:
                        case VK_F5:
                        case VK_F6:
                        case VK_F7:
                        case VK_F8:
                        case VK_F9:
                        case VK_F10:
                        case VK_F11:
                        case VK_F12:
                        {
                            if(IsDown)
                            {
                                if(VKCode == VK_F4)
                                {
                                    if(AltKeyWasDown)
                                    {
                                        GlobalRunning = false;
                                    }
                                }
                                else if(VKCode == VK_F11)
                                {
                                    if(Message.hwnd)
                                    {
                                        ToggleFullscreen(Message.hwnd);
                                    }
                                }
                            }
//                            Input->FKeyPressed[VKCode] = true;
                        } break;

                        case VK_RETURN:
                        {
                            if(IsDown)
                            {
                                if(AltKeyWasDown)
                                {
                                    if(Message.hwnd)
                                    {
                                        ToggleFullscreen(Message.hwnd);
                                    }
                                }
                            }
                        } break;

#if RUINENGLASS_INTERNAL
                        case 'P':
                        {
                            if(IsDown)
                            {
                                if(AltKeyWasDown)
                                {
                                    GlobalPause = !GlobalPause;
                                }
                            }
                        } break;

                        case 'L':
                        {
                            if(IsDown)
                            {
                                if(AltKeyWasDown)
                                {
                                    if(State->InputPlayingIndex == 0)
                                    {
                                        if(State->InputRecordingIndex == 0)
                                        {
                                            Win32BeginInputRecording(State, 1);
                                        }
                                        else
                                        {
                                            Win32EndInputRecording(State);
                                            Win32BeginInputPlayback(State, 1);
                                        }
                                    }
                                    else
                                    {
                                        Win32EndInputPlayback(State);
                                    }
                                }
                                else if(ShiftKeyWasDown)
                                {
                                    // NOTE(chowie): Replay previous recording (independent of recording)
                                    Win32BeginInputRecording(State, 1);
                                }
                            }
                        } break;
#endif
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
// types. Encumbant on user to cast the values, anonymous parameters
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
#if RUINENGLASS_INTERNAL
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
#endif

        case WM_CLOSE:
        {
            // TODO(chowie): Handle with an 'Are you sure?' message to user?
            GlobalRunning = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_DESTROY:
        {
            // TODO(chowie): Handle an error - recreate window?
            GlobalRunning = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        // STUDY(chowie): If keyboard messages was fielded here, it
        // would need to store the resulting input structure. However,
        // the callback's function prototype cannot be changed.
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

        // RESOURCE(martins): https://hero.handmade.network/forums/code-discussion/t/1911-opengl_swapping_buffers_only_after_the_window_resizes
        // RESOURCE(martins): https://hero.handmade.network/forums/code-discussion/t/6995-day_472_-_resizing_the_opengl_window
        // TODO(chowie): Avoid paint, use OpenGL to blit/resize -> call ValidateRect(WND, 0)
        case WM_PAINT:
        {
            // NOTE(chowie): Must be included for window redraw
            PAINTSTRUCT Paint;
            HDC WindowDC = BeginPaint(Window, &Paint);
//            v2u WindowDim = Win32GetWindowDim(Window);
//            Win32DisplayBufferInWindow(&RenderCommands, WindowDC, WindowDim);
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

// NOTE(chowie): wWinMain is the newer version and not necessary!
int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR     CommandLine,
        int       ShowCode)
{
    win32_state Win32State = {};

    Win32GetEXEFileName(&Win32State);

    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "ruinenglass.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "ruinenglass_temp.dll",
                              sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

    char LockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp",
                              sizeof(LockFullPath), LockFullPath);

    WNDCLASSA WindowClass = {};

    GlobalPerfCountFrequency = Win32GetPerfCountFreq();

    // NOTE(chowie): Fixed-sized backbuffer
    Win32ResizeDIBSection(&GlobalBackbuffer, V2U(1280, 720));

    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC; // NOTE(chowie): Repaint the whole window if resizing window.
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
//    WindowClass.hIcon = ;
//    WindowClass.hbrBackground = ;
    WindowClass.lpszClassName = "RuinenglassWindowClass";

    win32_loaded_sound_code SoundCode = {};
    win32_sound_function_table SoundTable = {};

    SoundCode.Functions = (void **)&SoundTable;
    SoundCode.FunctionNames = Win32SoundFunctionTableNames;

    Win32PreventDPIScaling();
    Win32LoadXInput();
    Win32LoadWASAPI(&SoundCode);

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
            // TODO(chowie): Weird GetMonitorInfo in fullscreen toggle
            // depends on DPI aware. Still doesn't scale correctly.
            // ToggleFullscreen(Window);
            HDC OpenGLWindowDC = GetDC(Window);
            b32x EnableVsync = true;
            HGLRC OpenGLRC = Win32InitOpenGL(OpenGLWindowDC, EnableVsync);
            // NOTE(chowie): ReleaseDC(Window, GlobalDC); Would be set
            // after the creation of the LowPriorityQueue. However,
            // there's a chance that the DC gets released before
            // creating the context. Thus, passing CS_OWNDC is better!

            // TODO(chowie): Enable when there's work to do!
#if 0
            win32_thread_startup HighPriorityStartups[6];
            platform_work_queue HighPriorityQueue = {};
            Win32MakeWorkQueue(&HighPriorityQueue, ArrayCount(HighPriorityStartups), HighPriorityStartups);

            win32_thread_startup LowPriorityStartups[2] = {};
            platform_work_queue LowPriorityQueue = {};
            Win32MakeWorkQueue(&LowPriorityQueue, ArrayCount(LowPriorityStartups), LowPriorityStartups);
#endif

            // STUDY(chowie): Audio latency is determined not by the
            // size of the buffer, but by how far ahead of the
            // PlayCursor you write. The optimal amount of latency is
            // the amount that will cause this frame's audio to
            // coincide with the display of this frame's image.
            int FramesOfAudioLatency = 1;
            int MonitorRefreshHz = 60; // TODO(chowie): Delegate this to the graphics card!
            HDC RefreshDC = GetDC(Window);
            int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            ReleaseDC(Window, RefreshDC);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            f32 GameUpdateHz = (f32)(MonitorRefreshHz);

            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000; // TODO(chowie): Set to 60 seconds?
            SoundOutput.BytesPerSample = sizeof(s16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = FramesOfAudioLatency*(u32)(SoundOutput.SamplesPerSecond / GameUpdateHz); // NOTE(chowie): Number of samples that can be played without updating with new info
            Win32InitWASAPI(SoundTable, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);

            GlobalRunning = true;
            b32x RdtscpSupported = Win32SupportsRdtscp();

#if RUINENGLASS_INTERNAL
            // RESOURCE: https://hero.handmade.network/forums/code-discussion/t/2120-day_023_looped_live_code-_use_of_fixed_base_address_for_game_memory
            // NOTE(chowie): In a single run of the program, the base
            // address should never be changed, as the memory dump
            // contains pointers only valid in the original region of
            // the memory. However, in multiple runs, the base address
            // needs to be the same to load the memory dump from a
            // previous execution of the program.
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif

            game_memory GameMemory = {};

#if RUINENGLASS_INTERNAL
            GameMemory.PlatformAPI.DEBUGFreeFileMemory = DEBUGPlatformFreeFileMemory;
            GameMemory.PlatformAPI.DEBUGReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.PlatformAPI.DEBUGWriteEntireFile = DEBUGPlatformWriteEntireFile;
#endif
            GameMemory.PlatformAPI.AllocateMemory = Win32AllocateMemory;
            GameMemory.PlatformAPI.DeallocateMemory = Win32DeallocateMemory;

//            GameMemory.HighPriorityQueue = &HighPriorityQueue;
//            GameMemory.LowPriorityQueue = &LowPriorityQueue;

            GameMemory.PlatformAPI.AddWorkQueueEntry = Win32AddWorkQueueEntry;
            GameMemory.PlatformAPI.CompleteAllWorkQueue = Win32CompleteAllWorkQueue;

            Platform = GameMemory.PlatformAPI;

            memory_arena *PermanentArena = &GameMemory.Permanent;
            PermanentArena->Size = Megabytes(64);

            memory_arena *TransientArena = &GameMemory.Transient;
            TransientArena->Size = Megabytes(256); // TODO(chowie): How big should this realisically be?

            memory_arena *SamplesArena = &GameMemory.Samples;
            SamplesArena->Size = (umm)SoundOutput.SecondaryBufferSize; // TODO(chowie): Roll "Samples + Audio" & Offset initial samples

            umm PushBufferSize = Megabytes(64); // TODO(chowie): Decide on proper push buffer size
            void *PushBuffer = Win32AllocateMemory(PushBufferSize);

            // TODO(chowie): Handle memory footprints with system metrics!
            // STUDY(chowie): Memory pooling = hard bounds at runtime
            // and memory usage. While dyanamic allocation hides the
            // platform's memory constraints; overflowing memory,
            // fragmentation, or needs more memory than it can provide.
            for(u32 ArenaSizeIndex = 0;
                ArenaSizeIndex < ArrayCount(GameMemory.E);
                ++ArenaSizeIndex)
            {
                memory_arena *Arena = GameMemory.E + ArenaSizeIndex;
                Win32State.TotalSize += Arena->Size;
            }

            // NOTE(chowie): This is for replay!
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, Win32State.TotalSize,
                                                      MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.Permanent.Base = (u8 *)Win32State.GameMemoryBlock;
            GameMemory.Transient.Base = (GameMemory.Permanent.Base +
                                         GameMemory.Permanent.Size);
            GameMemory.Samples.Base = (GameMemory.Transient.Base +
                                       GameMemory.Transient.Size);
            // TODO(chowie): I could alternatively pool with bitmap VirtualAlloc -> Subdivide it out?

            if(GameMemory.Permanent.Base &&
               GameMemory.Transient.Base &&
               GameMemory.Samples.Base)
            {
                // NOTE(chowie): Removes x4 repeat messages if a
                // controller was not plugged in!
                // TODO(chowie): Transforms the game into one where
                // you must plug the controller at startup. Not
                // ideal! Monitor XBoxControllers for being plugged in
                // after the fact! Repoll and change, but for now
                // assume they all present from the beginning!
                b32x XBoxControllerPresent[XUSER_MAX_COUNT] = {};
                for(u32 ControllerIndex = 0;
                    ControllerIndex < XUSER_MAX_COUNT;
                    ++ControllerIndex)
                {
                    XBoxControllerPresent[ControllerIndex] = true;
                }

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                u64 LastCounter = Win32GetWallClock();
                u64 LastCycleCount = Win32ReadCPUTimer(RdtscpSupported);

                win32_game_function_table Game = {};
                win32_loaded_code GameCode = {};
                GameCode.DLLFullPath = SourceGameCodeDLLFullPath;
                GameCode.TempFullPath = TempGameCodeDLLFullPath;
                GameCode.LockFullPath = LockFullPath;

                GameCode.FunctionCount = ArrayCount(Win32GameFunctionTableNames);
                GameCode.FunctionNames = Win32GameFunctionTableNames;
                GameCode.Functions = (void **)&Game;

                Win32LoadCode(&GameCode);
                u32 ExpectedFramesPerUpdate = 1;
                f32 TargetSecondsPerFrame = (f32)ExpectedFramesPerUpdate / (f32)GameUpdateHz;
                while(GlobalRunning)
                {
                    NewInput->dtForFrame = TargetSecondsPerFrame;

                    GameMemory.ExecutableReloaded = false;
                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                    if(CompareFileTime(&NewDLLWriteTime, &GameCode.DLLLastWriteTime) != 0)
                    {
                        Win32UnloadCode(&GameCode);
                        Win32LoadCode(&GameCode);

                        // NOTE(chowie): Leaves unlocked in between
                        // two lines. Because DLL is not being
                        // reloaded again. Locked while used.
                        GameMemory.ExecutableReloaded = true;
                    }

                    //
                    // NOTE(chowie): Input Processing
                    //

                    // NOTE(chowie): We can't zero everything, otherwise the up/down state will be wrong
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    ZeroStruct(*NewKeyboardController);
                    NewKeyboardController->IsConnected = true; // TODO(chowie): Poll for multiple keyboards?

                    // NOTE(chowie): Reset HalfTransitionCount (per
                    // frame), leave EndedDown.
                    for(u32 ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->E);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->E[ButtonIndex].EndedDown =
                            OldKeyboardController->E[ButtonIndex].EndedDown;
                    }

                    //
                    // NOTE(chowie): Keyboard
                    //

                    // STUDY(chowie): Messages don't always field in the
                    // dispatch queue. Windows reserves the rights to
                    // 'cold-call' sometimes (in Win32MainWindowCallback).
                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController);

                    if(!GlobalPause)
                    {
                        //
                        // NOTE(chowie): Mouse (On bounds, not half-coordinates; in pixels)
                        //

                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->Mouse.x = (f32)MouseP.x;
                        NewInput->Mouse.y = (f32)((GlobalBackbuffer.Dim.Height - 1) - MouseP.y);
                        NewInput->Mouse.z = 0; // TODO(chowie): Support Mousewheel

                        DWORD WinButtonID[PlatformMouseButton_Count]
                        {
                            VK_LBUTTON,
                            VK_MBUTTON,
                            VK_RBUTTON,
                            VK_XBUTTON1,
                            VK_XBUTTON2,
                        };

                        for(u32 ButtonIndex = 0;
                            ButtonIndex < PlatformMouseButton_Count;
                            ++ButtonIndex)
                        {
                            NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
                            NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                            Win32ProcessKeyboardMessage(&NewInput->MouseButtons[ButtonIndex],
                                                        GetKeyState(WinButtonID[ButtonIndex]) & KeyStateBit);
                        }

                        //
                        // NOTE(chowie): Controllers
                        //

                        // TODO(chowie): Avoid polling disconnected
                        // controllers to reduce xinput framerate hit
                        // on older libraries?
                        // TODO(chowie): Poll this more frequently in
                        // a separate thread for xinput sticks,
                        // lock-free queue - you would not have to
                        // have any mutexes?
                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
                        if(MaxControllerCount > ArrayCount(NewInput->Controllers) - 1) // NOTE(chowie): Excludes keyboard
                        {
                            MaxControllerCount = ArrayCount(NewInput->Controllers) - 1;
                        }

                        for(DWORD ControllerIndex = 0;
                            ControllerIndex < MaxControllerCount;
                            ++ControllerIndex)
                        {
                            DWORD OurControllerIndex = ControllerIndex + 1; // NOTE(chowie): Includes keyboard
                            game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                            game_controller_input *NewController = GetController(NewInput, OurControllerIndex);

                            XINPUT_STATE ControllerState;
                            if(XBoxControllerPresent[ControllerIndex] &&
                               XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                            {
                                // NOTE(chowie): The controller is plugged in
                                NewController->IsConnected = true;
                                NewController->IsAnalog = OldController->IsAnalog;
                                // NOTE(chowie): Guards if neither Dpad or Stick, defaults to value the
                                // controller before the previous previous frame. Old is set to
                                // analog, but New should also be set.

                                // TODO(chowie): See if ControllerState.dwPacketNumber increments too rapidly
                                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad; // STUDY(chowie): Syntactic convience by snapping the pointer

                                // RESOURCE: https://github.com/gingerBill/gb/blob/master/gb.h#L9726
                                // TODO(chowie): Right-hand thumb values?
                                NewController->StickAverage =
                                {
                                    Win32ProcessStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
                                    Win32ProcessStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
                                };

                                if((NewController->StickAverage.x != 0.0f) ||
                                   (NewController->StickAverage.y != 0.0f))
                                {
                                    NewController->IsAnalog = true;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                                {
                                    NewController->StickAverage.y = 1.0f;
                                    NewController->IsAnalog = false;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                                {
                                    NewController->StickAverage.y = -1.0f;
                                    NewController->IsAnalog = false;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                                {
                                    NewController->StickAverage.x = -1.0f;
                                    NewController->IsAnalog = false;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                                {
                                    NewController->StickAverage.x = 1.0f;
                                    NewController->IsAnalog = false;
                                }

                                //
                                // NOTE(chowie): Controller overwrites previous keyboard messages here
                                //

#define PROCESS_DIGITAL_BUTTON_THRESHOLD(Threshold, ButtonType, XInputButton)                             \
                                Win32ProcessXInputDigitalButton(Threshold,                                \
                                                                &OldController->ButtonType, XInputButton, \
                                                                &NewController->ButtonType)
#define PROCESS_DIGITAL_BUTTON(ButtonType, XInputButton) PROCESS_DIGITAL_BUTTON_THRESHOLD(Pad->wButtons, ButtonType, XInputButton)

                                // NOTE(chowie): Want to be smooth on the analog side, but look at
                                // keyboard messages for the purposes of buttons, e.g. double-tap
                                // is 3 half-transitions.
                                f32 StickThreshold = 0.5f;
                                PROCESS_DIGITAL_BUTTON_THRESHOLD((NewController->StickAverage.x < -StickThreshold) ? 1 : 0,
                                                                 MoveLeft, 1);
                                PROCESS_DIGITAL_BUTTON_THRESHOLD((NewController->StickAverage.x > StickThreshold) ? 1 : 0,
                                                                 MoveRight, 1);
                                PROCESS_DIGITAL_BUTTON_THRESHOLD((NewController->StickAverage.y < -StickThreshold) ? 1 : 0,
                                                                 MoveDown, 1);
                                PROCESS_DIGITAL_BUTTON_THRESHOLD((NewController->StickAverage.y > StickThreshold) ? 1 : 0,
                                                                 MoveUp, 1);

                                PROCESS_DIGITAL_BUTTON(ActionDown, XINPUT_GAMEPAD_A);
                                PROCESS_DIGITAL_BUTTON(ActionRight, XINPUT_GAMEPAD_B);
                                PROCESS_DIGITAL_BUTTON(ActionLeft, XINPUT_GAMEPAD_X);
                                PROCESS_DIGITAL_BUTTON(ActionUp, XINPUT_GAMEPAD_Y);

                                PROCESS_DIGITAL_BUTTON(LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
                                PROCESS_DIGITAL_BUTTON(RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);

                                PROCESS_DIGITAL_BUTTON(Start, XINPUT_GAMEPAD_START);
                                PROCESS_DIGITAL_BUTTON(Back, XINPUT_GAMEPAD_BACK);
                            }
                            else
                            {
                                // NOTE(chowie): Controller is unavailable
                                NewController->IsConnected = false;
                                XBoxControllerPresent[ControllerIndex] = false;
                            }
                        }
                    }

                    //
                    // NOTE(chowie): Game Update
                    //

                    game_render_commands RenderCommands = 
                        RenderCommandStruct(GlobalBackbuffer.Dim,
                                            PushBufferSize, PushBuffer);

                    if(!GlobalPause)
                    {
                        if(Win32State.InputRecordingIndex)
                        {
                            Win32RecordInput(&Win32State, NewInput);
                        }

                        if(Win32State.InputPlayingIndex)
                        {
                            // NOTE(chowie): Overwrites what new input
                            // was from the stream of previous inputs.
                            Win32PlaybackInput(&Win32State, NewInput);
                        }

                        // IMPORTANT(chowie): Without this check the
                        // lock file would fail. You would sleep on
                        // executable reload, or use a stub!
                        if(Game.UpdateAndRender)
                        {
                            Game.UpdateAndRender(&GameMemory, NewInput, &RenderCommands);
                        }
                    }

                    //
                    // NOTE(chowie): Audio Update - Always hit framerate, target at least 30hz
                    //

                    if(!GlobalPause)
                    {
                        // RESOURCE: https://hero.handmade.network/forums/code-discussion/t/102-day_19_-_audio_latency
                        // NOTE(chowie): Computes how much sound to write and where.
                        u32 SamplesToWrite = 0;
                        UINT32 SoundPaddingSize;
                        if(SUCCEEDED(GlobalSoundClient->GetCurrentPadding(&SoundPaddingSize)))
                        {
                            // NOTE(chowie): We want x samples (1 frames
                            // worth + latency). GetCurrentPadding returns
                            // number of samples ready and has not been
                            // read into the buffer. Thus, SamplesToWrite
                            // should be x - padding.
                            u32 MaxSampleCount = (SoundOutput.SecondaryBufferSize - SoundPaddingSize);
                            SamplesToWrite = (SoundOutput.LatencySampleCount - SoundPaddingSize);
                            if(SamplesToWrite < 0)
                            {
                                SamplesToWrite = 0;
                            }
                            Assert(SamplesToWrite <= MaxSampleCount);

                            /* STUDY(chowie): Compare vs this
                               SamplesToWrite = (int)(SoundOutput.SecondaryBufferSize - SoundPaddingSize);
                               if(SamplesToWrite > SoundOutput.LatencySampleCount)
                               {
                               SamplesToWrite = SoundOutput.LatencySampleCount;
                               }
                            */
                        }

                        game_sound_output_buffer SoundBuffer = {};
                        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                        SoundBuffer.SampleCount = SamplesToWrite;
                        SoundBuffer.Samples = (s16 *)GameMemory.Samples.Base; // NOTE(chowie): Unorthodox casting

                        // IMPORTANT(chowie): Without this check the
                        // lock file would fail, you would sleep on
                        // executable reload, or use a stub!
                        if(Game.GetSoundSamples)
                        {
                            Game.GetSoundSamples(&GameMemory, &SoundBuffer); // TODO(chowie): This seems superflous for getsound samples which takes both! But needed for fill sound buffer
                        }

                        Win32FillSoundBuffer(&SoundOutput, SamplesToWrite, &SoundBuffer);
                        GlobalSoundClient->Start();
                        // NOTE(chowie): Starts buffer the first time we
                        // fill data in it, rather than filling the buffer
                        // at init. It would introduce latency (you cannot
                        // overwrite previous samples). Called after audio
                        // engine has been initially loaded.
                    }

                    //
                    // NOTE(chowie): Frame Display
                    //

                    HDC WindowDC = GetDC(Window);
                    v2u WindowDim = Win32GetWindowDim(Window);
                    Win32DisplayBufferInWindow(&RenderCommands, WindowDC,
                                               WindowDim);
                    ReleaseDC(Window, WindowDC);

                    u64 EndCycleCount = Win32ReadCPUTimer(RdtscpSupported);

                    // TODO(chowie): Simple profilers from perfaware; keep cyclecount (represented as a percentage) & counters
                    u32 CyclesElapsed = (u32)EndCycleCount - (u32)LastCycleCount;
                    u32 EndCounter = (u32)Win32GetWallClock();
                    s32 CounterElapsed = EndCounter - (u32)LastCounter;
                    f32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);
                    f32 FPS = ((f32)GlobalPerfCountFrequency / (f32)CounterElapsed);
                    f32 MCPF = ((f32)CyclesElapsed / Square(1000.0f));
//                    OutputDebugStringA(d7sam_concat(MSPerFrame)("ms/f, ")(FPS)("f/s, ")(MCPF)("mc/f")("\n"));

                    // TODO: Should we clear these here?
                    Swap(game_input *, NewInput, OldInput);

                    // NOTE(chowie): Rather than begin/end counter at
                    // the top of the loop. The process to get back up
                    // top can take longer than expected (process got
                    // switched out), we missed that time. We would
                    // rather a single stable place with end counter.
                    LastCounter = EndCounter;
                    LastCycleCount = EndCycleCount;
                }
            }
            else
            {
                // TODO(chowie): Logging
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
