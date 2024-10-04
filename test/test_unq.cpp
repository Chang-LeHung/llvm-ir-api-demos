
#include <vector>
#include <iostream>

using namespace std;

int main() {
   auto ptr = make_unique<vector<int>>();
   ptr->push_back(1);
   auto vec = ptr.get();
   cout << ptr->size() << endl;
   return 0;
}