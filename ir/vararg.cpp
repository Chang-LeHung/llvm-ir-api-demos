#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

/*
 *
#include <stdio.h>
#include <stdarg.h>

int sum(int count, ...) {
    va_list args;
    int result = 0;

    va_start(args, count);
    for (int i = 0; i < count; i++) {
        result += va_arg(args, int);
    }
    va_end(args);

    return result;
}

int main() {
    printf("Sum is: %d\n", sum(3, 10, 20, 30));  // Should print 60
    return 0;
}

 */
int main() {
   LLVMContext context;
   Module *module = new Module("varargs_module", context);
   IRBuilder<> builder(context);

   // int sum(int count, ...);
   FunctionType *funcType = FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty()}, true);
   Function *sumFunc = Function::Create(funcType, Function::ExternalLinkage, "sum", module);

   // Create entry block for the function
   BasicBlock *entry = BasicBlock::Create(context, "entry", sumFunc);
   builder.SetInsertPoint(entry);

   // Get the first argument (count)
   auto args = sumFunc->arg_begin();
   Value *count = args++;
   count->setName("count");

   // Create a variable to store the result (initialize to 0)
   Value *result = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "result");
   builder.CreateStore(builder.getInt32(0), result);

   // va_list setup: allocate space for va_list
   Value *vaList = builder.CreateAlloca(builder.getInt8PtrTy(), nullptr, "va_list");

   // va_start equivalent in LLVM
   builder.CreateCall(Intrinsic::getDeclaration(module, Intrinsic::vastart), vaList);

   // Create the loop to sum the arguments
   BasicBlock *loopCond = BasicBlock::Create(context, "loopCond", sumFunc);
   BasicBlock *loopBody = BasicBlock::Create(context, "loopBody", sumFunc);
   BasicBlock *afterLoop = BasicBlock::Create(context, "afterLoop", sumFunc);

   // Initialize loop counter i = 0
   Value *i = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "i");
   builder.CreateStore(builder.getInt32(0), i);
   builder.CreateBr(loopCond);

   // Loop condition
   builder.SetInsertPoint(loopCond);
   Value *iVal = builder.CreateLoad(builder.getInt32Ty(), i);
   Value *cond = builder.CreateICmpSLT(iVal, count);
   builder.CreateCondBr(cond, loopBody, afterLoop);

   // Loop body: retrieve the next argument and add it to the result
   builder.SetInsertPoint(loopBody);
   Value *arg = builder.CreateVAArg(vaList, builder.getInt32Ty(), "arg");
   Value *sum = builder.CreateLoad(builder.getInt32Ty(), result);
   sum = builder.CreateAdd(sum, arg);
   builder.CreateStore(sum, result);

   // Increment i and jump to loop condition
   iVal = builder.CreateAdd(iVal, builder.getInt32(1));
   builder.CreateStore(iVal, i);
   builder.CreateBr(loopCond);

   // After the loop, call va_end and return the result
   builder.SetInsertPoint(afterLoop);
   builder.CreateCall(Intrinsic::getDeclaration(module, Intrinsic::vaend), vaList);
   Value *finalResult = builder.CreateLoad(builder.getInt32Ty(), result);
   builder.CreateRet(finalResult);

   // Print the module (LLVM IR)
   module->print(outs(), nullptr);

   delete module;
   return 0;
}
