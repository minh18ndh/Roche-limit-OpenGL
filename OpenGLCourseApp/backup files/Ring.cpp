// Ring.cpp
#include "Ring.h"

Ring::Ring(float innerRadius, float outerRadius, unsigned int sectorCount)
    : innerRadius(innerRadius), outerRadius(outerRadius), sectorCount(sectorCount) {
    generateRing();
}

Ring::~Ring() {}

void Ring::generateRing() {
    const float PI = 3.14159265359f;
    float x, y, z, s, t;
    float sectorStep = 2 * PI / sectorCount;
    float sectorAngle;

    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        x = cosf(sectorAngle);
        y = sinf(sectorAngle);

        // Outer vertices
        vertices.push_back(x * outerRadius);
        vertices.push_back(0.0f); // y
        vertices.push_back(y * outerRadius);
        vertices.push_back((x + 1.0f) / 2.0f); // u coordinate
        vertices.push_back((y + 1.0f) / 2.0f); // v coordinate
        vertices.push_back(x); // Normal x
        vertices.push_back(0.0f); // Normal y
        vertices.push_back(y); // Normal z

        // Inner vertices
        vertices.push_back(x * innerRadius);
        vertices.push_back(0.0f); // y
        vertices.push_back(y * innerRadius);
        vertices.push_back((x + 1.0f) / 2.0f); // u coordinate
        vertices.push_back((y + 1.0f) / 2.0f); // v coordinate
        vertices.push_back(x); // Normal x
        vertices.push_back(0.0f); // Normal y
        vertices.push_back(y); // Normal z
    }

    for (int i = 0; i < sectorCount; ++i) {
        int k1 = i * 2;
        int k2 = k1 + 1;
        int k3 = (i + 1) * 2;
        int k4 = k3 + 1;

        // Outer triangle
        indices.push_back(k1);
        indices.push_back(k3);
        indices.push_back(k2);

        // Inner triangle
        indices.push_back(k2);
        indices.push_back(k3);
        indices.push_back(k4);
    }
}

std::vector<float> Ring::getVertices() const {
    return vertices;
}

std::vector<unsigned int> Ring::getIndices() const {
    return indices;
}
