#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

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

   // 创建 main 函数
   FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
   Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());

   // 创建 basic block 并设置插入点
   BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunc);
   builder.SetInsertPoint(entry);

   // 创建 "Hello, LLVM!\n" 字符串
   Value *helloWorld = builder.CreateGlobalStringPtr("Hello, LLVM num = %d!\n");

   // 调用 printf 函数
   builder.CreateCall(printfFunc, {helloWorld, builder.getInt32(0x100)});

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
