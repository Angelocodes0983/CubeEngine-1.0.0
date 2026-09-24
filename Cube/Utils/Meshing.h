#pragma once
#include "Dependencies.h"

namespace Cube::Meshing
{
    struct Vertex {
        glm::vec3 pos;
    };

    struct Mesh {
        std::vector<glm::vec3> vertices;
        std::vector<uint16_t> indices;
    };

    Mesh boxMesh(glm::vec3 worldSize, const std::vector<unsigned char>& voxels);

    Mesh meshBox(glm::vec3 pos, glm::ivec3 size);

    void createQuadX(glm::vec3 worldSize, const std::vector<unsigned char>& voxels, float layer, bool direction, Mesh& m);
    void createQuadY(glm::vec3 worldSize, const std::vector<unsigned char>& voxels, float layer, bool direction, Mesh& m);
    void createQuadZ(glm::vec3 worldSize, const std::vector<unsigned char>& voxels, float layer, bool direction, Mesh& m);
    Mesh greedyMesh(glm::vec3 worldSize, const std::vector<unsigned char>& voxels);


    Mesh planeMesh(int yheight);

    bool isVisited(glm::vec3 pos, glm::vec3 worldSize, const std::vector<unsigned char>& visitedCells);
    bool isSolid(glm::vec3 pos, glm::vec3 worldSize, const std::vector<unsigned char>& voxels);

}