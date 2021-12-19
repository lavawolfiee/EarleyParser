#pragma once

#include "Grammar.h"

class EarleyParser {
 public:
  EarleyParser() = default;

  EarleyParser& fit(const Grammar& g);
  bool predict(const std::string& w);

 private:
  class Configuration;
  Configuration Scan(Configuration conf, char c);
  void Predict(const Configuration& conf, size_t k);
  void Complete(const Configuration& conf, size_t k);
  void AddToD(const Configuration& conf, size_t k);

  struct Configuration {
    const Rule& rule;
    size_t dot; // dot is before this index
    size_t i;

    Configuration(const Rule& rule, size_t dot, size_t i): rule(rule), dot(dot), i(i) {}
    char NextChar() const;
    bool IsFinished() const;
    void Move();
  };

  std::vector<std::unordered_set<Configuration>> d;
  std::vector<std::vector<Configuration>> d_vec;
  Grammar grammar;
  std::string word;
};

char EarleyParser::Configuration::NextChar() const {
  if (IsFinished())
    return '$';
  else
    return rule.to.at(dot);
}

bool EarleyParser::Configuration::IsFinished() const {
  return dot >= rule.to.size();
}

void EarleyParser::Configuration::Move() {
  ++dot;
}

EarleyParser& EarleyParser::fit(const Grammar& g) {
  grammar = g;
  return *this;
}

EarleyParser::Configuration EarleyParser::Scan(EarleyParser::Configuration conf, char c) {
  if (conf.IsFinished() || conf.NextChar() != c || grammar.IsNonTerminal(c))
    throw std::runtime_error("Can't perform that Scan");

  conf.Move();
  return conf;
}

void EarleyParser::Predict(const EarleyParser::Configuration& conf, size_t k) {
  char next = conf.NextChar();

  for (const auto& rule : grammar.GetRules(next)) {
    Configuration new_conf(rule, 0, k);
    AddToD(new_conf, k + 1);
  }
}

void EarleyParser::Complete(const EarleyParser::Configuration& conf, size_t k) {
  size_t x = conf.i;
  char B = conf.rule.from;

  for (size_t i = 0; i < d_vec.at(x).size(); ++i) {
    const auto& parent = d_vec.at(x).at(i);

    if (parent.NextChar() != B)
      continue;

    Configuration new_conf(parent);
    new_conf.Move();
    AddToD(new_conf, k);
  }
}

bool EarleyParser::predict(const std::string& w) {
  word = w;
  d.resize(w.size() + 1);
  d_vec.resize(w.size() + 1);

  std::string start_string = "'->";
  start_string.push_back(grammar.GetS());
  Rule start_rule(start_string);
  Configuration start_conf(start_rule, 0, 0);
  d.front().insert(start_conf);
  d_vec.front().push_back(start_conf);

  for (size_t k = 0; k <= w.size(); ++k) {
    for (size_t i = 0; i < d_vec.at(k).size(); ++i) {
      const auto& conf = d_vec.at(k).at(i);

      if (!conf.IsFinished()) {
        char c = conf.NextChar();
        if (grammar.IsNonTerminal(c)) {
          Predict(conf, k);
        } else if (grammar.IsTerminal(c)) {
          Configuration new_conf = Scan(conf, c);
          AddToD(new_conf, k + 1);
        }
      } else {
        Complete(conf, k);
      }
    }
  }

  Configuration target_conf(start_rule, 1, 0);
  return (d.back().find(target_conf) != d.back().end());
}

void EarleyParser::AddToD(const Configuration& conf, size_t k) {
  d.at(k).insert(conf);
  d_vec.at(k + 1).push_back(conf);
}
