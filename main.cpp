#include <iostream>
#include <vector>
#include <unordered_map>

#include "grammar.hpp"
#include "vec3.hpp"
#include "turtle.hpp"

int main(int argc, char * argv[]) {
    using namespace std;

    // L-system spec.
    //  production rules
    unordered_map<char,vector<char>> rules;
    rules['A'] = vector<char>{'A','B'};
    rules['B'] = vector<char>{'A'};
    //  axiom
    vector<char> axiom{'A'};
    // get 'num'-th production
    int num = 2;  // num productions
    for(int i = 0; i < num; ++i) {
        grammar::apply(axiom,rules);
    }
    for(char& c : axiom) cout << c;
    cout << "\n";

    // turtle interpretation spec.
    unordered_map<char,turtle::TurtleCommand*> interpretation;
    interpretation['A'] = &turtle::moveTurtle;
    interpretation['B'] = &turtle::rotateTurtleX;
    // get interpretation's path
    vector<vec3> moves = turtle::interpret(axiom, interpretation);
    for(vec3& vec : moves) cout << vec << ", ";
    cout << "\n";
}