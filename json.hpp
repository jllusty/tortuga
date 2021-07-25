// json.hpp
// --------
// Functions to parse 'input.json' for L-system specification and 
// turtle interpretation.

#ifndef JSON_HPP

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include "include/rapidjson/document.h"

#include <vector>
#include <unordered_map>

#include "grammar.hpp"
#include "turtle.hpp"

namespace json {
    // parse JSON InputFile
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
    // get resolution
    int getResolution() {
        const char* resCStr = doc["resolution"].GetString();
        std::string resString(resCStr);
        return std::stoi(resString);
    }
    // set grammar modules
    void setModules(std::vector<std::shared_ptr<grammar::module>>& modules) {
        const rapidjson::Value& ma = doc["modules"];
        for(auto& m : ma.GetArray()) {
            //grammar::module myModule;
            char letter = m["symbol"].GetString()[0];
            const rapidjson::Value& pa = m["parameters"];
            std::vector<char> params;
            for(auto& p : pa.GetArray()) {
                char c = p.GetString()[0];
                params.push_back(c);
            }
            modules.push_back(std::make_shared<grammar::module>(letter,params));
        }
    }
    // get axiom
    grammar::word getAxiom() {
        grammar::word axiom;
        const rapidjson::Value& a = doc["axiom"];
        for(auto& e : a.GetArray()) {
            //grammar::evaluation myEval;
            char letter = e["symbol"].GetString()[0];
            std::shared_ptr<grammar::module> mP;
            for(auto& m : grammar::modules) {
                if(m->getLetter() == letter) {
                    mP = m; break;
                }
            }
            const rapidjson::Value& pa = e["parameters"];
            std::vector<double> vals;
            for(auto& p : pa.GetArray()) {
                std::string pString = std::string(p.GetString());
                vals.push_back(std::stod(pString));
            }
            axiom.push_back(grammar::evaluation(mP,vals));
        }
        return axiom;
    }
    // get rewriting/production rules
    grammar::rewrites getRules() {
        grammar::rewrites rules;
        const rapidjson::Value& ra = doc["rules"];
        for(auto& r : ra.GetArray()) {
            char letter = r["symbol"].GetString()[0];
            const rapidjson::Value& ca = r["conditions"];
            std::shared_ptr<grammar::module> mP;
            for(auto& m : grammar::modules) {
                if(m->getLetter() == letter) {
                    mP = m; break;
                }
            }
            // get conditions on module
            std::vector<std::string> conditions;
            for(auto& c : ca.GetArray()) {
                std::string cString = c.GetString();
                conditions.push_back(cString);
            }
            // get blueprints
            const rapidjson::Value& ra = r["word"];
            std::vector<grammar::blueprint> blueprints;
            for(auto& w : ra.GetArray()) {
                // letter
                char letter = w["symbol"].GetString()[0];
                std::shared_ptr<grammar::module> mP;
                for(auto& m : grammar::modules) {
                    if(m->getLetter() == letter) {
                        mP = m; break;
                    }
                }
                // parameters
                const rapidjson::Value& pa = w["parameters"];
                std::vector<std::string> evals;
                for(auto& p : pa.GetArray()) {
                    std::string eString = std::string(p.GetString());
                    evals.push_back(eString);
                }
                blueprints.push_back(grammar::blueprint(mP,evals));
            }
            rules.push_back(grammar::production(mP,conditions,blueprints));
        }
        return rules;
    }
    // get turtle interpretation of symbols
    /*std::unordered_map<char,turtle::TurtleCommand*> getInterpretation() {
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
    }*/
}

#define JSON_HPP
#endif