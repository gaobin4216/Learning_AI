#pragma once
#include <vector>
#include "glad.h"
#include "math_utils.h"

struct Vertex {
    vec3 position;
    vec3 normal;
};

class Mesh {
public:
    Mesh() : vao_(0), vbo_(0), ebo_(0), count_(0) {}
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;

    void draw() const;

    static Mesh cube();
    static Mesh sphere(int slices = 32, int stacks = 16);
    static Mesh torus(int slices = 48, int stacks = 16, float R = 1.0f, float r = 0.35f);

private:
    GLuint vao_, vbo_, ebo_;
    GLsizei count_;
};
