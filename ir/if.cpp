
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;

int main() {
   // 创建一个 LLVM 上下文、模块和 IRBuilder
   LLVMContext Context;
   Module *module = new Module("if-else example", Context);
   IRBuilder<> Builder(Context);

   // 创建一个返回类型为 void 的函数，带一个 i1 类型的参数（布尔条件）
   FunctionType *funcType = FunctionType::get(Type::getVoidTy(Context), {Type::getInt1Ty(Context)}, false);
   Function *func = Function::Create(funcType, Function::ExternalLinkage, "ifElseFunction", module);

   // 给函数创建一个入口块
   BasicBlock *entryBB = BasicBlock::Create(Context, "start", func);
   Builder.SetInsertPoint(entryBB);

   // 获取函数的第一个参数 (条件)
   Value *cond = func->getArg(0);

   // 创建 then, else, 和 merge 基本块
   BasicBlock *thenBB = BasicBlock::Create(Context, "then", func);
   BasicBlock *elseBB = BasicBlock::Create(Context, "else", func);
   BasicBlock *mergeBB = BasicBlock::Create(Context, "merge", func);

   // 创建条件跳转指令：根据 cond 选择跳转到 thenBB 或 elseBB
   Builder.CreateCondBr(cond, thenBB, elseBB);

   // 构建 thenBB 的内容
   Builder.SetInsertPoint(thenBB);
   // 在 then 块中可以插入你想要的代码，以下为一个示例
   Builder.CreateCall(func);  // 假设再调用一次这个函数
   Builder.CreateBr(mergeBB);  // 跳转到 mergeBB


   // 构建 elseBB 的内容
   Builder.SetInsertPoint(elseBB);
   // 在 else 块中可以插入你想要的代码，以下为一个示例
   Builder.CreateCall(func);  // 假设再调用一次这个函数
   Builder.CreateBr(mergeBB);  // 跳转到 mergeBB


   // 构建 mergeBB 的内容
   Builder.SetInsertPoint(mergeBB);
   Builder.CreateRetVoid();

   // 打印生成的 IR 代码
   module->print(outs(), nullptr);

   // 清理内存
   delete module;
   return 0;
}
