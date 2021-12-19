#pragma once

#include <utility>
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

class Grammar {
 public:
  Grammar();
  Grammar(const std::vector<char>& non_terminals,
          const std::vector<char>& terminals,
          char S,
          const std::vector<std::string>& _rules);

  bool IsTerminal(char c) const;
  bool IsNonTerminal(char c) const;
  char GetS() const;
  const std::vector<Rule>& GetRules(char c) const;

  void AddRule(const Rule& rule);

 private:
  std::unordered_set<char> non_terminals;
  std::unordered_set<char> terminals;
  char S;
  std::unordered_map<char, std::vector<Rule>> rules;
  std::vector<Rule> empty;
};

Rule::Rule(const std::string& _s) {
  std::string s;
  s.reserve(_s.size());

  for (auto it = s.begin(); it != s.end(); ++it)
    if (!std::isspace(*it))
      s.push_back(*it);

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
