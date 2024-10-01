

#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"


using namespace std;


void llvm_ir_fib() {
   auto context = std::make_unique<llvm::LLVMContext>();
}

int main() {
   auto context = std::make_unique<llvm::LLVMContext>();
   auto module = std::make_unique<llvm::Module>("main", *context);
   auto builder = std::make_unique<llvm::IRBuilder<>>(*context);


   vector<llvm::Type *> args;
   args.push_back(llvm::Type::getInt32Ty(*context));
   args.push_back(llvm::Type::getInt64Ty(*context));
   args.push_back(llvm::Type::getDoubleTy(*context));
   auto funcType = llvm::FunctionType::get(
       llvm::Type::getInt32Ty(*context), args, false);
   auto func = llvm::Function::Create(
       funcType, llvm::Function::ExternalLinkage, "main", module.get());
   auto ret = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 101);
   // create a basic block
   auto bb = llvm::BasicBlock::Create(*context, "entry", func);
   builder->SetInsertPoint(bb);
   // return first argument
   auto arg = func->arg_begin();
   builder->CreateRet(arg + 2);
   auto ok = verifyFunction(*func);
   module->print(llvm::outs(), nullptr);
   return 0;
}