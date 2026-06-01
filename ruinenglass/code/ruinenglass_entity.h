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

#define MAX_ENTITY_ID U32Max
// TODO(chowie): Use stack (not queue) to reuse the most recently used
// free slot. Generate entity ids (1 to max) at startup. Removing an
// entity increment generation and push it back on stack. Pass
// entities around as indices to not worry about deletion. A risk of
// deletion needs a persistent reference, to validate compare value
// (array index) matches with current generation slot. If overflows,
// ban index and never use until safe e.g. new level resets everything
// NOTE(chowie): Generation is like a goblin holding a jar is
// defeated, an unrelated chicken takes fills it's spot in memory to
// not confuse the jar is held by the chicken. In other words,
// protects against deleted handle points to a newly occupying entity.
// IMPORTANT(chowie): Reserve 0 for null entity
struct entity_id
{
    u32 Value;
    u16 Generation; // NOTE(chowie): Entropy, array index incremented when deleted.
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

    // TODO(chowie): Eventually replace these with brains!

    //
    // NOTE(chowie): Everything below hasn't been worked out yet
    //

    entity_type Type;
    u32 Flags;

    u32 EntityID; // TODO(chowie): Change to using actual struct above!
    world_pos P; // TODO(chowie): Change P back to v3!
    v2 dP;
//    v2 ddP;
};

/*
inline b32x
IsEntityFlagSet(entity *Entity, u32 Flag)
{
    b32x Result = FlagSet(Entity->Flags, Flag);
    return(Result);
}

inline void
AddEntityFlags(entity *Entity, u32 Flag)
{
    AddFlag(Entity->Flags, Flag);
}

// STUDY(chowie): Passes all other flags except that flag. Thus,
// removing that flag.
inline void
ClearEntityFlags(entity *Entity, u32 Flag)
{
    ClearFlag(Entity->Flags, Flag);
}

inline void
ToggleEntityFlags(entity *Entity, b32x Test, u32 Flag)
{
    (Test) ? AddEntityFlags(Entity, Flag) : ClearEntityFlags(Entity, Flag);
}
*/

#define RUINENGLASS_ENTITY_H
#endif
