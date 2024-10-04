
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

int myAdd(int a, int b) {
   return a + b;
}

int main() {
   // 初始化 LLVM 环境
   InitializeNativeTarget();
   InitializeNativeTargetAsmPrinter();

   LLVMContext context;
   std::unique_ptr<Module> module = std::make_unique<Module>("my_module", context);
   IRBuilder<> builder(context);

   // 函数签名：int printf(const char*, ...)
   std::vector<Type *> printfArgs({builder.getInt8PtrTy()}); // const char*
   FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), printfArgs, true); // 可变参数

   // 声明 printf 函数
   FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

   // 创建 add 函数，签名：int add(int, int)
   std::vector<Type *> addArgs({builder.getInt32Ty(), builder.getInt32Ty()});
   FunctionType *addType = FunctionType::get(builder.getInt32Ty(), addArgs, false);
   Function *addFunc = Function::Create(addType, Function::ExternalLinkage, "add", module.get());

   // 创建 basic block 并设置插入点，定义 add 函数体
   BasicBlock *addEntry = BasicBlock::Create(context, "entry", addFunc);
   builder.SetInsertPoint(addEntry);

   // 获取 add 函数的参数
   Function::arg_iterator args = addFunc->arg_begin();
   Value *x = args++;
   x->setName("x");
   Value *y = args++;
   y->setName("y");

   // 生成函数体：return x + y;
   Value *sum = builder.CreateAdd(x, y, "sum");
   builder.CreateRet(sum);

   // 创建 main 函数
   FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
   Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());

   // 创建 basic block 并设置插入点，定义 main 函数体
   BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunc);
   builder.SetInsertPoint(entry);

   // 创建 "Hello, LLVM!\n" 字符串
   Value *helloWorld = builder.CreateGlobalStringPtr("Hello, LLVM! add result = %d\n");

   // 在 main 函数中调用 add(10, 32)
   Value *addResult = builder.CreateCall(addFunc, {builder.getInt32(10), builder.getInt32(32)}, "addCall");

   // 调用 printf 函数，打印 add 的结果
   builder.CreateCall(printfFunc, {helloWorld, addResult});

   // 返回 0
   builder.CreateRet(builder.getInt32(0));

   // 打印生成的 LLVM IR
   module->print(outs(), nullptr);

   // 使用 ORC JIT 执行
   auto JIT = orc::LLJITBuilder().create();
   if (!JIT) {
      std::cerr << "Error creating JIT\n";
      return 1;
   }

   auto threadSafeModule = orc::ThreadSafeModule(std::move(module), std::make_unique<LLVMContext>());
   if (auto err = JIT->get()->addIRModule(std::move(threadSafeModule))) {
      std::cerr << "Error adding module to JIT\n";
      return 1;
   }

   // 查找 main 函数并执行
   auto sym = JIT->get()->lookup("main");
   if (!sym) {
      std::cerr << "Error finding main function\n";
      return 1;
   }

   auto main = sym->toPtr<int()>();
   int result = main();
   std::cout << "Execution finished with return code: " << result << std::endl;

   return 0;
}
