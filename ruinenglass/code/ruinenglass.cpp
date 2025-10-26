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
#include "ruinenglass_world.cpp"
#include "ruinenglass_ui.cpp"

// TODO(chowie): Buckle down entity "index"/ID!
internal entity *
GetEntity(game_state *GameState, u32 Index)
{
    entity *Entity = 0;

    if((Index > 0) && (Index < ArrayCount(GameState->Entities)))
    {
        Entity = &GameState->Entities[Index];
        Entity->EntityID = Index;
    }

    return(Entity);
}

internal u32
AddEntity(game_state *GameState, entity_type Type, world_pos *P)
{
    Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
    u32 EntityIndex = GameState->EntityCount++;

    entity *Entity = &GameState->Entities[EntityIndex];
    ZeroStruct(*Entity);
    Entity->Type = Type;

    if(P)
    {
        Entity->P = *P;
        ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, 0, P);
    }

    return(EntityIndex);
}

internal u32
AddPlayer(game_state *GameState)
{
    world_pos P = GameState->CameraP;
    u32 EntityIndex = AddEntity(GameState, EntityType_Player, &P);
    entity *Entity = GetEntity(GameState, EntityIndex);
    AddFlag(Entity->Flags, EntityFlag_Collides|EntityFlag_Moveable);

    return(EntityIndex);
}

internal u32
AddWall(game_state *GameState, v3s AbsTile)
{
    world_pos P = ChunkPosFromTilePos(GameState->World, AbsTile);
    u32 EntityIndex = AddEntity(GameState, EntityType_Wall, &P);
    entity *Entity = GetEntity(GameState, EntityIndex);
    AddFlag(Entity->Flags, EntityFlag_Collides);

    return(EntityIndex);
}

internal void
MoveEntity(game_state *GameState, entity *Entity, f32 dt, v2 ddP, v2 MetersToPixels)
{
    world *World = GameState->World;

    // RESOURCE(HmH): https://guide.handmadehero.org/code/day211/#3368
    // STUDY(chowie): Numerical simulation, for proper ODE would need global t
    // RESOURCE(): https://web.archive.org/web/20210724053826/https://lolengine.net/blog/2015/05/03/damping-with-delta-time
    // TODO(chowie): You could dampen with lerp or pow/exp?

    // TODO(chowie): Change from pixels/second
    // TODO(chowie): Should it really be MaxddP > 1.0f?
    ddP = NOZ(ddP);

    f32 Speed = 30.0f;
    ddP *= Speed;

    f32 Drag = 8.0f;
    ddP += -Drag*Entity->dP;

#if 0
    // TODO(chowie): Change back to this!
    // RESOURCE(HmH): https://guide.handmadehero.org/code/day043/#4686
    // RESOURCE: Thomas and Finney calculus for more!
    // NOTE(chowie): (1/2)at^2 + vt + p
    Entity->P += MetersToPixels*(0.5f*ddP*Square(dt) + Entity->dP*dt);
    Entity->dP += ddP*dt;
#else
    v2 OldEntityP = Entity->P.Offset_.xy;
    v2 EntityDelta = (0.5f*ddP*Square(dt) +
                      Entity->dP*dt);
    Entity->dP = ddP*dt + Entity->dP;
    v2 NewEntityP = OldEntityP + EntityDelta;

    world_pos NewP = MapIntoChunkSpace(GameState->World, GameState->CameraP, Entity->P.Offset_);

    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->EntityID, &Entity->P, &NewP);
    Entity->P = NewP;
#endif
}

inline v2
GetCameraSpaceP(game_state *GameState, entity *Entity)
{
    v3 Diff = Subtract(GameState->World, &Entity->P, &GameState->CameraP);
    v2 Result = Diff.xy;
    return(Result);
}

