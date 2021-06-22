// parse.hpp
// ---------
// Functions to evaluate postfix conditional & arithmetic expressions

#ifndef PARSE_HPP

#include <cstdlib>
#include <cctype>
#include <cmath>

#include <functional>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <vector>
#include <unordered_map>

static bool is_number(const std::string& s) {
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
}

namespace parse {
    // parse lex'd conditional expression (assumes postfix)
    bool evaluateConditional(std::string expression, std::vector<char> params, std::vector<double> vals) {
        auto iss = std::istringstream{expression};
        std::string str;
        std::vector<double> tStack;
        while (iss >> str) {
            // check for a given parameter
            if((str.length() == 1)&&isalpha(str[0])) {
                for(int i = 0; i < params.size(); ++i) {
                    if(params[i] == str[0]) {
                        tStack.push_back(vals[i]);
                        break;
                    }
                }
                // catch something
            }
            // check for a number
            else if(is_number(str)) {
                tStack.push_back(std::stod(str));
            }
            // operations
            else if(str == "+") {
                double x = tStack.back(); tStack.pop_back();
                double y = tStack.back(); tStack.pop_back();
                tStack.push_back(x+y);
            }
            else if(str == "-") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                tStack.push_back(x-y);
            }
            else if(str == "/") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                tStack.push_back(x/y);
            }
            else if(str == "*") {
                double x = tStack.back(); tStack.pop_back();
                double y = tStack.back(); tStack.pop_back();
                tStack.push_back(x*y);
            }
            // conditionals
            else if(str == "<=") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                return (x <= y);
            }
            else if(str == ">=") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                return (x >= y);
            }
            else if(str == ">") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                return (x > y);
            }
            else if(str == "<") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                return (x < y);
            }
        }
        // catch something
        std::cout << "unexpected conditional\n";
        return false;
    }

    double evaluateArithmetic(std::string expression, std::vector<char> params,
        std::vector<double> vals)
    {
        //std::cout << "evaluting: \"" << expression << "\"\n";
        auto iss = std::istringstream{expression};
        std::string str;
        std::vector<double> tStack;
        while (iss >> str) {
            // check for a given parameter
            if((str.length() == 1)&&isalpha(str[0])) {
                for(int i = 0; i < params.size(); ++i) {
                    if(params[i] == str[0]) {
                        tStack.push_back(vals[i]);
                        break;
                    }
                }
                // catch something
            }
            // check for a number
            else if(is_number(str)) {
                tStack.push_back(std::stod(str));
            }
            // operations
            else if(str == "+") {
                double x = tStack.back(); tStack.pop_back();
                double y = tStack.back(); tStack.pop_back();
                tStack.push_back(x+y);
            }
            else if(str == "-") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                tStack.push_back(x-y);
            }
            else if(str == "/") {
                double y = tStack.back(); tStack.pop_back();
                double x = tStack.back(); tStack.pop_back();
                tStack.push_back(x/y);
            }
            else if(str == "*") {
                double x = tStack.back(); tStack.pop_back();
                double y = tStack.back(); tStack.pop_back();
                tStack.push_back(x*y);
            }
            // catch something
        }
        return tStack.back();
    }
}

#define PARSE_HPP
#endif