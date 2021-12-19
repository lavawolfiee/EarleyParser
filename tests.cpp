#include <iostream>
#include <fstream>
#include "Grammar.h"
#include "EarleyParser.h"

int main() {
  for (size_t j = 1; j <= 14; ++j) {
    std::ifstream in("../Tests/Test" + std::to_string(j) + ".txt");

    Grammar g;
    in >> g;

    EarleyParser parser;
    parser.fit(g);

    size_t m;
    in >> m;

    std::vector<std::string> answers(m);

    std::string word;
    for (size_t i = 0; i < m; ++i) {
      in >> word;
      answers[i] = (parser.predict(word) ? "YES" : "NO");
    }
    for (size_t i = 0; i < m; ++i) {
      in >> word;
      if (word != answers[i]) {
        std::cerr << "Error!" << std::endl;
      }
    }

    std::cout << "Test " << j << " passed" << std::endl;
  }

  return 0;
}