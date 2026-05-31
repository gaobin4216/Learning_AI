#include "shader.h"
#include <stdexcept>
#include <vector>

Shader::Shader(const std::string& vert_src, const std::string& frag_src) {
    GLuint vert = compile(GL_VERTEX_SHADER, vert_src);
    GLuint frag = compile(GL_FRAGMENT_SHADER, frag_src);

    id_ = glCreateProgram();
    glAttachShader(id_, vert);
    glAttachShader(id_, frag);
    glLinkProgram(id_);

    GLint success;
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(id_, 512, nullptr, log);
        throw std::runtime_error(std::string("Shader link error: ") + log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (id_) glDeleteProgram(id_);
}

void Shader::use() const { glUseProgram(id_); }

void Shader::set_float(const std::string& name, float v) const {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), v);
}

void Shader::set_vec3(const std::string& name, const vec3& v) const {
    glUniform3f(glGetUniformLocation(id_, name.c_str()), v.x, v.y, v.z);
}

void Shader::set_mat4(const std::string& name, const mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, m.ptr());
}

GLuint Shader::compile(GLenum type, const std::string& src) {
    GLuint shader = glCreateShader(type);
    const char* s = src.c_str();
    glShaderSource(shader, 1, &s, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        throw std::runtime_error(std::string("Shader compile error: ") + log);
    }
    return shader;
}
