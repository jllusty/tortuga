#ifndef CAMERA_HPP

#include "vec3.hpp"
#include "input.hpp"

class Camera {
    const float speed = 2.5f;
    const float turn = 1.5f;
public:
    vec3 position;
    vec3 forward;
    vec3 up;
    vec3 right;
    
    Camera() {
        position = vec3(0.f,0.f,0.f);
        forward = vec3(1.f,0.f,0.f);
        up = vec3(0.f,0.f,1.f);
        right = normalize(cross(forward,up));
    }
    Camera(vec3 position, vec3 forward, vec3 up)
        : position(position), forward(forward),
          up(up) 
    {
        right = normalize(cross(forward,up));
    }

    void update(float dt) {
        // keep these normalized, for good luck
        forward = normalize(forward);
        up = normalize(up);
        right = normalize(right);

        // WASD -> update position
        // W/S: forward/backward
        if(input::getKey(input::keyid::W) == 1 && input::getKey(input::keyid::S) == 1) {
            // do nothing
        }
        else if(input::getKey(input::keyid::W) == 1) {
            position += normalize(forward)*speed*dt;
        }
        else if(input::getKey(input::keyid::S) == 1) {
            position -= normalize(forward)*speed*dt;
        }
        // A/D: strafe left/right
        if(input::getKey(input::keyid::A) == 1 && input::getKey(input::keyid::D) == 1) {
            // do nothing
        }
        else if(input::getKey(input::keyid::D) == 1) {
            position += normalize(right)*speed*dt;
        }
        else if(input::getKey(input::keyid::A) == 1) {
            position -= normalize(right)*speed*dt;
        }

        // HJKL -> update orientation
        if(input::getKey(input::keyid::H) == 1 && input::getKey(input::keyid::L) == 1) {
            // do nothing
        }
        else if(input::getKey(input::keyid::H) == 1) {
            rotate(forward, up, dt*turn);
            rotate(right, up, dt*turn);
        }
        else if(input::getKey(input::keyid::L) == 1) {
            rotate(forward, up, -dt*turn);
            rotate(right, up, -dt*turn);
        }
        // J/K: turn up and down
        if(input::getKey(input::keyid::J) == 1 && input::getKey(input::keyid::K) == 1) {
            // do nothing
        }
        else if(input::getKey(input::keyid::J) == 1) {
            rotate(forward, right, -dt*turn);
        }
        else if(input::getKey(input::keyid::K) == 1) {
            rotate(forward, right, dt*turn);
        }

        // Q/E: elevation
        if(input::getKey(input::keyid::Q) == 1 && input::getKey(input::keyid::E) == 1) {
            // do nothing
        }
        else if(input::getKey(input::keyid::Q) == 1) {
            position -= normalize(up)*dt*speed;
        }
        else if(input::getKey(input::keyid::E) == 1) {
            position += normalize(up)*dt*speed;
        }
    }
};

#define CAMERA_HPP
#endif