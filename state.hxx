#ifndef state_hxx
#define state_hxx

#include <memory>
#include <map>

#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"

class codegenState;

#include "APInt.hxx"
#include "FlowControlNode.hxx"

template<class T>
void noop(T* ptr) {}

class codegenState {
	public:
		inline codegenState(): Builder(llvm::getGlobalContext()), TheModule(new llvm::Module("wspace-jit",llvm::getGlobalContext())) {}		

		llvm::Module* TheModule;
		llvm::IRBuilder<> Builder;
};

#endif
