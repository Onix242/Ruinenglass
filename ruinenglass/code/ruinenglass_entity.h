#if !defined(RUINENGLASS_ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

struct move_spec
{
    b32x UnitMaxAccelVector;
    r32 Speed;
    r32 Drag;
};

enum entity_type
{
    EntityType_Null,

    EntityType_Space,
    EntityType_Player,
    EntityType_Wall,
};

enum entity_flags
{
    EntityFlag_Collides = BitSet(0),
    EntityFlag_Moveable = BitSet(1),
    EntityFlag_Deleted = BitSet(2),
//    EntityFlag_Traversable = BitSet(3), // TODO(chowie): Might be interesting for optimisation
};

struct entity
{
    //
    // NOTE(chowie): Thought-out things
    //

    //
    // NOTE(chowie): Everything below hasn't been worked out yet
    //

    entity_type Type; // TODO(chowie): Eventually replace this with brains!
    u32 Flags;
    move_spec MoveSpec;
};

inline b32x
IsSet(entity *Entity, u32 Flag)
{
    b32x Result = Entity->Flags & Flag;
    return(Result);
}

inline void
AddFlags(entity *Entity, u32 Flag)
{
    Entity->Flags |= Flag;
}

// STUDY(chowie): Passes all other flags except that flag. Thus,
// removing that flag.
inline void
ClearFlags(entity *Entity, u32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void
ToggleFlags(entity *Entity, b32x Test, u32 Flag)
{
    (Test) ? AddFlags(Entity, Flag) : ClearFlags(Entity, Flag);
}

#define RUINENGLASS_ENTITY_H
#endif
