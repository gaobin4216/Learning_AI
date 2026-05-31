#include "mesh.h"
#include <cmath>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vao_(0), vbo_(0), ebo_(0), count_(static_cast<GLsizei>(indices.size()))
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
}

Mesh::Mesh(Mesh&& o) noexcept : vao_(o.vao_), vbo_(o.vbo_), ebo_(o.ebo_), count_(o.count_) {
    o.vao_ = o.vbo_ = o.ebo_ = 0; o.count_ = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (ebo_) glDeleteBuffers(1, &ebo_);
        vao_=o.vao_; vbo_=o.vbo_; ebo_=o.ebo_; count_=o.count_;
        o.vao_=o.vbo_=o.ebo_=0; o.count_=0;
    }
    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ---- Cube ----
Mesh Mesh::cube() {
    std::vector<Vertex> v = {
        // front
        {{-1,-1, 1}, { 0, 0, 1}}, {{ 1,-1, 1}, { 0, 0, 1}},
        {{ 1, 1, 1}, { 0, 0, 1}}, {{-1, 1, 1}, { 0, 0, 1}},
        // back
        {{ 1,-1,-1}, { 0, 0,-1}}, {{-1,-1,-1}, { 0, 0,-1}},
        {{-1, 1,-1}, { 0, 0,-1}}, {{ 1, 1,-1}, { 0, 0,-1}},
        // top
        {{-1, 1, 1}, { 0, 1, 0}}, {{ 1, 1, 1}, { 0, 1, 0}},
        {{ 1, 1,-1}, { 0, 1, 0}}, {{-1, 1,-1}, { 0, 1, 0}},
        // bottom
        {{-1,-1,-1}, { 0,-1, 0}}, {{ 1,-1,-1}, { 0,-1, 0}},
        {{ 1,-1, 1}, { 0,-1, 0}}, {{-1,-1, 1}, { 0,-1, 0}},
        // right
        {{ 1,-1, 1}, { 1, 0, 0}}, {{ 1,-1,-1}, { 1, 0, 0}},
        {{ 1, 1,-1}, { 1, 0, 0}}, {{ 1, 1, 1}, { 1, 0, 0}},
        // left
        {{-1,-1,-1}, {-1, 0, 0}}, {{-1,-1, 1}, {-1, 0, 0}},
        {{-1, 1, 1}, {-1, 0, 0}}, {{-1, 1,-1}, {-1, 0, 0}},
    };
    std::vector<unsigned int> idx;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned int base = i * 4;
        idx.insert(idx.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    return Mesh(v, idx);
}

// ---- Sphere ----
Mesh Mesh::sphere(int slices, int stacks) {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= stacks; i++) {
        float phi = PI * i / stacks;
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * PI * j / slices;
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            v.push_back({{x, y, z}, {x, y, z}});
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned int a = i * (slices + 1) + j;
            unsigned int b = a + slices + 1;
            idx.insert(idx.end(), {a, b, a+1, a+1, b, b+1});
        }
    }
    return Mesh(v, idx);
}

// ---- Torus ----
Mesh Mesh::torus(int slices, int stacks, float R, float r) {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= stacks; i++) {
        float phi = 2.0f * PI * i / stacks;
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * PI * j / slices;
            float x = (R + r * std::cos(phi)) * std::cos(theta);
            float y = r * std::sin(phi);
            float z = (R + r * std::cos(phi)) * std::sin(theta);
            // normal
            float nx = std::cos(phi) * std::cos(theta);
            float ny = std::sin(phi);
            float nz = std::cos(phi) * std::sin(theta);
            v.push_back({{x, y, z}, {nx, ny, nz}});
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned int a = i * (slices + 1) + j;
            unsigned int b = a + slices + 1;
            idx.insert(idx.end(), {a, b, a+1, a+1, b, b+1});
        }
    }
    return Mesh(v, idx);
}
