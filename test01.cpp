
#include <iostream>

using namespace std;

class Demo {

public:
    Demo() {
       cout << "Demo()" << endl;
    }

    Demo(const Demo& d) {
       cout << "Demo(const Demo&)" << endl;
    }

    Demo& operator=(const Demo& d) {
       cout << "Demo& operator=(const Demo&)" << endl;
       return *this;
    }

    ~Demo() {
       cout << "~Demo()" << endl;
    }
};

int main() {
   auto demo = make_unique<Demo>();
   auto d = *demo;
   return 0;
}