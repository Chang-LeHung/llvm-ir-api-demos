
#include <iostream>

template<typename FuncSignature>
class FunctionWrapper;

template<typename Ret, typename... Args>
class FunctionWrapper<Ret(Args...)> {
public:
    Ret (*toPtr() const )(Args...) {
       return reinterpret_cast<Ret(*)(Args...)>(funcAddr);
    }

    void setAddress(void *addr) {
       funcAddr = addr;
    }

private:
    void *funcAddr;
};

// 示例函数
int add(int a, int b) {
   return a + b;
}

int main() {
   FunctionWrapper<int(int, int)> wrapper; // 创建 FunctionWrapper 实例
   wrapper.setAddress(reinterpret_cast<void *>(&add)); // 设置函数地址

   // 获取函数指针
   auto funcPtr = wrapper.toPtr();

   // 调用函数并输出结果
   int result = funcPtr(3, 4);
   std::cout << "Result of add(3, 4): " << result << std::endl; // 输出: Result of add(3, 4): 7

   return 0;
}
