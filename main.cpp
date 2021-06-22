#include <iostream>
#include <vector>
#include <unordered_map>

#include <string>

#include "json.hpp"
#include "grammar.hpp"
#include "vec3.hpp"
#include "turtle.hpp"

int main(int argc, char * argv[]) {
    // set parsing filename
    json::setInputFilename("input.json");
    // L-system spec.
    //  set modules
    json::setModules(grammar::modules);
    //  get axiom
    grammar::word axiom = json::getAxiom();
    //  get iterations
    int iter = json::getIterations();
    //  get rewriting rules
    grammar::rewrites rules = json::getRules();

    std::cout << "will do " << iter << " applications\n";

    // get 'num'-th production
    std::cout << "axiom:    " << grammar::wordToString(axiom) << "\n";
    for(int i = 1; i <= iter; ++i) {
        grammar::apply(axiom,rules);
        std::cout << "(i = " << i << ") = " << grammar::wordToString(axiom) << "\n";
    }

    // turtle interpretation spec.
    //std::vector<vec3> moves = turtle::interpret(axiom, inter);
    //for(vec3& vec : moves) std::cout << vec << ", ";
    //std::cout << "\n";
}