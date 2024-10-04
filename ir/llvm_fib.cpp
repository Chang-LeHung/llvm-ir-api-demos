#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>


using namespace llvm;
using namespace std;

int main() {
   llvm::InitializeNativeTarget();
   llvm::InitializeNativeTargetAsmPrinter();
   auto ctx = std::make_unique<llvm::LLVMContext>();
   auto module = std::make_unique<llvm::Module>("demo", *ctx);

   auto fib_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx),
                                           llvm::Type::getInt32Ty(*ctx),
                                           false);
   auto func = llvm::Function::Create(fib_type,
                                      llvm::Function::ExternalLinkage,
                                      "fib",
                                      module.get());
   auto builder = IRBuilder(*ctx);
   auto entryBlock = BasicBlock::Create(*ctx, "entry", func);
   auto ifBlock = BasicBlock::Create(*ctx, "ifBlock", func);
   auto elseBlock = BasicBlock::Create(*ctx, "elseBlock", func);
   builder.SetInsertPoint(entryBlock);
   auto arg = func->arg_begin();
   auto c1 = builder.CreateICmpEQ(arg, builder.getInt32(0));
   auto c2 = builder.CreateICmpEQ(arg, builder.getInt32(1));
   auto c3 = builder.CreateOr(c1, c2);
   builder.CreateCondBr(c3, ifBlock, elseBlock);
   builder.SetInsertPoint(ifBlock);
   builder.CreateRet(arg);
   builder.SetInsertPoint(elseBlock);
   auto p1 = builder.CreateSub(arg, builder.getInt32(1));
   auto p2 = builder.CreateSub(arg, builder.getInt32(2));
   auto r1 = builder.CreateCall(func, p1);
   auto r2 = builder.CreateCall(func, p2);
   builder.CreateRet(builder.CreateAdd(r1, r2));
   if (verifyFunction(*func, &errs())) {
      errs() << "Error in function\n";
      exit(-1);
   }
   module->print(outs(), nullptr);
   auto JIT = orc::LLJITBuilder().create();
   if (!JIT) {
      errs() << "Error creating JIT\n";
      exit(-1);
   }
   auto mod = orc::ThreadSafeModule(std::move(module), std::move(ctx));
   auto &theJIT = *JIT;
   auto err = theJIT->addIRModule(std::move(mod));
   if (err) {
      errs() << err;
      exit(-1);
   }
   auto sym = theJIT->lookup("fib");
   auto fib = sym->toPtr<int(int )>();
   if (fib == nullptr) {
      cout << "null\n";
   }
   cout << fib(1) << "\n";
   return 0;
}