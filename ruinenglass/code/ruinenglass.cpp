/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass.h"
#include "ruinenglass_renderer.cpp"
#include "ruinenglass_audio.cpp"

/*
  RESOURCE(geier): https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
  RESOURCE: https://www.reddit.com/r/VoxelGameDev/comments/f8wv5q/minecraft_style_worlds_best_way_to_store_voxel/
  RESOURCE(0fps): https://0fps.net/2012/01/14/an-analysis-of-minecraft-like-engines/
  RESOURCE(tom forsyth): Search "voxel"; Sparse-world storage formats -
  https://tomforsyth1000.github.io/blog.wiki.html

  STUDY(chowie): There's been lots of discussion about sparse-world
  voxel storage / spatial partioning, namely between octrees and
  hashing.

  Octrees (SVO and DAGs) has certain properties appealing for some
  engines - given their popularity, for some compelling reasons:
  - "In-built" multiple LODs (more necessary for 3D engines compared
    to 2D), to achieve a highly detailed look closeby and to see
    distant horizons. Looking through a camera in a 3D world is
    non-balancing (good for octrees).
  - A heirachy of different update frequencies at certain sections,
    "leaves" of the world. (While a good hash would try to avoid any
    spatial locality).
  - Easier batching of raytracing/raymarching/marching cubes for lighting

  Octrees has caveats that I believe is important for this engine:
  - Slow with updating the "leaves" when not within its bounds,
    especially when across multiple (similar with Mortons encoding!)
  - Top-most node must encompass the whole world. Bigger worlds = more
    nodes = pointer chasing. Updates best near the center of the
    world, worse for larger worlds.
  NOTE(chowie): Technically octrees can be a "flatter", linear hash.
  Alternatively, you could do a mixed approach. A flat hashed octree
  with "bricks" (dense arrays). But I'm not considering either here.

  RESOURCE(john lin): https://voxely.net/blog/the-perfect-voxel-engine/
  Ultimately, according to Lin, octrees are "only _acceptable_ at
  storage and rendering" and doesn't consider:
  - Collision detection
  - Global illumination
  - Pathfinding
  - Per-voxel attributes (besides albedo and normals)
  - Dynamic objects (bouncy/physics-y).

  RESOURCE(arseny kapoulkine): https://zeux.io/2017/03/27/voxel-terrain-storage/
  * tags difficulty
  Octrees stands against these intentions for this engine:
  - Heavy pathfinding, coop pathfinding (David Silverman)
  - Frequent non-static entity updates each frame at certain intervals,
    ~100 for pathfinding and up to 500 for copy-by-movement "stretching".
  - Updates (see "stretching") that may be potentially larger than a
    "chunk" (chunks are typically 16^3 numbers chosen in part for
    physics update see Kapoulkine of Roblox, or 32^3 see Aflockofmeese
    for modern hardware)
  * Non-uniform grid (basically a lot more maths)
      = To use an octree may instead require a k-D tree for updates
      = Harder for typical raytracing assumptions
      = View-frustums slightly harder
  - No realistic lighting (I'm not a graphics programmer)

  Thus, I plan to use hashing for multiple other benefits:
  - Quick / flat access for _neighbourhood access_ being the most
    common op (octrees updates based on the number of "leaves").
  - No "edge" to the world unlike octrees, nor do they care about how
    far is the world is from one side to the other.
  - Coordinates are flexible. Doesn't require the voxel to conform to
    a strict uniform placement. Allows for off-grid placement. However,
    you do lose specific compression for uniform grids.
  NOTE(chowie): Using octrees vs hash steers towards the size of
  the voxel itself; the smaller the voxel -> octrees.

  In addition to hashing, I plan to also use:
  - Sim regions (can mimic the effect of different update frequencies)
  - RLE Compression + LZ4 NOTE(chowie): Doing anymore is probably
    overkill according to Kapoulkine. However, an interesting
    exploration is doing Morton encoding compression on x,y,z coords
    and neighbourhood access while still compressed. For larger
    worlds, to conserve memory, you would need to test if compression
    and recompression timings outweighs accessing through Morton's
    compression using PEXT.
*/

// IMPORTANT(chowie): Remember renderer is bottom-up / Y is up
// rendered. Consider this when reading world data on disk!
struct world_position
{
    v3s Chunk;
    v3 Offset_; // NOTE: Offsets from chunk center
};

