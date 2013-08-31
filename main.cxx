#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>

#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/DataLayout.h"

#include "tokeniser.hxx"
#include "parser.hxx"
#include "state.hxx"
#include "FlowControlNode.hxx"

void usage();
void processfile(char* filename, bool listinstructions);
void initalcode(std::shared_ptr<codegenState> state);
std::shared_ptr<llvm::FunctionPassManager> FPMSetup(llvm::Module* TheModule, std::shared_ptr<llvm::ExecutionEngine> ExecutionEngine);

int main(int argc, char* argv[]) {
	//try {
	if (argc != 2 && argc != 3) {
		usage();
	} else {
		if (argc == 3) {
			if(strcmp(argv[1],"--list-instructions") == 0) {
				processfile(argv[2],true);
			} else if (strcmp(argv[2],"--list-instructions") == 0) {
				processfile(argv[1],true);		
			} else {
				usage();
			}
		} else {
			if(strcmp(argv[1],"--list-instructions") == 0) {
				usage();			
			} else {
				processfile(argv[1],false);
			}
		}
	}
	return 0; 
	/*} catch (std::exception& ex) {
		std::cout << "failed" << std::endl;
		return 1;	
	}*/
}

void usage() {
	std::cout << "wspace JIT" << std::endl;
	std::cout << "usage: wspace-jit --list-instructions [file]" << std::endl;
}

void processfile(char* filename, bool listinstructions){
	//ascii support only - change this stream to support unicode
	std::shared_ptr<std::ifstream> filehandle(new std::ifstream(filename));
	std::shared_ptr<tokeniser> tok(new tokeniser(filehandle));
	parser par(tok);
	std::shared_ptr<instruction> instruction;
	std::shared_ptr<codegenState> state(new codegenState());
//tempory
	initalcode(state);

	llvm::LLVMContext& context = llvm::getGlobalContext();
	llvm::FunctionType* VoidFunc = llvm::FunctionType::get(llvm::Type::getInt32Ty(context),false);
	llvm::Function* main = llvm::Function::Create(VoidFunc,llvm::Function::ExternalLinkage, "",state->TheModule);
	// Create a new basic block to start insertion into.
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(context, "entry", main);
	state->Builder.SetInsertPoint(BB);
	std::string ErrStr;
	llvm::InitializeNativeTarget();
	std::shared_ptr<llvm::ExecutionEngine> ExecutionEngine = std::shared_ptr<llvm::ExecutionEngine>(llvm::EngineBuilder(state->TheModule).setErrorStr(&ErrStr).create());
  	if (!ExecutionEngine) {
    	fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
    	exit(1);
  	}
	std::shared_ptr<llvm::FunctionPassManager> FPM = FPMSetup(state->TheModule,ExecutionEngine);
	while(par.TryGetNextInstruction(instruction)) {
		if (listinstructions) std::cout << instruction->name() << std::endl;
		instruction->Codegen(state);
	}
	state->CurrentNode = std::shared_ptr<FlowControlNode>(new StartNode());
	state->Builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),2));
	llvm::verifyFunction(*main);
	//FPM->run(*main);
	// JIT the function, returning a function pointer.
	void *FPtr = ExecutionEngine->getPointerToFunction(main);

	// Cast it to the right type (takes no arguments, returns void) so we
	// can call it as a native function.
	void (*FP)() = (void (*)())(intptr_t)FPtr;
	FP();
	state->TheModule->dump();
}

void initalcode(std::shared_ptr<codegenState> state){
	llvm::LLVMContext& context = llvm::getGlobalContext();
	llvm::StructType* APInt = llvm::StructType::create(context, "JITAPInt");
	llvm::Type* APIntElementsarr[] = {llvm::Type::getInt64Ty(context),APInt->getPointerTo()};
	llvm::ArrayRef<llvm::Type*> APIntElements(APIntElementsarr,2);
	APInt->setBody(APIntElements,"JITAPInt");
	
	llvm::ArrayRef<llvm::Type*> PushInstrArgs(APInt);
	llvm::FunctionType* PushInstrType = llvm::FunctionType::get(llvm::Type::getVoidTy(context),PushInstrArgs, false);
	llvm::Function::Create(PushInstrType, llvm::Function::ExternalLinkage, "PushInstr", state->TheModule);

	llvm::FunctionType* PopInstrType = llvm::FunctionType::get(APInt, false);
	llvm::Function::Create(PopInstrType, llvm::Function::ExternalLinkage, "PopInstr", state->TheModule);

	llvm::Type* AddInstrArgsarr[] = {APInt,APInt};
	llvm::ArrayRef<llvm::Type*> AddInstrArgs(AddInstrArgsarr, 2);
	llvm::FunctionType* AddInstrType = llvm::FunctionType::get(APInt,AddInstrArgs,false);
	llvm::Function::Create(AddInstrType, llvm::Function::ExternalLinkage, "AddInstr", state->TheModule);

	llvm::Function::Create(AddInstrType, llvm::Function::ExternalLinkage, "MinusInstr", state->TheModule);
	llvm::Function::Create(AddInstrType, llvm::Function::ExternalLinkage, "TimesInstr", state->TheModule);
	
	llvm::FunctionType* CmpZeroInstrType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context),AddInstrArgs, false);
	llvm::Function::Create(CmpZeroInstrType, llvm::Function::ExternalLinkage, "CmpEqualInstr", state->TheModule);

	llvm::Function::Create(PushInstrType, llvm::Function::ExternalLinkage, "OutputCharInstr", state->TheModule);

	llvm::Function::Create(PushInstrType, llvm::Function::ExternalLinkage, "OutputNumInstr", state->TheModule);

	llvm::ArrayRef<llvm::Type*> PushCallStackInstrArgs(llvm::Type::getInt64Ty(llvm::getGlobalContext()));
	llvm::FunctionType* PushCallStackInstrType = llvm::FunctionType::get(llvm::Type::getVoidTy(context),PushCallStackInstrArgs, false);
	llvm::Function::Create(PushCallStackInstrType, llvm::Function::ExternalLinkage, "PushCallStackInstr", state->TheModule);

	llvm::FunctionType* PopCallStackInstrType = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), false);
	llvm::Function::Create(PopCallStackInstrType, llvm::Function::ExternalLinkage, "PopCallStackInstr", state->TheModule);
}

std::shared_ptr<llvm::FunctionPassManager> FPMSetup(llvm::Module* TheModule, std::shared_ptr<llvm::ExecutionEngine> ExecutionEngine) {

	std::shared_ptr<llvm::FunctionPassManager> OurFPM(new llvm::FunctionPassManager(TheModule));

	// Set up the optimizer pipeline.  Start with registering info about how the
	// target lays out data structures.
	OurFPM->add(new llvm::DataLayout(*ExecutionEngine->getDataLayout()));
	// Provide basic AliasAnalysis support for GVN.
	OurFPM->add(llvm::createBasicAliasAnalysisPass());
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	OurFPM->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	OurFPM->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	OurFPM->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	OurFPM->add(llvm::createCFGSimplificationPass());

	OurFPM->doInitialization();

	// Set the global so the code gen can use this.
	return OurFPM;

}
