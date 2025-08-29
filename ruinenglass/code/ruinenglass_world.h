#if !defined(RUINENGLASS_WORLD_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): It seems like we have store ChunkX/Y/Z with each
// entity because even though the sim region gather doesn't need it at
// first, and we could get by without it, entity references pull in
// references without going through their world_chunk, thus still need
// to know the chunk X,Y,Z.
struct world_pos
{
    v3s Chunk; // NOTE(chowie): Absolute pos
    v3 Offset_; // NOTE(chowie): Relative Offsets from chunk center
};

// NOTE(chowie): Invert reference from world, but most of the
// time you're not actively accessing say walls
// STUDY(chowie): Could make chunks and allow multiple chunks per X,Y,Z (hash bucket)
#define MaxEntitiesPerBlock 16
struct world_entity_block
{
    u32 EntityCount; // NOTE(chowie): Fill level to max entities per block
    u32 EntityIndex[MaxEntitiesPerBlock];
    world_entity_block *Next;
};

// NOTE(chowie): Actual 3D chunks, not like Minecraft's top-to-bottom (16x128x16)
struct world_chunk
{
    v3s Chunk;
    // STUDY(chowie): To fill and move block, this must be a pointer
    // to tell where the last one was, or you'll have to keep a second
    // pointer. You could instead mix this with the hash, walk and
    // pull from hash. But that's not preferable.
    // TODO(chowie): If querying is more frequent than reinsertion
    // (not done at runtime), don't block-copy. Profile this!
    world_entity_block FirstBlock;
    world_chunk *NextInHash; // STUDY(chowie): External Chaining
};

// IMPORTANT(chowie): What's stored in the world is persistent data
// IMPORTANT(chowie): Origin is the world's center, at [0, 0, 0]
// NOTE(chowie): Origin nice for perspective transform and relativeness
struct world
{
    v3 ChunkDimInMeters;

    world_entity_block *FirstFree;

    // TODO(chowie): ChunkHash are pointers if entity blocks continue
    // to be stored en mass directly in chunk. Check for performance
    // as a pointer? Don't want the first lookup to be a pointer.
    // NOTE(chowie): At the moment, this must be a power of two!
    // NOTE(chowie): External Chaining
    world_chunk ChunkHash[HashSizePow2];
};

