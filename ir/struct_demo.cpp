#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

int main() {
   // 初始化 LLVM 环境
   InitializeNativeTarget();
   InitializeNativeTargetAsmPrinter();

   LLVMContext context;
   IRBuilder<> builder(context);
   auto module = std::make_unique<Module>("struct_example", context);

   // 创建结构体类型
   std::vector<Type*> structElements;
   structElements.push_back(builder.getInt32Ty());  // int 类型字段
   structElements.push_back(builder.getInt8Ty());   // char 类型字段
   StructType* myStructType = StructType::create(structElements, "MyStruct");

   // 创建函数签名: void foo(MyStruct*)
   FunctionType* fooType = FunctionType::get(builder.getVoidTy(), {myStructType->getPointerTo()}, false);
   Function* fooFunc = Function::Create(fooType, Function::ExternalLinkage, "foo", module.get());

   // 创建函数主体
   BasicBlock* entryBlock = BasicBlock::Create(context, "entry", fooFunc);
   builder.SetInsertPoint(entryBlock);

   // 获取函数的第一个参数 (MyStruct*)
   Value* structPtr = fooFunc->arg_begin();

   // 从结构体指针中加载第一个字段 (int 类型)
   Value* firstFieldPtr = builder.CreateStructGEP(myStructType, structPtr, 0, "firstFieldPtr");
   Value* firstField = builder.CreateLoad(builder.getInt32Ty(), firstFieldPtr, "firstField");

   // 从结构体指针中加载第二个字段 (char 类型)
   Value* secondFieldPtr = builder.CreateStructGEP(myStructType, structPtr, 1, "secondFieldPtr");
   Value* secondField = builder.CreateLoad(builder.getInt8Ty(), secondFieldPtr, "secondField");

   // 这里你可以对字段进行操作，例如打印、加法运算等
   // 我们这里暂时做一个简单的打印操作
   Value *formatStr = builder.CreateGlobalStringPtr("First field = %d, Second field = %d\n");

   std::vector<Type *> printfArgs({builder.getInt8PtrTy()});
   FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), printfArgs, true);
   FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

   // 调用 printf 打印结构体的两个字段
   builder.CreateCall(printfFunc, {formatStr, firstField, secondField});

   // 结束函数
   builder.CreateRetVoid();

   // 创建 main 函数：int main()
   FunctionType* mainType = FunctionType::get(builder.getInt32Ty(), false);
   Function* mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());

   BasicBlock* mainEntry = BasicBlock::Create(context, "entry", mainFunc);
   builder.SetInsertPoint(mainEntry);

   // 在 main 函数中创建结构体并初始化
   Value* structAlloc = builder.CreateAlloca(myStructType, nullptr, "myStruct");

   // 给结构体的第一个字段赋值 (int 类型)
   Value* firstFieldAlloc = builder.CreateStructGEP(myStructType, structAlloc, 0, "firstFieldPtr");
   builder.CreateStore(builder.getInt32(123), firstFieldAlloc);

   // 给结构体的第二个字段赋值 (char 类型)
   Value* secondFieldAlloc = builder.CreateStructGEP(myStructType, structAlloc, 1, "secondFieldPtr");
   builder.CreateStore(builder.getInt8(65), secondFieldAlloc);  // ASCII 值 65 对应 'A'

   // 调用 foo 函数并传递结构体
   builder.CreateCall(fooFunc, {structAlloc});

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
