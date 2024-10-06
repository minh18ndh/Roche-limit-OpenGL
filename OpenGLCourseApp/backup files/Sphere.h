
#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glm/glm.hpp>

class Sphere {
public:
    Sphere(float radius, unsigned int sectorCount, unsigned int stackCount);
    ~Sphere();

    std::vector<float> getVertices() const;
    std::vector<unsigned int> getIndices() const;

private:
    float radius;
    unsigned int sectorCount;
    unsigned int stackCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void generateSphere();
};

#endif
