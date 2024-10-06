#ifndef RING_H
#define RING_H

#include <vector>
#include <glm/glm.hpp>

class Ring {
public:
    Ring(float innerRadius, float outerRadius, unsigned int sectorCount);
    ~Ring();

    std::vector<float> getVertices() const;
    std::vector<unsigned int> getIndices() const;

private:
    float innerRadius;
    float outerRadius;
    unsigned int sectorCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void generateRing();
};

#endif
