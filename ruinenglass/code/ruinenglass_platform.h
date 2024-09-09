#if !defined(RUINENGLASS_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

/*
  NOTE(chowie):

  RUINENGLASS_INTERNAL:
  0 - Build for public release
  1 - Build for developer only

  RUINENGLASS_SLOW:
  0 - No slow code allowed!
  1 - Slow code welcome.
*/

// NOTE(chowie): Prevent brackets {} from screwing formatting in Emacs
// STUDY(chowie): Extern prevents _C++ name mangling_
#define CExternStart extern "C" {
#define CExternEnd }

#ifdef __cplusplus
CExternStart
#endif

#include "ruinenglass_types.h"
#include "ruinenglass_memory.h"

//
// NOTE: Services that the platform layer provides to game
//

#if RUINENGLASS_INTERNAL

typedef struct debug_read_file_result
{
    u32 ContentsSize;
    void *Contents;
} debug_read_file_result;

/*
  IMPORTANT(chowie):

  These are not for doing anything in the shipping game - they
  are blocking and the write does not protect against lost data!

  When writing to a critical file, don't overwrite the old file
  because it can fail; it could only partially overwrite (which can
  corrupt the file).

  TODO(chowie):

  Instead, write to a different file with a rolling buffer scheme. A
  and B file on alternating runs of the game. Or a temp file, delete
  the old file, rename it in its place. Rename would have to fail.
*/

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32x name(char *FileName, u32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#endif

#define PLATFORM_ALLOCATE_MEMORY(name) void *name(umm Size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void *Memory)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

#define PLATFORM_ADD_WORK_QUEUE_ENTRY(name) void name(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
typedef PLATFORM_ADD_WORK_QUEUE_ENTRY(platform_add_work_queue_entry);

#define PLATFORM_COMPLETE_ALL_WORK_QUEUE(name) void name(platform_work_queue *Queue)
typedef PLATFORM_COMPLETE_ALL_WORK_QUEUE(platform_complete_all_work_queue);

// STUDY(chowie): Custom dispatch as if a visible V-table
typedef struct platform_api
{
    platform_add_work_queue_entry *AddWorkQueueEntry;
    platform_complete_all_work_queue *CompleteAllWorkQueue;
    
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;

#if RUINENGLASS_INTERNAL
    debug_platform_free_file_memory *DEBUGFreeFileMemory;
    debug_platform_read_entire_file *DEBUGReadEntireFile;
    debug_platform_write_entire_file *DEBUGWriteEntireFile;
#endif
} platform_api;

typedef struct game_memory
{
    // IMPORTANT(chowie): VirtualAlloc is guaranteed to be cleared to 0 at startup!
    FIELD_ARRAY(memory_arena,
    {
        memory_arena Permanent; // NOTE(chowie): What game needs to be run; entity state, player
        memory_arena Transient; // NOTE(chowie): Render stack, cache textures, sounds; recreated and can be loaded off disc
        memory_arena Samples; // TODO(chowie): Make this into a dedicated audio arena -> suballocate room for SoundSamples
        // TODO(chowie): Debug Arena?
        // TODO(chowie): How would I use this? -> "char ArenaBuffer[sizeof(memory_arena)];"
    });
    // TODO(chowie): Flush transient store!

    // TODO(chowie): Use!
    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;

    platform_api PlatformAPI;
    b32x ExecutableReloaded;
} game_memory;

//
// NOTE: Services that the game provides to the platform layer.
// Takes - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
//

typedef struct game_sound_output_buffer
{
    u32 SamplesPerSecond;
    u32 SampleCount;
    s16 *Samples; // IMPORTANT(chowie): Must be padded to a multiple of 4!
} game_sound_output_buffer;

typedef struct game_button_state
{
    u32 HalfTransitionCount; // NOTE(chowie): Either up or down = "half" a keypress
    b32x EndedDown;
} game_button_state;

inline b32x
WasPressed(game_button_state State)
{
    b32x Result = ((State.HalfTransitionCount > 1) ||
                  ((State.HalfTransitionCount == 1) && State.EndedDown));
    return(Result);
}

inline b32x
IsDown(game_button_state State)
{
    b32x Result = (State.EndedDown);
    return(Result);
}

typedef struct game_controller_input
{
    b32x IsConnected;
    b32x IsAnalog;
    v2 StickAverage;

    FIELD_ARRAY(game_button_state,
    {
        game_button_state MoveUp;
        game_button_state MoveDown;
        game_button_state MoveLeft;
        game_button_state MoveRight;

        game_button_state ActionUp;
        game_button_state ActionDown;
        game_button_state ActionLeft;
        game_button_state ActionRight;

        game_button_state LeftShoulder;
        game_button_state RightShoulder;

        game_button_state Back;
        game_button_state Start;
    });
} game_controller_input;

enum game_input_mouse_button
{
    PlatformMouseButton_Left,
    PlatformMouseButton_Middle,
    PlatformMouseButton_Right,
    PlatformMouseButton_Extended0,
    PlatformMouseButton_Extended1,

    PlatformMouseButton_Count,
};

typedef struct game_input
{
    // STUDY(chowie): The wall clock is passed as an input device
    // since time is passing (the elapsed interval). What do you have
    // to record to replay a user's session? Keyboard, mouse and time.
    r32 dtForFrame;

    b32x QuitRequested;

// NOTE(chowie): Includes Keyboard
#define MAX_CONTROLLERS 5
    game_controller_input Controllers[MAX_CONTROLLERS];

    game_button_state MouseButtons[PlatformMouseButton_Count];
    v3 Mouse;
    // TODO(chowie): Find better ways for ShiftDown, ControlDown, AltDown etc...
} game_input;

// NOTE(chowie): Processed in Win32, game sees a virtualised
// controller. If the keyboard is functioning like a controller, the
// platform is responsible for that. To detect different keyboards.
inline game_controller_input *
GetController(game_input *Input, u32 ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers)); // TODO(chowie): Proper bounds checking?
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

