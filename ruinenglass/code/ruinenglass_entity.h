#if !defined(RUINENGLASS_ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

enum entity_type
{
    EntityType_Null,

    EntityType_Player,
    EntityType_Wall,
    EntityType_Space,
};

enum entity_flags
{
    EntityFlag_Collides = BitSet(0),
    EntityFlag_Deleted = BitSet(1),
    EntityFlag_Moveable = BitSet(2), // TODO(chowie): REMOVE! Once have brains!
//    EntityFlag_Traversable = BitSet(3), // TODO(chowie): Might be interesting for optimisation
};

struct entity;

// IMPORTANT(chowie): Reserve 0 for null entity
struct entity_id
{
    u32 Value;
};
union entity_ref
{
    entity *Ptr;
    entity_id ID;
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

    v2 P;
    v2 dP;
//    v2 ddP;
};

/*
inline b32x
IsEntityFlagSet(entity *Entity, u32 Flag)
{
    b32x Result = Entity->Flags & Flag;
    return(Result);
}

inline void
AddEntityFlags(entity *Entity, u32 Flag)
{
    Entity->Flags |= Flag;
}

// STUDY(chowie): Passes all other flags except that flag. Thus,
// removing that flag.
inline void
ClearEntityFlags(entity *Entity, u32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void
ToggleEntityFlags(entity *Entity, b32x Test, u32 Flag)
{
    (Test) ? AddEntityFlags(Entity, Flag) : ClearEntityFlags(Entity, Flag);
}
*/

#define RUINENGLASS_ENTITY_H
#endif
