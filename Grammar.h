#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <functional>

class Rule {
 public:
  explicit Rule(const std::string& s);
  Rule(char from, std::string to) : from(from), to(std::move(to)) {};

  char from;
  std::string to;
};

namespace std {
template <>
struct hash<Rule> {
  size_t operator()(const Rule& rule) {
    size_t p = 31;
    size_t m = 1'000'000'007;
    size_t pn = p;
    size_t r = static_cast<unsigned char>(rule.from);

    for (const char& c : rule.to) {
      r = (r + (pn * c) % m) % m;
      pn = (pn * p) % m;
    }

    return r;
  }
};
}

bool operator==(const Rule& a, const Rule& b) {
  return (a.from == b.from) && (a.to == b.to);
}

class Grammar {
 public:
  Grammar();
  Grammar(const std::vector<char>& non_terminals,
          const std::vector<char>& terminals,
          char S,
          const std::vector<std::string>& _rules);

  bool IsTerminal(char c) const;
  bool IsNonTerminal(char c) const;
  bool IsEpsilonDeduce(char c) const;
  char GetS() const;
  const std::vector<Rule>& GetRules(char c) const;

  void AddRule(const Rule& rule);
  void AddNonTerminal(char c);
  void Clear();

 private:
  std::unordered_set<char> non_terminals;
  std::unordered_map<char, size_t> non_terminals_ind;
  std::unordered_set<char> terminals;
  char S;
  std::unordered_map<char, std::vector<Rule>> rules;
  std::vector<Rule> empty;
  std::unordered_set<char> epsilon_deduce;
};

Rule::Rule(const std::string& _s) {
  std::string s;
  s.reserve(_s.size());

  for (char c : _s)
    if (!std::isspace(c))
      s.push_back(c);

  from = s.front();
  to = s.substr(3, std::string::npos);
}

Grammar::Grammar(const std::vector<char>& non_terminals,
                 const std::vector<char>& terminals,
                 char S,
                 const std::vector<std::string>& _rules) : non_terminals(non_terminals.begin(), non_terminals.end()),
                                                           terminals(terminals.begin(), terminals.end()),
                                                           S(S) {
  for (const std::string& s : _rules) {
    AddRule(Rule(s));
  }

  for (char c : non_terminals) {
    non_terminals_ind.emplace(c, non_terminals_ind.size());
  }

  for (const auto&[c, c_rules] : rules) {
    for (const Rule& rule : c_rules)
      if (rule.to.empty())
        epsilon_deduce.insert(rule.from);
  }

  bool changed = !epsilon_deduce.empty();

  while (changed) {
    changed = false;

    for (const auto& [c, c_rules] : rules) {
      for (const Rule& rule : c_rules) {
        if (epsilon_deduce.find(rule.from) != epsilon_deduce.end())
          continue;

        bool flag = true;

        for (char ch : rule.to) {
          if (epsilon_deduce.find(ch) == epsilon_deduce.end()) {
            flag = false;
            break;
          }
        }

        if (flag) {
          epsilon_deduce.insert(rule.from);
          changed = true;
        }
      }
    }
  }
}

bool Grammar::IsTerminal(char c) const {
  return terminals.find(c) != terminals.end();
}

bool Grammar::IsNonTerminal(char c) const {
  return non_terminals.find(c) != non_terminals.end();
}

char Grammar::GetS() const {
  return S;
}

const std::vector<Rule>& Grammar::GetRules(char c) const {
  auto it = rules.find(c);

  if (it == rules.end())
    return empty;
  else
    return it->second;
}

void Grammar::AddRule(const Rule& rule) {
  auto it = rules.find(rule.from);

  if (it == rules.end()) {
    rules.insert({rule.from, {rule}});
  } else {
    it->second.push_back(rule);
  }
}

Grammar::Grammar(): S('S') {}

void Grammar::Clear() {
  non_terminals.clear();
  terminals.clear();
  rules.clear();
  empty.clear();
  epsilon_deduce.clear();
}

void Grammar::AddNonTerminal(char c) {
  non_terminals.insert(c);
  non_terminals_ind.emplace(c, non_terminals_ind.size());
}

bool Grammar::IsEpsilonDeduce(char c) const {
  return (epsilon_deduce.find(c) != epsilon_deduce.end());
}

std::istream& operator>>(std::istream& in, Grammar& g) {
  size_t non_terms, terms, rules_count;
  std::vector<char> non_terminals;
  std::vector<char> terminals;
  char S;
  std::vector<std::string> rules;
  std::string s;
  char c;

  in >> non_terms >> terms >> rules_count;

  for (size_t i = 0; i < non_terms; ++i) {
    in >> c;
    non_terminals.push_back(c);
  }

  for (size_t i = 0; i < terms; ++i) {
    in >> c;
    terminals.push_back(c);
  }

  for (size_t i = 0; i < rules_count; ++i) {
    in >> s;
    rules.push_back(s);
  }

  in >> S;

  g = Grammar(non_terminals, terminals, S, rules);

  return in;
}
