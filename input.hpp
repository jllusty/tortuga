#ifndef INPUT_HPP

#include "include/GLFW/glfw3.h"

#include <array>

namespace input {
    enum class keyid : int {
        W=0, A=1, S=2, D=3,
        H=4, J=5, K=6, L=7,
        Q=8, E=9
    };
    int keys[26]{};

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        #define UPDATE_KEY(GKEY,TKEY) \
        if(key == GKEY && action == GLFW_PRESS) {\
            keys[static_cast<int>( TKEY )] = 1;\
        }\
        else if(key == GKEY && action == GLFW_RELEASE) {\
            keys[static_cast<int>( TKEY )] = 0;\
        }

        UPDATE_KEY(GLFW_KEY_W, keyid::W)
        UPDATE_KEY(GLFW_KEY_A, keyid::A)
        UPDATE_KEY(GLFW_KEY_S, keyid::S)
        UPDATE_KEY(GLFW_KEY_D, keyid::D)

        UPDATE_KEY(GLFW_KEY_H, keyid::H)
        UPDATE_KEY(GLFW_KEY_J, keyid::J)
        UPDATE_KEY(GLFW_KEY_K, keyid::K)
        UPDATE_KEY(GLFW_KEY_L, keyid::L)

        UPDATE_KEY(GLFW_KEY_Q, keyid::Q)
        UPDATE_KEY(GLFW_KEY_E, keyid::E)

        #undef UPDATE_KEY
    }

    int getKey(keyid id) { return keys[static_cast<int>(id)]; }
}

#define INPUT_HPP
#endif