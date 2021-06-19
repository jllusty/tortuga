#ifndef TURTLE_HPP

#include <vector>
#include <unordered_map>
#include <cmath>
#include "vec3.hpp"
#include <iostream>
#include <string>

namespace turtle {
    // turtle state
    struct Turtle {
        // position vec3 = (x,y,z)
        vec3 pos = vec3(0,0,0);
        // heading vec3 = (hx,hy,hz)
        vec3 head = vec3(0,0,1);
    };

    // turtle commands
    class TurtleCommand {
    public:
        virtual ~TurtleCommand() {}
        virtual void execute(Turtle& t) = 0;
    };
    // move commands
    //  move forward/backward, tracks moves
    class MoveCommand : public TurtleCommand {
        static std::vector<vec3> moves;
        float dist;
    public:
        MoveCommand(float dist) : dist{dist} {}
        inline virtual void execute(Turtle& t) {
            t.pos += dist*t.head;
            moves.push_back(t.pos);
        }
        static std::vector<vec3> getMoves() {
            return moves;
        }
    };
    std::vector<vec3> MoveCommand::moves{ vec3(0,0,0) };
    //  rotate
    class RotateCommand : public TurtleCommand {
        vec3 axis = vec3(0,0,0);
        float theta = 0;
    public:
        inline RotateCommand(vec3 axis, float theta) 
            : axis{axis}, theta{theta} {}
        inline virtual void execute(Turtle& t) {
            rotate(t.head, axis, theta);
        }
    };
    // state commands
    class StateCommand : public TurtleCommand {
    public:
        static std::vector<Turtle> getStates() {
            return states;
        }
    protected:
        static std::vector<Turtle> states;
    };
    std::vector<Turtle> StateCommand::states;
    // push turtle state
    class PushCommand : public StateCommand {
    public:
        inline virtual void execute(Turtle& t) {
            states.push_back(t);
        }
    };
    // pop turtle state
    class PopCommand : public StateCommand {
    public:
        inline virtual void execute(Turtle& t) {
            t = states.back();
            states.pop_back();
        }
    };
    // static command instances
    static PopCommand popTurtle;
    static PushCommand pushTurtle;
    static MoveCommand moveTurtle = MoveCommand(1);
    const float pi = 4.0f * atanf(1.0f);
    static float theta = pi/2.0f;
    static RotateCommand rotateTurtleX = RotateCommand(vec3(1,0,0),theta);
    static RotateCommand rotateTurtleY = RotateCommand(vec3(0,1,0),theta);
    static RotateCommand rotateTurtleZ = RotateCommand(vec3(0,0,1),theta);

    // interactive interpretation
    std::vector<vec3> interact(std::vector<char> axiom, 
        std::unordered_map<char,TurtleCommand*> interpretation) 
    {
        Turtle t; 
        std::string input;
        // test turtle engine
        while(true) {
            std::cout << "--- turtle stack ---\n";
            for(Turtle& t : StateCommand::getStates()) {
                std::cout << "\tt.pos = " << t.pos << "\n";
                std::cout << "\tt.head = " << t.head << "\n";
            }
            std::cout << "--- turtle state ---\n";
            std::cout << "t.pos = " << t.pos << "\n";
            std::cout << "t.head = " << t.head << "\n";
            std::cout << "> ";
            std::getline(std::cin,input);
            if(input == "move") { 
                moveTurtle.execute(t);
            }
            else if (input == "rotateX") {
                rotateTurtleX.execute(t);
            }
            else if (input == "rotateY") {
                rotateTurtleY.execute(t);
            }
            else if (input == "rotateZ") {
                rotateTurtleZ.execute(t);
            }
            else if (input == "push") {
                pushTurtle.execute(t);
            }
            else if (input == "pop") {
                popTurtle.execute(t);
            }
            else if (input == "exit") {
                break;
            }
        }
        return MoveCommand::getMoves();
    }

    // perform interpretation
    std::vector<vec3> interpret(std::vector<char> axiom, 
        std::unordered_map<char,TurtleCommand*> interpretation) 
    {
        Turtle t; 
        for(char& c : axiom) {
            if(interpretation.count(c) != 0) {
                TurtleCommand* cmd = interpretation[c];
                cmd->execute(t);
            }
        }
        return MoveCommand::getMoves();
    }
}

#define TURTLE_HPP
#endif