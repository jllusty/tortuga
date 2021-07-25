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
    struct Line {
        vec3 initial;
        vec3 terminal;
        float width;
    };

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
        std::vector<Line> moves;
    public:
        Turtle(vec3 pos, vec3 head)
            : state{pos,head}
            {}
        const std::vector<Line>& getMoves() const {
            return moves;
        }
        inline void move(float dist) {
            vec3 org = state.pos;
            state.pos += dist*state.head;
            float width = 1.0f/(float)(states.size()+2);
            Line path{org,state.pos,width};
            moves.push_back(path);
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
    std::vector<Line> interpret(grammar::word axiom) 
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