/*
  RESOURCE(eisenwave): https://eisenwave.github.io/voxel-compression-docs/
  RESOURCE(geier): https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
  RESOURCE(): https://www.reddit.com/r/VoxelGameDev/comments/f8wv5q/minecraft_style_worlds_best_way_to_store_voxel/
  RESOURCE(): https://www.reddit.com/r/VoxelGameDev/comments/15dzuus/question_about_octreesother_data_structures_for_a/
  RESOURCE(0fps): https://0fps.net/2012/01/14/an-analysis-of-minecraft-like-engines/
  RESOURCE(tom forsyth): Search "voxel"; Sparse-world storage formats -
  https://tomforsyth1000.github.io/blog.wiki.html
  RESOURCE(): https://www.reddit.com/r/VoxelGameDev/comments/nijezf/downsides_of_using_dags_instead_of_octrees/

  STUDY(onix242): There's been lots of discussion about sparse-world
  voxel storage / spatial partioning, namely between octrees and hash
  tables. The former being most popular, but is that ideal for my own
  voxel prototype engine?

  Octrees (SVO and DAGs) has certain properties appealing for some
  engines - given their popularity, for some compelling reasons:
  - "In-built" multiple LODs (more necessary for 3D engines compared
    to 2D), to achieve a highly-detailed look closeby and to see
    distant horizons. Ideal for smaller voxels sizes. Looking through
    a camera in a 3D world is non-balancing (good for octrees).
  - A heirachy of different update frequencies at certain sections,
    "leaves" of the world. (While a good hash would try to avoid any
    spatial locality).
  - Easier batching of raytracing/raymarching/marching cubes for lighting
  - Fustrum culling pairs well with a heirarchical partition of the world

  Octrees has caveats that I believe is important for this engine:
  - Slow with updating the "leaves" when not within its bounds,
    especially when it lies across multiple. This is similar to
    Mortons encoding!
  - Top-most node must encompass the whole world. Bigger worlds = more
    nodes = pointer chasing. Updates best near the centre of the
    world, worse for larger worlds.
  - Often people encode identical geometry directly into octrees, with
    a few shared attributes (position, UV, colours etc). But
    optimisations often involve limiting these attributes'
    resolution. While disc-loaded textures paired with octrees has
    little research interest since you can achieve the "same" with
    smaller voxels.
  - As part of voxel animation optimisations e.g. trees/grass moving
    in the wind or water flowing. Some restrict movement by the
    underlying grid. With smaller voxels/grid, this is less noticable
    to the eye.
  - Storing and compressing octrees directly to disc may be the most
    convienient format, but certainly other formats such as RLE are
    much simpler and beneficial for long running empty voxel data.
  NOTE(chowie): Technically octrees can be a "flatter" linear hash.
  Alternatively, you could do a mixed approach. A flat hashed octree
  with "bricks" (dense arrays). But I'm not considering either here.

  RESOURCE(lin): https://voxely.net/blog/the-perfect-voxel-engine/
  Ultimately, according to Lin, octrees are "only _acceptable_ at
  storage and rendering"; not even that good, and doesn't consider
  these important aspects for this engine:
  - Collision detection
  - Global illumination
  - Pathfinding
  - Per-voxel attributes (besides albedo and normals)
  - Dynamic objects (bouncy/physics-y).

  RESOURCE(kapoulkine): https://zeux.io/2017/03/27/voxel-terrain-storage/
  * tags difficulty
  Octrees stands against these architectural intentions for this engine:
  - Heavy pathfinding, coop pathfinding (David Silverman)
  - Frequent non-static entity updates, ~100 for pathfinding and up to 
    500 for copy-by-movement "stretching".
  - Updates (by "stretching") that may be potentially larger than a
    "chunk". Chunks are typically 16^3 numbers chosen in part for
    physics update see Kapoulkine of Roblox, or 32^3 see Aflockofmeese
    for modern hardware = 27-bits of chunks.
  * Non-uniform grid (basically a lot more maths)
      = To use an octree may instead require a k-D tree for updates
      = Harder for typical raytracing assumptions
      = View-frustums slightly harder
  - Disc-loaded textures for voxels
  - No realistic HDR lighting (I don't see myself as a good graphics programmer)
  - Contrary to lots of other voxel engines, doesn't need to support
    direct player-controlled building and destruction (diggging into
    solid bodies of voxels). Instead voxels models "stretches"/"recedes"
    controlled by the game/engine. (Ignores any editor considerations)
  - Voxels to more expressively represent "stretching" as shearing shapes,
    granularly _moving_ to its total bounds (rather than splitting
    blocks as a half/quarter) with entities moving along the surface.

  Thus, I plan to use hash tables for multiple other benefits:
  - Quick/flat access for _neighbourhood access_ being the most common
    op. While octrees updates based on the number of "children"
    heirarchically deep.
  - No "edge" to the world unlike octrees, nor do they care about how
    far is the world is from one side to the other. For a world where
    the _large voxel memory dramatically switches spatially_ when many
    voxel models "stretches"/"recedes". A lower memory consumption
    during the low times could be beneficial.
  - Coordinates are flexible. Doesn't require the voxel to conform to
    a strict uniform placement. Allows for off-grid (dis)placement.
    However, you do lose specific compression for uniform grids.
  - Using octrees vs hash tables steers towards the size of the voxel
    itself; octrees are more attactive the smaller the voxel. For
    larger voxels faces similar to Minecraft's blocks, hash tables
    seems to have better performance.
  - Far quicker and easier to optimise (good hash function is all its
    needed), less on optimising the data structure unlike octrees.

  In addition to hash tables, I plan to also use:
  - Sim regions, can mimic the effect of different update frequencies.
    But otherwise has no relation to spatial partitioning space (just
    so happens that people use BSP for both). In fact you don't even
    need to chunk the world! You need some sim policy and way to
    convert entities into world space from sim space (with packing /
    unpacking). It does have the added benefit of not requiring to be
    world aligned.
  - RLE Compression + LZ4 NOTE(chowie): Doing anymore is probably
    overkill according to Kapoulkine. However, an interesting
    exploration is doing Morton encoding compression on x,y,z coords
    and neighbourhood access while still compressed. For larger
    worlds, to conserve memory, you would need to test if compression
    and recompression timings outweighs accessing through Morton's
    compression using PEXT.
    NOTE(chowie): Voxels and LOD is interesting because small holes
    can be covered up from a distance.
*/

#define RUINENGLASS_WORLD_H
#endif
