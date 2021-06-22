#ifndef TURTLE_HPP

#include <vector>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include <string>
#include <memory>
#include <utility>

#include "vec3.hpp"
#include "grammar.hpp"

namespace turtle {
    class Turtle {
        // pushdown position and heading, for branching
        struct TurtleState {
            // position vec3 = (x,y,z)
            vec3 pos = vec3(0,0,0);
            // heading vec3 = (hx,hy,hz)
            vec3 head = vec3(0,0,1);
        } state;
        // turtle states
        std::vector<TurtleState> states;
        // moves
        std::vector<std::pair<vec3,vec3>> moves;
    public:
        Turtle(vec3 pos, vec3 head)
            : state{pos,head}
            {}
        const std::vector<std::pair<vec3,vec3>>& getMoves() const {
            return moves;
        }
        inline void move(float dist) {
            vec3 org = state.pos;
            state.pos += dist*state.head;
            moves.push_back(std::make_pair(org,state.pos));
        }
        inline void rotX(float theta) {
            rotate(state.head, vec3(1,0,0), theta);
        }
        inline void rotY(float theta) {
            rotate(state.head, vec3(0,1,0), theta);
        }
        inline void rotZ(float theta) {
            rotate(state.head, vec3(0,0,1), theta);
        }
        inline void push() {
            states.push_back(state);
        }
        inline void pop() {
            state = states.back();
            states.pop_back();
        }
    };

    const float pi = 4.0f * atanf(1.0f);

    // perform static interpretation
    std::vector<std::pair<vec3,vec3>> interpret(grammar::word axiom) 
    {
        Turtle t(vec3(0,0,0),vec3(0,0,1));
        for(grammar::evaluation& e : axiom) {
            char c = e.getLetter();
            std::vector<double> p = e.getVals();
            if(c == 'F') {
                t.move(p[0]);
            }
            else if(c == '[') {
                t.push();
            }
            else if(c == ']') {
                t.pop();
            }
            else if(c == '+') {
                t.rotX(p[0]);
            }
            else if(c == '-') {
                t.rotX(-p[0]);
            }
            else if(c == '&') {
                t.rotY(p[0]);
            }
            else if(c == '^') {
                t.rotY(-p[0]);
            }
            else if(c == '*') {
                t.rotX(p[0]);
            }
            else if(c == '~') {
                t.rotX(-p[0]);
            }
        }
        return t.getMoves();
    }
}

#define TURTLE_HPP
#endif