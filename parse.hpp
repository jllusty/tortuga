#ifndef PARSE_HPP

#include <string>
#include <fstream>
#include <streambuf>
#include "rapidjson/document.h"

#include <vector>
#include <unordered_map>

#include "turtle.hpp"


namespace parse {
    rapidjson::Document doc;
    void setInputFilename(const std::string& infilename) {
        std::ifstream input(infilename);
        // read file into std::string
        std::string json((std::istreambuf_iterator<char>(input)),
                        std::istreambuf_iterator<char>());
        // parse JSON into rapidJSON::Document
        doc.Parse(json.c_str());
    }
    // get iter
    int getIterations() {
        const char* iterCStr = doc["iter"].GetString();
        std::string iterString(iterCStr);
        return std::stoi(iterString);
    }
    // get axiom
    std::vector<char> getAxiom() {
        const char* axiomCStr = doc["axiom"].GetString();
        std::string axiomString(axiomCStr);
        return std::vector<char>(axiomString.begin(), axiomString.end());
    }
    // get rewriting/production rules
    std::unordered_map<char,std::vector<char>> getRules() {
        std::unordered_map<char,std::vector<char>> rules;
        const rapidjson::Value::Object& rulesObject = doc["rules"].GetObject();
        for(rapidjson::Value::ConstMemberIterator itr = rulesObject.MemberBegin();
            itr != rulesObject.MemberEnd(); ++itr)
        {
            const char* keyCStr = itr->name.GetString();
                std::string keyString(keyCStr);
                char key = keyString[0];
            const char* valCStr = itr->value.GetString();
                std::string valString(valCStr);
                std::vector<char> val(valString.begin(),valString.end());
            rules[key] = val;
        }
        return rules;
    }
    // get turtle interpretation of symbols
    std::unordered_map<char,turtle::TurtleCommand*> getInterpretation() {
        std::unordered_map<char,turtle::TurtleCommand*> inter;
        const rapidjson::Value::Object& interObject = doc["inter"].GetObject();
        for(rapidjson::Value::ConstMemberIterator itr = interObject.MemberBegin();
            itr != interObject.MemberEnd(); ++itr)
        {
            const char* keyCStr = itr->name.GetString();
                std::string keyString(keyCStr);
                char key = keyString[0];
            const char* valCStr = itr->value.GetString();
                std::string valString(valCStr);
            if(valString == "move") {
                inter[key] = &turtle::moveTurtle;
            }
            else if(valString == "rotateX") {
                inter[key] = &turtle::rotateTurtleX;
            }
            else if(valString == "rotateY") {
                inter[key] = &turtle::rotateTurtleY;
            }
            else if(valString == "rotateZ") {
                inter[key] = &turtle::rotateTurtleZ;
            }
            else if(valString == "pop") {
                inter[key] = &turtle::popTurtle;
            }
            else if(valString == "push") {
                inter[key] = &turtle::pushTurtle;
            }
        }
        return inter;
    }
}

#define PARSE_HPP
#endif