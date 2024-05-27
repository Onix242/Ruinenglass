#if !defined(WIN32_RUINENGLASS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// RESOURCE(gilman & nimbok): https://hero.handmade.network/forums/code-discussion/t/1990-memory_mapping_.hmi_files_and_interpreting_msdn
// TODO(chowie): Test this code!
// TODO(chowie): Clean this up!
// NOTE(chowie): Never use MAX_PATH for user-facing code, we would get
// a truncated file path if it was larger than it was passed in!
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer
{
    HANDLE MappedFile; // NOTE(chowie): FileHandle
    HANDLE MemoryMap;
    u64 FileSize;
    void *Memory; // NOTE(chowie): MemoryBlock // TODO(chowie): (u8 *) instead?

    // NOTE(chowie): Only for win32_current_buffer
    u64 WrittenSize;

    // NOTE(chowie): Only for replay_buffers
    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    b32x IsInitialised;
};

// NOTE(chowie): Memory snapshot is akin to a save-state like an emulator
struct win32_state
{
    u64 TotalSize;
    void *GameMemoryBlock;

    win32_replay_buffer *CurrentBuffer;
    win32_replay_buffer ReplayBuffers; // TODO(chowie): Does this need to be passed by ref?

//    HANDLE RecordingHandle;
    u32 InputRecordingIndex;
//    HANDLE InputPlayingHandle;
    u32 InputPlayingIndex;
    u64 CurrentRecordSize;

    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
};

// RESOURCE(casey): https://guide.handmadehero.org/code/day575/
// TODO(chowie): Make a renderer, with it hot-reloadable?
#define WIN32_LOADED_CODE_ENTRY_POINT(name) b32x name(HMODULE Module, void *FunctionTable)
typedef WIN32_LOADED_CODE_ENTRY_POINT(win32_loaded_code_entry_point);

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
    u32 RunningSampleIndex; // TODO(chowie): Record this!
    u32 BytesPerSample;
    DWORD SecondaryBufferSize;
    u32 LatencySampleCount;
    // TODO(chowie): BytesPerSecond field would make for easier computation
    // TODO(chowie): Should RunningSampleIndex be in bytes?
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

// NOTE(chowie): CoInitalizeEx
#define CO_INITIALIZE_EX(name) HRESULT WINAPI name(LPVOID pvReserved, DWORD dwCoInit)
typedef CO_INITIALIZE_EX(co_initalize_ex);
global co_initalize_ex *CoInitializeEx_;
#define CoInitializeEx CoInitializeEx_

// NOTE(chowie): CoCreateInstance
#define CO_CREATE_INSTANCE(name) HRESULT WINAPI name(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
typedef CO_CREATE_INSTANCE(co_create_instance);
global co_create_instance *CoCreateInstance_;
#define CoCreateInstance CoCreateInstance_

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

// RESOURCE(microsoft): https://github.com/microsoft/cpprestsdk/blob/master/Release/tests/functional/streams/CppSparseFile.cpp
// #include <winioctl.h> // NOTE(chowie): For sparse files; above macros removes this
// DWORD Ignored;
// DeviceIoControl(Buffer->MappedFile, FSCTL_SET_SPARSE, 0, 0, 0, 0, &Ignored, 0);
// NOTE(chowie): When copying directly to memory, seems like
// it makes hardly any difference. Probably need to define the range?

            /*
            win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[1];

            // TODO: Recording systems takes too long on record start, find out what
            // windows is doing. And can speed up / defer some of that processing.
                
            Win32GetInputFileLocation(&Win32State, 1, sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);

            ReplayBuffer->MappedFile =
                CreateFileA(ReplayBuffer->FileName, 
                            GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

            LARGE_INTEGER MaxSize;
            MaxSize.QuadPart = Win32State.TotalSize;
            ReplayBuffer->MemoryMap = CreateFileMappingA(
                ReplayBuffer->MappedFile, 0, PAGE_READWRITE,
                MaxSize.HighPart, MaxSize.LowPart, 0);

            ReplayBuffer->Memory = MapViewOfFile(
                ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
            */

#define WIN32_RUINENGLASS_H
#endif
