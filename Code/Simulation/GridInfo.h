#pragma once
#include <glm/glm.hpp>
struct GridInfo {
    glm::ivec3 dims;
    glm::vec3  origin;
    float      cellSize;
    uint32_t   numCells;
};

inline GridInfo makeGrid(const glm::vec3& boundsMin, const glm::vec3& boundsMax, float h)
{
    GridInfo g;
    g.cellSize = h;
    g.origin = boundsMin;
    glm::vec3 extent = boundsMax - boundsMin;
    g.dims = glm::max(glm::ivec3(glm::ceil(extent / h)), glm::ivec3(1));
    g.numCells = (uint32_t)g.dims.x * g.dims.y * g.dims.z;
    return g;
}