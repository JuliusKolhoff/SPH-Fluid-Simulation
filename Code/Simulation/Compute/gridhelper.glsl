layout(std140, binding = 0) uniform GridParams {
    vec4  origin_cell;   // xyz = grid origin, w = cell size
    ivec4 dims_count;    // xyz = cells per axis, w = numCells
    uint count;
} grid;

#define GRID_ORIGIN (grid.origin_cell.xyz)
#define CELL_SIZE   (grid.origin_cell.w)
#define GRID_DIMS   (grid.dims_count.xyz)
#define NUM_CELLS   (uint(grid.dims_count.w))

#ifndef COUNT
#define COUNT (grid.count)
#endif

layout(std430, binding = 7)  buffer CellIdBuf    { uint cell_id[];    };  // N
layout(std430, binding = 8)  buffer RankBuf      { uint rank[];       };  // N
layout(std430, binding = 9)  buffer CellCountBuf { uint cell_count[]; };  // numCells
layout(std430, binding = 10) buffer CellStartBuf { uint cell_start[]; };  // numCells + 1
layout(std430, binding = 11) buffer SortedBuf    { uint sorted_ids[]; };  // N

//i must be defined before loop
#define NEIGHBOR_LOOP_BEGIN(P, J) {  vec3 P = pos[i].xyz; ivec3 _c = cellCoord(P); \
  for (int _dz=-1; _dz<=1; _dz++) { int _z=_c.z+_dz; if (_z<0||_z>=GRID_DIMS.z) continue; \
  for (int _dy=-1; _dy<=1; _dy++) { int _y=_c.y+_dy; if (_y<0||_y>=GRID_DIMS.y) continue; \
    uint _s0 = cell_start[cellIndex(ivec3(max(_c.x-1,0), _y, _z))]; \
    uint _s1 = cell_start[cellIndex(ivec3(min(_c.x+1,GRID_DIMS.x-1), _y, _z)) + 1u]; \
    for (uint _s=_s0; _s<_s1; _s++) { uint J = sorted_ids[_s]; \
    vec3 d  = P - pos[J].xyz; float r2 = dot(d, d);if (r2 > H2){ continue;}
#define NEIGHBOR_LOOP_END }}}}






ivec3 cellCoord(vec3 p) {
    ivec3 c = ivec3(floor((p - GRID_ORIGIN) / CELL_SIZE));
    return clamp(c, ivec3(0), GRID_DIMS - 1);
}

uint cellIndex(ivec3 c) {
    return uint(c.x + c.y * GRID_DIMS.x + c.z * GRID_DIMS.x * GRID_DIMS.y);
}
