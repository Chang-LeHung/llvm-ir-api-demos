#include <iostream>

using namespace std;

// 简化版 unique_ptr
template<typename T>
class MyUniquePtr {
private:
    T *ptr; // 管理的原生指针

public:
    // 构造函数：通过指针初始化 unique_ptr
    explicit MyUniquePtr(T *p = nullptr) : ptr(p) {
       cout << "constructor called\n";
    }

    // 禁止拷贝构造函数
    MyUniquePtr(const MyUniquePtr &) = delete;

    // 禁止拷贝赋值运算符
    MyUniquePtr &operator=(const MyUniquePtr &) = delete;

    MyUniquePtr(MyUniquePtr &&other) noexcept: ptr(other.ptr) {
       other.ptr = nullptr;
       cout << "move constructor called\n";
    }

    // 移动赋值运算符：转移所有权
    MyUniquePtr &operator=(MyUniquePtr &&other) noexcept {
       if (this != &other) {
          delete ptr;
          ptr = other.ptr;
          other.ptr = nullptr;
       }
       return *this;
    }

    // 重载解引用运算符
    T &operator*() const {
       return *ptr;
    }

    // 重载箭头运算符
    T *operator->() const {
       return ptr;
    }

    // 检查是否为空
    bool is_null() const {
       return ptr == nullptr;
    }

    // 析构函数：释放资源
    ~MyUniquePtr() {
       delete ptr;
    }
};

// 实现 make_unique 函数
template<typename T, typename... Args>
MyUniquePtr<T> my_make_unique(Args &&... args) {
   return MyUniquePtr<T>(new T(std::forward<Args>(args)...));
}

int main() {
   // 使用自定义的 MyUniquePtr 和 my_make_unique
   MyUniquePtr<int> ptr = my_make_unique<int>(10);
   std::cout << "Value: " << *ptr << std::endl;

   // 通过移动语义转移所有权
   MyUniquePtr<int> ptr2 = std::move(ptr);
   if (ptr.is_null()) {
      std::cout << "ptr is null after move!" << std::endl;
   }
   std::cout << "Value in ptr2: " << *ptr2 << std::endl;

   return 0;
}
