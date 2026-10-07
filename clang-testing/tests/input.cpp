#include "helpers.h"
#include "logging.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

struct Thing {
  int id = 7;
  std::string name() const { return "thing"; }
};
#define WHERE "input.cpp"
#define TWICE(x) ((x) * 2)
#define LOG_IT(x) std::cout << x << std::endl

template <class T> void show(const T &t) {
  std::cout << "templated: " << t << std::endl;
}

class Widget {
  int w = 3, h = 4;

public:
  void print() const {
    std::cout << "Widget " << w << "x" << h
              << " area=" << w * h << '\n';
  }
};

int add(int a, int b) { return a + b; }

int main() {
  int a = 1, b = 2, n = 42;
  double pi = 3.14159;
  std::string s = "str";
  const char *cs = "cstr";
  char c = 'z';
  std::vector<int> vec{10, 20, 30};
  std::map<std::string, int> m{{"k", 5}};
  Thing t;
  Thing *tp = &t;

  std::cout << "plain" << std::endl;
  std::cout << "plain newline\n";
  std::cout << n << std::endl;
  std::cout << "a=" << a << " b=" << b << std::endl;
  std::cout << a << b << n << std::endl;
  std::cout << "multi "
            << "line "
            << a // a comment
            << " chain"
            << std::endl;
  cout << "using namespace " << s << endl;
  std::cout << "adjacent " "literals " << a << std::endl;
  std::cout << R"(raw "string" \ here)" << std::endl;
  std::cout << "braces {} and {" << a << "}" << std::endl;
  std::cout << "quote \" backslash \\ tab\tend" << std::endl;
  std::cout << "chars " << 'q' << '"' << '\'' << c << std::endl;
  std::cout << "endl mid" << std::endl << "next line " << b << std::endl;
  std::cout << "expr " << a + b << " call " << add(a, b)
            << " ternary " << (a > b ? "yes" : "no") << std::endl;
  std::cout << "member " << t.id << " " << t.name() << " " << tp->id
            << " index " << vec[1] << " map " << m["k"] << std::endl;
  std::cout << "concat " << s + "!" << " cstr " << cs << " pi " << pi
            << std::endl;
  std::cout << "macros " << WHERE << " " << TWICE(a) << std::endl;
  std::cout << "lambda " << [&] { return a + 1; }() << std::endl;
  std::cout << std::endl;
  for (int i = 0; i < 2; ++i) std::cout << "loop " << i << std::endl;
  if (a) std::cout << "if " << a << std::endl; else std::cout << "else\n";
  auto lam = [&]() { std::cout << "in lambda " << b << std::endl; };
  lam();
  Widget{}.print();
  show(n);
  show(s);
  fromHeader(n);
  std::cerr << "to cerr " << a << std::endl;

  // These should be skipped and reported:
  std::cout << std::hex << n << std::dec << std::endl;
  std::cout << std::setw(5) << n << std::endl;
  if (std::cout << "") {}
  LOG_IT("in a macro " << a);
  return 0;
}
