#include <iostream>
#include <vector>
#include <unordered_map>

#include <string>
#include <fstream>
#include <streambuf>
#include "rapidjson/document.h"

#include "parse.hpp"
#include "grammar.hpp"
#include "vec3.hpp"
#include "turtle.hpp"

int main(int argc, char * argv[]) {
    // set parsing filename
    parse::setInputFilename("input.json");
    // L-system spec.
    //  get iterations
    int iter = parse::getIterations();
    //  get axiom
    std::vector<char> axiom = parse::getAxiom();
    //  get rewriting/production rules
    std::unordered_map<char,std::vector<char>> rules = parse::getRules();
    // get turtle interpretation of symbols
    std::unordered_map<char,turtle::TurtleCommand*> inter = parse::getInterpretation();

    // get 'num'-th production
    for(int i = 0; i < iter; ++i) {
        grammar::apply(axiom,rules);
    }
    for(char& c : axiom) std::cout << c;
    std::cout << "\n";

    // turtle interpretation spec.
    std::vector<vec3> moves = turtle::interpret(axiom, inter);
    for(vec3& vec : moves) std::cout << vec << ", ";
    std::cout << "\n";
}