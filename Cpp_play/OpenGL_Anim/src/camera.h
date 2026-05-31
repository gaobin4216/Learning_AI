#pragma once
#include "math_utils.h"

class Camera {
public:
    Camera(float distance = 6.0f, float speed = 0.3f)
        : distance_(distance), angle_(0), pitch_(0.3f), speed_(speed) {}

    void update(float dt) { angle_ += speed_ * dt; }

    vec3 position() const {
        return {
            distance_ * std::cos(pitch_) * std::sin(angle_),
            distance_ * std::sin(pitch_),
            distance_ * std::cos(pitch_) * std::cos(angle_)
        };
    }

    mat4 view_matrix() const {
        vec3 eye = position();
        return mat4::look_at(eye, {0,0,0}, {0,1,0});
    }

    void set_distance(float d) { distance_ = d; }
    void set_pitch(float p) { pitch_ = p; }

private:
    float distance_, angle_, pitch_, speed_;
};
