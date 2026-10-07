#include <iostream>
#include <string>

struct Point {
  int x, y;
};
std::ostream &operator<<(std::ostream &Out, const Point &P) {
  return Out << P.x << "," << P.y;
}

void warnings() {
  bool ok = true;
  int value = 3;
  int *ptr = &value;
  std::cout << "flag " << ok << std::endl;
  std::cout << "pointer " << ptr << std::endl;
  std::cout << "point " << Point{1, 2} << std::endl;
}
