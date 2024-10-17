#if !defined(RUINENGLASS_WORLD_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

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

#define RUINENGLASS_WORLD_H
#endif
