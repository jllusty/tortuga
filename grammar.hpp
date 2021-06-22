// grammar.hpp
// -----------
// Objects and methods for rewriting L-systems

#ifndef GRAMMAR_HPP

#include <cmath>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

#include "parse.hpp"

namespace grammar {
    // module - letter/symbol, and symbolic parameters
    class module {
        char letter;
        std::vector<char> params;
    public:
        module(char letter, std::vector<char> params) 
            : letter(letter), params(params) {}
        const char& getLetter() const {
            return letter;
        }
        const std::vector<char>& getParams() const {
            return params;
        }
    };
    std::vector<std::shared_ptr<module>> modules; // populated by reading json
    // evaluated module - module, and numerical parameters
    class evaluation {
        std::shared_ptr<module> m;
        std::vector<double> vals;
    public:
        evaluation(std::shared_ptr<module> pModule, const std::vector<double>& vals)
            : m(pModule), vals(vals) {}
        const char& getLetter() const { return m->getLetter(); }
        const std::vector<char>& getParams() const { return m->getParams(); }
        const std::vector<double>& getVals() const { return vals; }
    };
    using word = std::vector<evaluation>;
    // blueprint - module, and symbolic parameters to be evaluated into an evaluation
    class blueprint {
        std::shared_ptr<module> m;
        std::vector<std::string> evals;
    public:
        blueprint(std::shared_ptr<module> pModule, 
            const std::vector<std::string>& evals) 
            : m(pModule), evals(evals)
        {}
        const char& getLetter() const {
            return m->getLetter();
        }
        evaluation evaluate(const std::vector<char>& params,
                const std::vector<double>& vals) const {
            std::vector<double> passVals(m->getParams().size(),0);
            // match parameter names
            for(int i = 0; i < m->getParams().size(); ++i) {
                for(int j = 0; j < params.size(); ++j) {
                    if(m->getParams()[i] == params[j]) {
                        passVals[i] = vals[j];
                    }
                }
            }
            // pass matched values to user-specified arithmetic expressions
            std::vector<double> newVals = passVals;
            for(int i = 0; i < passVals.size(); ++i)
                newVals[i] = parse::evaluateArithmetic(evals[i], m->getParams(), passVals);
            return evaluation(m,newVals);
        }
    };
    // production rule: conditional map from evaluation -> word
    // calling code: if production.condition(eval), 
    //               then substitute production.rewrite(eval)
    class production {
        std::shared_ptr<module> m;
        // lex'ed condition strings on evaluation's values
        std::vector<std::string> conds;
        // vector of parameterized modules
        std::vector<blueprint> blueprints;
    public:
        production(std::shared_ptr<module> pModule,
            const std::vector<std::string>& conditions,
            std::vector<blueprint>& blueprints)
                : m(pModule), conds(conditions), blueprints(blueprints)
            {}
        const char& getLetter() const {
            return m->getLetter();
        }
        const std::vector<std::string>& getConditions() const {
            return conds;
        }
        const std::vector<blueprint>& getBlueprints() const {
            return blueprints;
        }
        bool condition(const evaluation& e) const {
            for(auto& cs : conds) {
                if(!parse::evaluateConditional(cs, e.getParams(), e.getVals())) 
                    return false;
            }
            return true;
        }
        word rewrite(const evaluation& e) const {
            word w;
            for(const auto& bp : blueprints) {
                w.push_back(bp.evaluate(e.getParams(), e.getVals()));
            }
            return w;
        }
    }; 
    using rewrites = std::vector<production>;

    // Parametric OL system
    void apply(word& axiom, rewrites& rules) {
        // iterate over the current word
        for(int i = 0; i < axiom.size(); ++i) {
            evaluation& e = axiom[i];
            char letter = e.getLetter();
            // iterate over all the rules
            for(const auto& rule : rules) {
                // letter matches
                if(rule.getLetter() == letter) {
                    // try conditions
                    if(rule.condition(e)) {
                        word w = rule.rewrite(e);
                        axiom.erase(axiom.begin()+i);
                        axiom.insert(axiom.begin()+i, w.begin(), w.end());
                        i += w.size()-1; break;
                    }
                }
            }
        }
    }

    // print an evaluate string of modules
    std::string wordToString(const word& w) {
        std::stringstream buffer;
        for(int i = 0; i < w.size(); ++i) {
            if(w[i].getVals().size() > 0) {
                buffer << w[i].getLetter() << "(";
                for(int ii = 0; ii < w[i].getVals().size()-1; ++ii) 
                    buffer << w[i].getVals()[ii] << ",";
                buffer << w[i].getVals().back() << ")";
            }
            else {
                buffer << w[i].getLetter();
            }
        }
        return buffer.str();
    }
}

#define GRAMMAR_HPP
#endif