#if !defined(WIN32_RUINENGLASS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

/*
  RESOURCE: https://d3s.mff.cuni.cz/legacy/~holub/c_features.html
  STUDY(chowie): Defining an array at the end of a struct e.g.
  BITMAPINFO, allows variable-length arrays accessor-wise. Don't
  recommend.
  ______________________________________________________________
  RESOURCE(BitTwiddleGames): https://www.reddit.com/r/learnprogramming/comments/12q5bho/can_somebody_explain_back_buffers_and_how_they/
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
    v2u Dim;
    s32 Pitch;
};

struct win32_sound_output
{
    u32 SamplesPerSecond;
    u32 RunningSampleIndex; // TODO(chowie): Record with this!
    u32 BytesPerSample;
    DWORD SecondaryBufferSize;
    u32 LatencySampleCount;
    // TODO(chowie): BytesPerSecond field would make for easier computation
    // TODO(chowie): Should RunningSampleIndex be in bytes?
};

//
// NOTE: Live-loop
//

// RESOURCE(gilman & nimbok): https://hero.handmade.network/forums/code-discussion/t/1990-memory_mapping_.hmi_files_and_interpreting_msdn

// STUDY(nimbok): "When using memory-mapped files, operates in memory
// isn't immediately reflected in file itself. Windows does not write
// into files until it swaps mapped memory out of RAM (physical mem).
// This could be in pieces or in aggregate; Same idea as swapping data
// between RAM and paging memory to make room in RAM, instead of
// paging the file, it uses your mapped file on disk."

// STUDY(nimbok): "Windows guarantees coherency among views of the
// same file. However, the views doesn't necessarily match the data in
// the file itself in a given moment."

// IMPORTANT(chowie): Never use MAX_PATH for user-facing code. We
// would get a truncated filepath if it was larger than was passed in!
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
#define REPLAY_BUFFER_RESIZE_BYTES Kilobytes(4)
struct win32_replay_buffer
{
    HANDLE MappedFile;
    HANDLE MemoryMap;
    u64 FileSize;
    void *MemoryBlock;

    u64 WrittenSize; // STUDY(chowie): Read/write are blocking API, guarantees file contains data on next LOC)

    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    b32x IsInitialised;
};

// NOTE(chowie): Memory snapshot is akin to an emulator's save-states
struct win32_state
{
    u64 TotalSize;
    void *GameMemoryBlock;

    win32_replay_buffer *CurrentBuffer;
    win32_replay_buffer SaveBuffer; // TODO(chowie): Remove? But I think you must have a second copy. Change to pointer?

    u32 InputRecordingIndex;
    u32 InputPlayingIndex;
    u64 CurrentRecordSize;

    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
};
internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *State)
{
    win32_replay_buffer *Result = &State->SaveBuffer;
    return(Result);
}

//
// NOTE: Hot Reloading
//

// RESOURCE(casey): https://guide.handmadehero.org/code/day575/
// TODO(chowie): Make a renderer, with it hot-reloadable?
#define WIN32_LOADED_CODE_ENTRY_POINT(name) b32x name(HMODULE Module, void *FunctionTable)
typedef WIN32_LOADED_CODE_ENTRY_POINT(win32_loaded_code_entry_point);

// RESOURCE(lacton): https://hero.handmade.network/forums/code-discussion/t/2686-function_pointer_assignment_trick
// RESOURCE(kerrisk): https://man7.org/linux/man-pages/man3/dlopen.3.html
// NOTE(chowie): A big advantage is assigning to a struct or
// an array of function pointers, since you can use a loop.
// STUDY(chowie): This was how the function started as
// *(void **)(&XInputGetState) = GetProcAddress(XInputLibrary, "XInputGetState");
struct win32_loaded_code
{
    b32x IsValid;

    char *DLLFullPath;
    char *TempFullPath;
    char *LockFullPath;

    HMODULE DLL;
    FILETIME DLLLastWriteTime;

    u32 FunctionCount;
    char **FunctionNames;
    void **Functions;
};

struct win32_game_function_table
{
    // IMPORTANT(chowie): All callbacks can be null! Must check before
    // calling, or check the IsValid in Win32LoadedGameCode
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;
};
// TODO(chowie): Introspection to automatically expand here? Removing
// the need to double-up calling function names!
global char *Win32GameFunctionTableNames[] =
{
    "GameUpdateAndRender",
    "GameGetSoundSamples",
};

#define CO_INITIALIZE_EX(name) HRESULT WINAPI name(LPVOID pvReserved, DWORD dwCoInit)
typedef CO_INITIALIZE_EX(co_initalize_ex);

#define CO_CREATE_INSTANCE(name) HRESULT WINAPI name(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
typedef CO_CREATE_INSTANCE(co_create_instance);

// TODO(chowie): Generalise this?
// NOTE(chowie): Sound functions are always loaded unlike game_functions!
struct win32_loaded_sound_code
{
    char **FunctionNames;
    void **Functions;
};

struct win32_sound_function_table
{
    // IMPORTANT(chowie): All callbacks can be null! Must check before
    // calling, or check the IsValid in Win32LoadedGameCode
    co_initalize_ex *CoInitializeEx;
    co_create_instance *CoCreateInstance;
};
// TODO(chowie): Introspection to automatically expand here? Removing
// the need to double-up calling function names!
global char *Win32SoundFunctionTableNames[] =
{
    "CoInitializeEx",
    "CoCreateInstance",
};

//
// NOTE: Multi-threading
//

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

// STUDY(chowie): Queues are necessary as thread time-to-process might
// vary drastically for each operation.
// STUDY(HmH Ray): Volatile prevents register / stack cache in a
// visible for multiple threads value that is changing on one thread
// at a time. For accumulators!
// NOTE(HmH 124): Semaphore is a countable weight, a number the OS
// tracks incremented or decremented.
struct platform_work_queue
{
    platform_work_queue_entry Entries[256];

    HANDLE SemaphoreHandle;

    u32 volatile CompletionGoal;
    u32 volatile CompletionCount;
    u32 volatile NextEntryToWrite;
    u32 volatile NextEntryToRead;
};

struct win32_thread_startup
{
    platform_work_queue *Queue;
};

//
// NOTE: Late bindings (manually calls GetProcAddress)
//

// NOTE(chowie): XInputGetState
// STUDY(chowie): This avoids execution violations by declaring a
// pointer to function with that signature
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(chowie): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// NOTE(chowie): SetProcessDpiAware
#define SET_PROCESS_DPI_AWARE(name) BOOL WINAPI name(void)
typedef SET_PROCESS_DPI_AWARE(set_process_dpi_aware);
global set_process_dpi_aware *SetProcessDpiAware_;
#define SetProcessDpiAware SetProcessDpiAware_

// NOTE(chowie): SetProcessDpiAwarenessContext
#define SET_PROCESS_DPI_AWARENESS_CONTEXT(name) BOOL WINAPI name(DPI_AWARENESS_CONTEXT);
typedef SET_PROCESS_DPI_AWARENESS_CONTEXT(set_process_dpi_awareness_context);
global set_process_dpi_awareness_context *SetProcessDpiAwarenessContext_;
#define SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_

//
//
//

// HRESULT Test = HRESULT_FROM_WIN32(GetLastError());

/*
// TODO(chowie): Buckle with this Keyboard test case!
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
*/

// TODO(chowie): Sparse files?
// #include <winioctl.h> // NOTE(chowie): For sparse files; above macros removes this
// RESOURCE(microsoft): https://github.com/microsoft/cpprestsdk/blob/master/Release/tests/functional/streams/CppSparseFile.cpp
// TODO(chowie): When copying directly to memory, seems like
// it makes little difference. Do I need to define the range?
// DWORD Ignored;
// DeviceIoControl(Buffer->MappedFile, FSCTL_SET_SPARSE, 0, 0, 0, 0, &Ignored, 0);

/* TODO(chowie): Audio update reference
   UINT64 PositionFrequency;
   UINT64 PositionUnits;

   IAudioClock* AudioClock;
   GlobalSoundClient->GetService(IID_PPV_ARGS(&AudioClock));
   AudioClock->GetFrequency(&PositionFrequency);
   AudioClock->GetPosition(&PositionUnits, 0);
   AudioClock->Release();
                        
   Marker->PlayCursor = (DWORD)(SoundOutput.SamplesPerSecond * PositionUnits / PositionFrequency) % SoundOutput.SamplesPerSecond;
*/

#define WIN32_RUINENGLASS_H
#endif
