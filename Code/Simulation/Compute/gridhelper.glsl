layout(std140, binding = 0) uniform GridParams {
    vec4  origin_cell;   // xyz = grid origin, w = cell size
    ivec4 dims_count;    // xyz = cells per axis, w = numCells
    uint count;
} grid;

#define GRID_ORIGIN (grid.origin_cell.xyz)
#define CELL_SIZE   (grid.origin_cell.w)
#define GRID_DIMS   (grid.dims_count.xyz)
#define NUM_CELLS   (uint(grid.dims_count.w))
#define COUNT (grid.count)

layout(std430, binding = 7)  buffer CellIdBuf    { uint cell_id[];    };  // N
layout(std430, binding = 8)  buffer RankBuf      { uint rank[];       };  // N
layout(std430, binding = 9)  buffer CellCountBuf { uint cell_count[]; };  // numCells
layout(std430, binding = 10) buffer CellStartBuf { uint cell_start[]; };  // numCells + 1
layout(std430, binding = 11) buffer SortedBuf    { uint sorted_ids[]; };  // N

ivec3 cellCoord(vec3 p) {
    ivec3 c = ivec3(floor((p - GRID_ORIGIN) / CELL_SIZE));
    return clamp(c, ivec3(0), GRID_DIMS - 1);
}

uint cellIndex(ivec3 c) {
    return uint(c.x + c.y * GRID_DIMS.x + c.z * GRID_DIMS.x * GRID_DIMS.y);
}
