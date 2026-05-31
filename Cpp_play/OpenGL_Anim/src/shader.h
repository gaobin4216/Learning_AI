#pragma once
#include <string>
#include "glad.h"
#include "math_utils.h"

class Shader {
public:
    Shader() : id_(0) {}
    Shader(const std::string& vert_src, const std::string& frag_src);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& o) noexcept : id_(o.id_) { o.id_ = 0; }
    Shader& operator=(Shader&& o) noexcept { id_=o.id_; o.id_=0; return *this; }

    void use() const;
    void set_float(const std::string& name, float v) const;
    void set_vec3(const std::string& name, const vec3& v) const;
    void set_mat4(const std::string& name, const mat4& m) const;

    GLuint id() const { return id_; }

private:
    GLuint compile(GLenum type, const std::string& src);
    GLuint id_;
};