//
// NOTE(chowie): Exported Functions
//

struct game_render_commands;
#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
CExternEnd
#endif

// RESOURCE: https://www.computerenhance.com/p/powerful-page-mapping-techniques
// https://github.com/cmuratori/computer_enhance/blob/main/perfaware/part3/listing_0122_write_watch_main.cpp
// TODO(chowie): Can I use any of these page mapping techniques?
// TODO(chowie): Circular buffer for Worker Threads? Probably with VirtualAlloc2 + PagedMemory // RESOURCE: https://www.computerenhance.com/p/powerful-page-mapping-techniques

/*
  NOTE(chowie): If you're every feeling stuck / unsure

  IMPORTANT(chowie): When dealing with memory always check Windows' Task
  Manager to ensure the memory allocated is reasonable!

  RESOURCE: https://austinmorlan.com/posts/pass_by_value_vs_pointer/
  TODO(chowie): For performance critical code, check where
  pointer-aliasing could happen!

  STUDY(chowie): Pointer-aliasing is when two pointers could point to
  the same memory and the compiler doesn't know if a _write_ to one of
  those pointers might effect a read from the other pointer. (Assuming
  it's non-volatile).
  *A = *B;
  *D = 5;
  *C = *B;

  STUDY(chowie): Bitshifting a negative value can never equal 0, it
  becomes -1.

  STUDY(chowie): Remember that modulo op transforms an absolute size
  to relative!

  STUDY(chowie): OS like handles, sockets, file streams needs a way to
  talk to you. Either indexes into tables or pointers into kernel
  memory.

  IMPORTANT(chowie): On function naming, make NOT create (latter is
  for functions I didn't write), Win32 and DEBUG prefix.
*/

#define RUINENGLASS_PLATFORM_H
#endif
