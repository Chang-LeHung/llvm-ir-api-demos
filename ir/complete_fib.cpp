#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/Error.h>

using namespace llvm;

int main() {
   // 初始化 LLVM 上下文和模块
   LLVMContext Context;
   Module *module = new Module("fib_module", Context);
   IRBuilder<> Builder(Context);

   // 定义函数类型：int(int)
   FunctionType *fibFuncType = FunctionType::get(
       Builder.getInt32Ty(),
       {Builder.getInt32Ty()},
       false
   );

   // 创建函数并将其添加到模块中
   Function *fibFunction = Function::Create(
       fibFuncType,
       Function::ExternalLinkage,
       "fib",
       module
   );

   // 命名函数参数
   Function::arg_iterator args = fibFunction->arg_begin();
   Value *n = args++;
   n->setName("n");

   // 创建基本块
   BasicBlock *entryBB = BasicBlock::Create(Context, "entry", fibFunction);
   BasicBlock *ifBB = BasicBlock::Create(Context, "if_then", fibFunction);
   BasicBlock *elseBB = BasicBlock::Create(Context, "if_else", fibFunction);
   BasicBlock *mergeBB = BasicBlock::Create(Context, "merge", fibFunction);

   // 设置插入点到入口块
   Builder.SetInsertPoint(entryBB);

   // 比较 n < 2
   Value *cond = Builder.CreateICmpSLT(n, Builder.getInt32(2), "cond");
   // 创建条件跳转
   Builder.CreateCondBr(cond, ifBB, elseBB);

   // 构建 if_then 块
   Builder.SetInsertPoint(ifBB);
   // 在 if 块中直接跳转到 merge 块，并将 n 作为返回值
   Builder.CreateBr(mergeBB);

   // 构建 if_else 块
   Builder.SetInsertPoint(elseBB);
   // 计算 fib(n-1)
   Value *nMinus1 = Builder.CreateSub(n, Builder.getInt32(1), "nMinus1");
   std::vector<Value *> args1;
   args1.push_back(nMinus1);
   CallInst *fib_n1 = Builder.CreateCall(fibFunction, args1, "fib_n1");

   // 计算 fib(n-2)
   Value *nMinus2 = Builder.CreateSub(n, Builder.getInt32(2), "nMinus2");
   std::vector<Value *> args2;
   args2.push_back(nMinus2);
   CallInst *fib_n2 = Builder.CreateCall(fibFunction, args2, "fib_n2");

   // 计算 fib(n-1) + fib(n-2)
   Value *sum = Builder.CreateAdd(fib_n1, fib_n2, "sum");
   Builder.CreateBr(mergeBB);

   // 构建 merge 块
   Builder.SetInsertPoint(mergeBB);
   // 创建一个 phi 节点来合并 if 和 else 的返回值
   PHINode *phi = Builder.CreatePHI(Builder.getInt32Ty(), 2, "result");
   // if 块返回 n
   phi->addIncoming(n, ifBB);
   // else 块返回 sum
   phi->addIncoming(sum, elseBB);
   // 返回结果
   Builder.CreateRet(phi);

   // 验证生成的 IR 是否正确
   if (verifyFunction(*fibFunction, &errs())) {
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

   // 使用 ThreadSafeContext 包装 LLVMContext
   auto TSCtx = std::make_unique<llvm::orc::ThreadSafeContext>(std::make_unique<LLVMContext>());

   // 将模块和线程安全上下文传递给 ThreadSafeModule
   llvm::orc::ThreadSafeModule TSM(std::move(module), TSCtx);
   if (auto err = theJIT->addIRModule(std::move(TSM))) {
      errs() << "Failed to add module: " << toString(std::move(err)) << "\n";
      return 1;
   }

   // 查找 fib 函数的符号
   auto sym = theJIT->lookup("fib");
   if (!sym) {
      errs() << "Function not found: " << toString(sym.takeError()) << "\n";
      return 1;
   }

   // 获取函数指针
   auto fibPtr = (int(*)(int))sym->getAddress();

   // 调用 fib 函数
   int input = 10;
   int result = fibPtr(input);
   outs() << "fib(" << input << ") = " << result << "\n";

   module->print(outs(), nullptr);
   return 0;
}
