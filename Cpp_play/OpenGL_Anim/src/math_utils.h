#pragma once
#include <cmath>
#include <cstring>

constexpr float PI = 3.14159265358979323846f;

struct vec3 {
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3 operator+(const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    vec3 operator-(const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    vec3& operator+=(const vec3& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    vec3 normalized() const { float l=length(); return l>0 ? vec3(x/l,y/l,z/l) : vec3(); }
};

inline vec3 cross(const vec3& a, const vec3& b) {
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

inline float dot(const vec3& a, const vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

struct mat4 {
    float m[16];

    mat4() { std::memset(m, 0, sizeof(m)); }

    static mat4 identity() {
        mat4 r;
        r.m[0]=1; r.m[5]=1; r.m[10]=1; r.m[15]=1;
        return r;
    }

    static mat4 translate(float x, float y, float z) {
        mat4 r = identity();
        r.m[12]=x; r.m[13]=y; r.m[14]=z;
        return r;
    }

    static mat4 scale(float sx, float sy, float sz) {
        mat4 r;
        r.m[0]=sx; r.m[5]=sy; r.m[10]=sz; r.m[15]=1;
        return r;
    }

    static mat4 rotate_x(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        mat4 r = identity();
        r.m[5]=c;  r.m[6]=s;
        r.m[9]=-s; r.m[10]=c;
        return r;
    }

    static mat4 rotate_y(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        mat4 r = identity();
        r.m[0]=c;  r.m[2]=-s;
        r.m[8]=s;  r.m[10]=c;
        return r;
    }

    static mat4 rotate_z(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        mat4 r = identity();
        r.m[0]=c;  r.m[1]=s;
        r.m[4]=-s; r.m[5]=c;
        return r;
    }

    static mat4 perspective(float fov_deg, float aspect, float near_plane, float far_plane) {
        float tan_half = std::tan(fov_deg * PI / 360.0f);
        mat4 r;
        r.m[0] = 1.0f / (aspect * tan_half);
        r.m[5] = 1.0f / tan_half;
        r.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
        return r;
    }

    static mat4 look_at(const vec3& eye, const vec3& center, const vec3& up) {
        vec3 f = (center - eye).normalized();
        vec3 s = cross(f, up).normalized();
        vec3 u = cross(s, f);

        mat4 r = identity();
        r.m[0]=s.x;  r.m[4]=s.y;  r.m[8]=s.z;
        r.m[1]=u.x;  r.m[5]=u.y;  r.m[9]=u.z;
        r.m[2]=-f.x; r.m[6]=-f.y; r.m[10]=-f.z;
        r.m[12]= -dot(s, eye);
        r.m[13]= -dot(u, eye);
        r.m[14]= dot(f, eye);
        return r;
    }

    mat4 operator*(const mat4& b) const {
        mat4 r;
        for (int c = 0; c < 4; c++)
            for (int row = 0; row < 4; row++) {
                float sum = 0;
                for (int k = 0; k < 4; k++)
                    sum += m[k*4+row] * b.m[c*4+k];
                r.m[c*4+row] = sum;
            }
        return r;
    }

    const float* ptr() const { return m; }
};
