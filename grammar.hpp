#ifndef GRAMMAR_HPP

#include <cmath>
#include <vector>
#include <unordered_map>

namespace grammar {
    void apply(std::vector<char>& axiom, 
        std::unordered_map<char,std::vector<char>>& rules)
    {
        for(int i = 0; i < axiom.size(); ++i) {
            if(rules.count(axiom[i]) != 0) {
                std::vector<char>& rule = rules[axiom[i]];
                axiom.erase(axiom.begin()+i);
                axiom.insert(axiom.begin()+i, rule.begin(), rule.end());
                i += rule.size()-1;
            }
        }
    }
}

#define GRAMMAR_HPP
#endif