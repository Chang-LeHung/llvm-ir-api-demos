#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;
using namespace std;

int main() {
   // 初始化 LLVM 的目标架构
   llvm::InitializeNativeTarget();
   llvm::InitializeNativeTargetAsmPrinter();

   // 初始化 LLVM 上下文和模块
   auto TSCtx = std::make_unique<llvm::orc::ThreadSafeContext>(std::make_unique<LLVMContext>());
   auto Context = TSCtx->getContext();
   auto module = std::make_unique<Module>("add_module", *Context);
   IRBuilder<> Builder(*Context);

   // 创建函数类型：int(int a, int b)
   FunctionType *addFuncType = FunctionType::get(
       Builder.getInt32Ty(),
       {Builder.getInt32Ty(), Builder.getInt32Ty()},
       false
   );

   // 创建函数并将其添加到模块中
   Function *addFunction = Function::Create(
       addFuncType,
       Function::ExternalLinkage,
       "add",
       module.get()
   );

   // 命名函数参数
   auto args = addFunction->args();
   Value *a = addFunction->getArg(0);
   a->setName("a");
   Value *b = addFunction->getArg(1);
   b->setName("b");

   // 创建基本块
   BasicBlock *entryBB = BasicBlock::Create(*Context, "entry", addFunction);
   Builder.SetInsertPoint(entryBB);

   // 创建加法指令
   Value *sum = Builder.CreateAdd(a, b, "sum");

   // 返回计算结果
   Builder.CreateRet(sum);

   // 验证生成的 IR 是否正确
   if (verifyFunction(*addFunction, &errs())) {
      errs() << "Function verification failed!\n";
      return 1;
   }

   // 初始化 JIT 编译器
   auto jit = llvm::orc::LLJITBuilder().create();
   if (!jit) {
      errs() << "Failed to create LLJIT: " << toString(jit.takeError()) << "\n";
      return 1;
   }

   auto &theJIT = *jit;

   llvm::orc::ThreadSafeModule TSM(std::move(module), *TSCtx);
   if (auto err = theJIT->addIRModule(std::move(TSM))) {
      errs() << "Failed to add module: " << toString(std::move(err)) << "\n";
      return 1;
   }

   auto sym = theJIT->lookup("add");
   if (!sym) {
      errs() << "Function not found: " << toString(sym.takeError()) << "\n";
      return 1;
   }
   // 获取函数指针
   auto addPtr = sym->toPtr<int(int, int)>();

   // 调用 add 函数
   int aInput = 3;
   int bInput = 7;
   int result = addPtr(aInput, bInput);
   outs() << "add(" << aInput << ", " << bInput << ") = " << result << "\n";
   cout << addFunction->hasNUses(0);
   return 0;
}
