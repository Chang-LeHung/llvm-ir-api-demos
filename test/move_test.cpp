#include <iostream>
#include <memory>  // 包含 std::unique_ptr

// 定义一个简单的类 Module
class Module {
public:
    Module(const std::string &name) : name(name) {
       std::cout << "Module " << name << " constructed.\n";
    }
    ~Module() {
       std::cout << "Module " << name << " destructed.\n";
    }
    Module(Module&& module) = delete;

    Module& operator=(Module&&) = delete;

    void display() const {
       std::cout << "Module name: " << name << "\n";
    }

private:
    std::string name;
};

// 接收 std::unique_ptr 的函数，使用移动语义
void processModule(std::unique_ptr<Module> mod) {
   std::cout << "Processing module...\n";
   mod->display();  // 使用传入的模块指针
   // 当函数结束时，mod 的生命周期结束，资源会自动释放
}

int main() {
   // 使用 std::make_unique 创建一个 std::unique_ptr<Module>
   auto mod = std::make_unique<Module>("TestModule");

   // 将 mod 通过 std::move 移动到函数内部
   processModule(std::move(mod));

   // mod 现在是空的，因为它的所有权已经被移动
   if (!mod) {
      std::cout << "mod is now nullptr after being moved.\n";
   }

   return 0;
}
