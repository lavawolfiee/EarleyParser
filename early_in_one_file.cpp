#include <iostream>
#include <random>
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



class EarleyParser {
 public:
  EarleyParser() = default;

  EarleyParser& fit(const Grammar& g);
  bool predict(const std::string& w);

 public:
  class Configuration;

 private:
  Configuration Scan(Configuration conf, char c);
  void Predict(Configuration conf, size_t k);
  void Complete(const Configuration& conf, size_t k);
  void AddToD(const Configuration& conf, size_t k);
  void Clear();

 public:
  struct Configuration {
    const Rule& rule;
    size_t dot; // dot is before this index
    size_t i;

    Configuration(const Rule& rule, size_t dot, size_t i): rule(rule), dot(dot), i(i) {}
    char NextChar() const;
    bool IsFinished() const;
    void Move();
  };

 private:
  std::vector<std::unordered_set<Configuration>> d;
  std::vector<std::vector<Configuration>> d_vec;
  Grammar grammar;
  std::string word;
};

namespace std {
template<>
struct hash<EarleyParser::Configuration> {
  size_t a, b, m;

  hash() {
    std::mt19937 random((std::random_device()()));
    a = random() % 2'003;
    b = random() % 1'000'007;
    m = random() % 1'000'007;
  }

  size_t operator()(const EarleyParser::Configuration& conf) const {
    size_t hash1 = (a * conf.dot + b) % m;
    size_t hash2 = (a * conf.i + b) % m;
    return hash<Rule>()(conf.rule) ^ hash1 ^ (hash2 << 16U);
  }
};
}

bool operator==(const EarleyParser::Configuration& a, const EarleyParser::Configuration& b) {
  return (a.rule == b.rule) && (a.dot == b.dot) && (a.i == b.i);
}

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

void EarleyParser::Predict(EarleyParser::Configuration conf, size_t k) {
  char next = conf.NextChar();

  for (const auto& rule : grammar.GetRules(next)) {
    Configuration new_conf(rule, 0, k);
    AddToD(new_conf, k);
  }

  if (grammar.IsEpsilonDeduce(next)) {
    conf.Move();
    AddToD(conf, k);
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
  d.resize(w.size() + 2);
  d_vec.resize(w.size() + 2);

  std::string start_string = "'->";
  grammar.AddNonTerminal('\'');
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
        } else if (grammar.IsTerminal(c) && k < word.size() && c == word.at(k)) {
          Configuration new_conf = Scan(conf, c);
          AddToD(new_conf, k + 1);
        }
      } else {
        Complete(conf, k);
      }
    }
  }

  Configuration target_conf(start_rule, 1, 0);
  bool res = (d.at(w.size()).find(target_conf) != d.back().end());
  Clear();
  return res;
}

void EarleyParser::AddToD(const Configuration& conf, size_t k) {
  if (d.at(k).find(conf) == d.at(k).end()) {
    d.at(k).insert(conf);
    d_vec.at(k).push_back(conf);
  }
}

void EarleyParser::Clear() {
  word.clear();
  d.clear();
  d_vec.clear();
}

int main() {
  Grammar g;
  std::cin >> g;

  EarleyParser parser;
  parser.fit(g);

  size_t m;
  std::cin >> m;

  std::string word;
  for (size_t i = 0; i < m; ++i) {
    std::cin >> word;
    std::cout << (parser.predict(word) ? "Yes" : "No") << std::endl;
  }

  return 0;
}