// TODO: IMPORTANT: Use the idea of SimBounds!
internal void
SetCamera(game_state *GameState, world_pos NewCameraP, v3 TileSideInMeters)
{
    world *World = GameState->World;

    v3 dCameraP = Subtract(World, &NewCameraP, &GameState->CameraP);
    GameState->CameraP = NewCameraP;

    v2u TileSpan = V2U(17*3, 9*3);
    rect2 CameraBounds = RectCenterDim(V2(0, 0),
                                       TileSideInMeters.xy*V2i(TileSpan));

    world_pos MinChunkP = MapIntoChunkSpace(World, NewCameraP, V3(GetMin(CameraBounds), 0));
    world_pos MaxChunkP = MapIntoChunkSpace(World, NewCameraP, V3(GetMax(CameraBounds), 0));

    for(s32 ChunkY = MinChunkP.Chunk.y;
        ChunkY <= MaxChunkP.Chunk.y;
        ++ChunkY)
    {
        for(s32 ChunkX = MinChunkP.Chunk.x;
            ChunkX <= MaxChunkP.Chunk.x;
            ++ChunkX)
        {
            world_chunk *Chunk = GetWorldChunk(World, V3S(ChunkX, ChunkY, NewCameraP.Chunk.z));
            if(Chunk)
            {
                for(world_entity_block *Block = &Chunk->FirstBlock;
                    Block;
                    Block = Block->Next)
                {
                    for(u32 EntityIndex = 0;
                        EntityIndex < Block->EntityCount;
                        ++EntityIndex)
                    {
                        entity *Entity = GameState->Entities + Block->EntityIndex[EntityIndex];
                        v2 CameraSpaceP = GetCameraSpaceP(GameState, Entity);
                        if(IsInRect(CameraBounds, CameraSpaceP))
                        {
                        }
                    }
                }
            }
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->Permanent.Size);
    game_state *GameState = (game_state *)Memory->Permanent.Base; // STUDY(chowie): Cold-cast
    if(!GameState->IsInitialised)
    {
        // TODO(chowie): REMOVE! Have entity hashing remember this!
        AddEntity(GameState, EntityType_Null, 0);
        GameState->EntityCount = 1;
        GameState->Offset = V2(500.0f, 500.0f);

        // TODO(chowie): Initialise Audio State when there's proper
        // sound! Do I have to rearrange "audio -> game" to prevent
        // audio clipping at the beginning?
//        InitialiseAudioState(&GameState->AudioState);

        InitialiseArena(&GameState->WorldArena,
                        Memory->Permanent.Size - sizeof(game_state),
                        Memory->Permanent.Base + sizeof(game_state));

        f32 PixelsToMeters = 1.0f / 42.0f;
        v3 WorldChunkDimInMeters = V3(PixelsToMeters*256.0f, PixelsToMeters*256.0f, PixelsToMeters*256.0f);

        GameState->World = PushStruct(&GameState->WorldArena, world);
        world *World = GameState->World;
        InitialiseWorld(World, WorldChunkDimInMeters);

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

    //
    // World Init & Generation
    //

    // STUDY(chowie): Shortcut to avoid call with "&" all of the time
    render_group RenderGroup_ = BeginRenderGroup(RenderCommands);
    render_group *RenderGroup = &RenderGroup_;

    PushClear(RenderGroup, V4(0.25f, 0.25f, 0.25f, 0.0f));

    world *World = GameState->World;

    // RESOURCE: https://accuratesigns.net/the-sign-letter-height-visibility-chart/
    // NOTE: Average human height is 1.7m
    // TODO(chowie): Once have perspective transform & camera
    // viewport, remove everything but tilesideinmeters!
    v3 TileSideInMeters = V3(1.0f, 1.3f, 1.0f); // TODO(chowie): Change these to TotalTilingCubeEdge
    v3 TileSideInPixels = V3(80.0f, 95.0f, 80.0f); // TODO(chowie): REMOVE! This is more of a renderer concept, not the tiles
    v3 MetersToPixels = TileSideInPixels / TileSideInMeters; // TODO(chowie): REMOVE! This is more of a renderer concept, not the tiles

    //
    // Play World
    //

    f32 dt = Input->dtForFrame;
    for(u32 ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_player *ConPlayer = GameState->ControlledPlayer + ControllerIndex;
        if(ConPlayer->EntityIndex == 0)
        {
//            if(WasPressed(Controller->Start))
            {
                ZeroStruct(*ConPlayer);
                ConPlayer->EntityIndex = AddPlayer(GameState);
            }
        }
        else
        {
            ConPlayer->ddP = {};
            if(Controller->IsAnalog)
            {
                ConPlayer->ddP += Controller->StickAverage;
            }
            else
            {
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

#if 0
                // RESOURCE(HmH): https://guide.handmadehero.org/code/day211/#3368
                // STUDY(chowie): Numerical simulation, for proper ODE would need global t
                // RESOURCE(): https://web.archive.org/web/20210724053826/https://lolengine.net/blog/2015/05/03/damping-with-delta-time
                // TODO(chowie): You could dampen with lerp or pow/exp?

                ConPlayer->ddP = NOZ(ConPlayer->ddP);

                f32 Speed = 30.0f;
                ConPlayer->ddP *= Speed;

                f32 Drag = 8.0f;
                ConPlayer->ddP += -Drag*GameState->dP;

                // RESOURCE(HmH): https://guide.handmadehero.org/code/day043/#4686
                // RESOURCE: Thomas and Finney calculus for more!
                // NOTE(chowie): (1/2)at^2 + vt + p
                GameState->Offset += MetersToPixels.xy*(0.5f*ConPlayer->ddP*Square(dt) + GameState->dP*dt);
                GameState->dP += ConPlayer->ddP*dt;
#endif
            }
        }
    }

    //
    // NOTE(chowie): Simulate all entities
    //

    PushCircle(RenderGroup, V3(600, 600, 0), 50.0f, 4, V4(0.5f, 0.5f, 0.5f, 0.5f));

    // STUDY(chowie): Straight ahead loop
    for(u32 EntityIndex = 0;
        EntityIndex < GameState->EntityCount;
        ++EntityIndex)
    {
        entity *Entity = GameState->Entities + EntityIndex;

        //
        // "Physics"
        //

#if 1
        v3 ddP = {};
        if(Entity->Type == EntityType_Player)
        {
            for(u32 ControllerIndex = 0;
                ControllerIndex < ArrayCount(Input->Controllers);
                ++ControllerIndex)
            {
                controlled_player *ConPlayer = GameState->ControlledPlayer + ControllerIndex;
                ddP = V3(ConPlayer->ddP, 0);
            }
        }

        if(FlagSet(Entity->Flags, EntityFlag_Moveable))
        {
            MoveEntity(GameState, Entity, dt, ddP.xy, MetersToPixels.xy);
        }
#endif        

        //
        // Rendering
        //

        if(Entity->Type == EntityType_Player)
        {
            v4 HeroColour = V4(0.3f, 0.2f, 0.5f, 1);
            v2 PlayerDim = MetersToPixels.xy*V2(0.8f, 1.4f);
            PushRect(RenderGroup, V3(GameState->Offset, 0), PlayerDim, HeroColour);
        }
    }
}

// TODO(chowie): Reenable this once have proper spatial partitioning!
#if 0
    v3 Offset = {};

    /*
      RESOURCE(): https://www.livescience.com/50027-tessellation-tiling.html
      RESOURCE(): https://en.wikipedia.org/wiki/Honeycomb_(geometry)
      RESOURCE(shawcross): https://grahamshawcross.com/2012/10/12/periodic-and-non-periodic-tiling/
      RESOURCE(shawcross): https://grahamshawcross.com/2023/08/02/modules-and-proportions/
      RESOURCE(vanderlaan): https://domhansvanderlaan.nl/theory-practice/theory/the-plastic-number-series-8/
      RESOURCE(vanderlaan): https://domhansvanderlaan.nl/theory-practice/theory/the-plastic-number-ratio/
      (Naming based on Van Der Laan's Form Banks / Morphotheeks)
                                       ___________________
                                      /     /            /|
                                     /Dry  /            / | Rain comes from the north or east?
                                    /Blank/  Tile      /  |
                                   /     /            /   |
                                  /_____/____________/   /|
                                 /     /            /|  / |
      ____________________      /_____/____________/ | /  |
      |     |            |      |     |    Wet     | |/   |
      |Block|   Blank    |      |Block|   Blank    | /    |
      |_____|____________|      |_____|____________|/|    |
      |     |            |      |     |            | |    |
      |     |            |      |     |            | |Box |
      |     |            |      |     |            | |    |
      |     |            |      |     |   Wet      | |    |
      | Bar |   Slab     |      | Bar |   Slab     | |   /
      |     |            |      |     |            | |  /
      |     |            |      |     |            | | /
      |     |            |      |     |            | |/
      |     |            |      |     |            | /
      |_____|____________|      |_____|____________|/
       Tiling/Tesselation            Honeycomb

      Blank forms are transitional forms between Block|Slab (usually Bar too)
      Rods are derived forms; "a doubling of the authentic form".

      TODO(chowie): Slab|Bar = External; Block|Blank|Bar = Partition.

      I'm opting to use 'modular fixing' over 'location
      grids' to place modules/slabs.

      RESOURCE(): https://vanseodesign.com/web-design/grid-anatomy/
         Design Anatomy of a Modular Grid
      _______________________________________ ----o Flowline
      |     |            |     |            |
      |     |            |     |            | < Gutter
      |_____|____________|_____|____________| ----o
      |     |###############################|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#  Module/  |     |           #|
      |     |#   Slab    |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |_____|#___________|_____|___________#|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |_____|#___________|_____|___________#|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |     |#           |     |           #|
      |_____|###############################|
         ^                  ^
       Gutter           Spatial Zone

      TODO(chowie): Show structural grid alignment with "Witness" lines
      TODO(chowie): JP Modular/Heirarchical UI to view locations on a map?

      The use of uniform / semiregular tiling feels like a
      modular-proportional system hybrid.

      RESOURCE(vanderlaan): https://domhansvanderlaan.nl/theory-practice/theory/scale-i-inside-and-outside/
                        Domain
                    (Visual Field)
      _______________________________________
      |                                      |
      |                                      |
      |                                      |
      |                                      |
      |                 Court                |
      |             (Walking Space)          |
      |            _______________           |
      |            |    Cell     |           |
      |            | (Workspace) |           |
      |            |     ___     |           |
      |            |     |_|     |           |
      |            |             |           |
      |            |             |           |
      |            |_____________|           |
      |                                      |
      |                                      |
      |                                      |
      |                                      |
      |                                      |
      |______________________________________|
    */

    // TODO(chowie): Is it easier to make these a m3x3 matrix?
    // TODO(chowie): Express this as a ratio, clamp01?
    v2 SlabDim = MetersToPixels.xy*V2(0.8f, 1.3f);
    v2 BlockDim = MetersToPixels.xy*V2(0.3f, 0.3f);
    v2 BlankDim = V2(SlabDim.x, BlockDim.y);
    v2 BarDim = V2(BlockDim.x, SlabDim.y);
    v2 HoneycombDim = SlabDim + BlockDim;

    // TODO(chowie): Is it possible to make row y and column x?
    u32 TilesPerHeight = 4;
    u32 TilesPerWidth = 8;
    for(u32 Row = 0;
        Row < 2*TilesPerHeight;
        ++Row)
    {
        for(u32 Column = 0;
            Column < 2*TilesPerWidth;
            ++Column)
        {
            // TODO(chowie): Simplify b32x with DeMorgan's Law/Truth table?
            // TODO(chowie): Encode these using triangles numbers or something?
            // RESOURCE(amit): https://www.redblobgames.com/grids/edges/#corners
            v4 BaseColour = {};
            if(Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): Slab / Tile = Blue
                BaseColour = V4(0.3f, 0.5f, 0.8f, 1);
            }
            else if(!Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): Blank / S Edge = White
                BaseColour = V4(0.9f, 0.9f, 0.9f, 1);
            }
            else if(Odd(Row) && !Odd(Column))
            {
                // NOTE(chowie): Bar / W Edge = Yellow
                BaseColour = V4(0.9f, 0.9f, 0.5f, 1);
            }
            else
            {
                // NOTE(chowie): Block / Corner = Brown / Pastel
                BaseColour = V4(0.6f, 0.3f, 0.3f, 1);
            }

            // TODO(chowie): Tesselate for n-sized group tiles?
            v2 Min;
            Min.x = (f32)((Column - 1*Odd(Column)) / 2)*HoneycombDim.x + BlockDim.x*Odd(Column);
            Min.y = (f32)((Row - 1*Odd(Row)) / 2)*HoneycombDim.y + BlockDim.y*Odd(Row);

            v2 Max;
            Max.x = Min.x + BlankDim.x*Odd(Column) + BlockDim.x*!Odd(Column);
            Max.y = Min.y + BarDim.y*Odd(Row) + BlockDim.y*!Odd(Row);

            /*
            if(!Odd(Row))
            {
                Min.y = (Row / 2)*HoneycombDim.y;
                Max.y = Min.y + BlockDim.y;
            }
            else
            {
                Min.y = ((Row - 1) / 2)*HoneycombDim.y + BlockDim.y;
                Max.y = Min.y + BarDim.y;
            }

            if(!Odd(Column))
            {
                Min.x = (Column / 2)*HoneycombDim.x;
                Max.x = Min.x + BlockDim.x;
            }
            else
            {
                Min.x = ((Column - 1) / 2)*HoneycombDim.x + BlockDim.x;
                Max.x = Min.x + BlankDim.x;
            }
            */

            PushRect(RenderGroup, Offset, RectMinMax(Min, Max), BaseColour);
        }
    }
#endif

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->Permanent.Base; // TODO(chowie): Replace with an dedicated Audio.Base

    // TODO(chowie): Allow sample offsets here for more robust
    // platform options!
    // TODO(chowie): OutputPlayingSounds() Mixer
//    TestOutputWilwaDialTone(&GameState->AudioState, SoundBuffer);
}