// NOTE(chowie): Actual 3D chunks, not like Minecraft's top-to-bottom (16x16x128)
struct world_chunk
{
    v3s Chunk;
};

struct world
{
    v3 ChunkDimInMeters;
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->Permanent.Size);
    game_state *GameState = (game_state *)Memory->Permanent.Base; // STUDY(chowie): Cold-cast
    if(!GameState->IsInitialised)
    {
        GameState->Offset = V2(500.0f, 500.0f);

        // TODO(chowie): Initialise Audio State when there's proper
        // sound! Do I have to rearrange "audio -> game" to prevent
        // audio clipping at the beginning?
//        InitialiseAudioState(&GameState->AudioState);

        InitialiseArena(&GameState->WorldArena,
                        Memory->Permanent.Size - sizeof(game_state),
                        Memory->Permanent.Base + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        // STUDY(chowie): One less thing the platform layer has to do; the game would always.
        GameState->IsInitialised = true;
    }

    Assert(sizeof(transient_state) <= Memory->Transient.Size);
    transient_state *TranState = (transient_state *)Memory->Transient.Base;
    if(!TranState->IsInitialised)
    {
        InitialiseArena(&TranState->TranArena,
                        Memory->Transient.Size - sizeof(transient_state),
                        Memory->Transient.Base + sizeof(transient_state));

        TranState->IsInitialised = true;
    }

    // STUDY(chowie): Shortcut to avoid call with "&" all of the time
    render_group RenderGroup_ = BeginRenderGroup(RenderCommands);
    render_group *RenderGroup = &RenderGroup_;

    //
    // NOTE(chowie): World Init & Generation
    //

    PushClear(RenderGroup, V4(0.25f, 0.25f, 0.25f, 0.0f));
//    PushCircle(RenderGroup, V3(600, 600, 0), 50.0f, 24, V4(0.5f, 1.0f, 0.5f, 0.5f));

    // RESOURCE: https://accuratesigns.net/the-sign-letter-height-visibility-chart/
    // NOTE: Average human height is 1.7m
    // TODO(chowie): Once have perspective transform & camera
    // viewport, remove everything but tilesideinmeters!
//    r32 PixelsToMeters = 1.0f / 42.0f;
    v3 TileSideInMeters = V3(1.0f, 1.3f, 1.0f); // TODO(chowie): Change these to TotalTesselationVoxelEdge
    v3 TileSideInPixels = V3(80.0f, 95.0f, 80.0f); // TODO(chowie): REMOVE! This is more of a renderer concept, not the tiles
    v3 MetersToPixels = TileSideInPixels / TileSideInMeters; // TODO(chowie): REMOVE! This is more of a renderer concept, not the tiles

    v3 Offset = {};

    // TODO(chowie): Replace these with proper coordinate systems!
    // TODO(chowie): Is it easier to make these a m3x3 matrix?
    // TODO(chowie): Express this as a ratio, clamp01?
    v2 CornerDim = MetersToPixels.xy*V2(0.3f, 0.3f); // 20.0f, 20.0f
    v2 TileDim = MetersToPixels.xy*V2(0.8f, 1.3f); // 60.0f, 75.0f
    v2 SEdgeDim = V2(TileDim.x, CornerDim.y);
    v2 WEdgeDim = V2(CornerDim.x, TileDim.y);
    // RESOURCE: On Naming - https://en.wikipedia.org/wiki/List_of_Euclidean_uniform_tilings
    v2 TesselationDim = TileDim + CornerDim;

    // TODO(chowie): Simplify b32 with DeMorgan's Law/Truth table?
    // STUDY(chowie): In practice, the compiler does this, can pull
    // this out into b32x to more easily read them linguistically.

    // RESOURCE(amit): https://www.redblobgames.com/grids/edges/#corners
    u32 TilesPerHeight = 4;
    u32 TilesPerWidth = 8;
    // TODO(chowie): Is it possible to make row y and column x?
    for(u32 Row = 0;
        Row < 2*TilesPerHeight;
        ++Row)
    {
        for(u32 Column = 0;
            Column < 2*TilesPerWidth;
            ++Column)
        {
            // TODO(chowie): Encode these using triangles numbers or something? !Odd(Row + Column)
            v4 BaseColour = {};
            if(Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): Tile = Violet
                BaseColour = V4(0.3f, 0.3f, 0.7f, 1);
            }
            else if(Odd(Row) && !Odd(Column))
            {
                // NOTE(chowie): W Edge = Green
                BaseColour = V4(0.5f, 0.7f, 0.5f, 1);
            }
            else if(!Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): S Edge = Pastel
                BaseColour = V4(0.7f, 0.3f, 0.3f, 1);
            }
            else
            {
                // NOTE(chowie): Corner = Grey
                BaseColour = V4(0.1f, 0.1f, 0.1f, 1);
            }

            // TODO(chowie): Tesselate for n-sized group tiles?
            v2 Min;
            Min.x = ((Column - 1*Odd(Column)) / 2)*TesselationDim.x + CornerDim.x*Odd(Column);
            Min.y = ((Row - 1*Odd(Row)) / 2)*TesselationDim.y + CornerDim.y*Odd(Row);

            v2 Max;
            Max.x = Min.x + CornerDim.x*!Odd(Column) + SEdgeDim.x*Odd(Column);
            Max.y = Min.y + CornerDim.y*!Odd(Row) + WEdgeDim.y*Odd(Row);

            /*
            if(!Odd(Row))
            {
                Min.y = (Row / 2)*TesselationDim.y;
                Max.y = Min.y + CornerDim.y;
            }
            else
            {
                Min.y = ((Row - 1) / 2)*TesselationDim.y + CornerDim.y;
                Max.y = Min.y + WEdgeDim.y;
            }

            if(!Odd(Column))
            {
                Min.x = (Column / 2)*TesselationDim.x;
                Max.x = Min.x + CornerDim.x;
            }
            else
            {
                Min.x = ((Column - 1) / 2)*TesselationDim.x + CornerDim.x;
                Max.x = Min.x + SEdgeDim.x;
            }
            */

            PushRect(RenderGroup, Offset, RectMinMax(Min, Max), BaseColour);
        }
    }

    //
    // NOTE(chowie): Play World
    //

    r32 dt = Input->dtForFrame;
    for(u32 ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = &Input->Controllers[ControllerIndex];
        controlled_player *ConPlayer = GameState->ControlledPlayer + ControllerIndex;
        if(Controller->IsAnalog)
        {
            ConPlayer->ddP += Controller->StickAverage;
        }
        else
        {
            ZeroStruct(*ConPlayer);
            if(IsDown(Controller->MoveUp))
            {
                ++ConPlayer->ddP.y;
            }
            if(IsDown(Controller->MoveDown))
            {
                --ConPlayer->ddP.y;
            }
            if(IsDown(Controller->MoveLeft))
            {
                --ConPlayer->ddP.x;
            }
            if(IsDown(Controller->MoveRight))
            {
                ++ConPlayer->ddP.x;
            }

            // RESOURCE(HmH): https://guide.handmadehero.org/code/day211/#3368
            // STUDY(chowie): Numerical simulation, for proper ODE would need global t

            // TODO(chowie): Change from pixels/second
            // TODO(chowie): Should it really be MaxddP > 1.0f?
            ConPlayer->ddP = NOZ(ConPlayer->ddP);

            r32 Speed = 30.0f;
            ConPlayer->ddP *= Speed;

            r32 Drag = 8.0f;
            ConPlayer->ddP += -Drag*GameState->dP;

            // RESOURCE(HmH): https://guide.handmadehero.org/code/day043/#4686
            // RESOURCE: Thomas and Finney calculus for more!
            // NOTE(chowie): (1/2)at^2 + vt + p
            GameState->Offset += MetersToPixels.xy*(0.5f*ConPlayer->ddP*Square(dt) + GameState->dP*dt);
            GameState->dP += ConPlayer->ddP*dt;
        }
    }

    v4 HeroColour = V4(0.5f, 0.2f, 0.5f, 1);
    v2 PlayerDim = MetersToPixels.xy*V2(0.8f, 1.4f);
    PushRect(RenderGroup, V3(GameState->Offset, 0), PlayerDim, HeroColour);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->Permanent.Base; // TODO(chowie): Replace with an dedicated Audio.Base

    // TODO(chowie): Allow sample offsets here for more robust
    // platform options!
    // TODO(chowie): OutputPlayingSounds() Mixer
//    TestOutputWilwaDialTone(&GameState->AudioState, SoundBuffer);
}
