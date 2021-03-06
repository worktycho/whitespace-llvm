#ifndef state_hxx
#define state_hxx

#include <memory>
#include <map>

#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"

class codegenState;

class DataSourceNode;

class codegenState {
	public:
		inline codegenState(): Builder(llvm::getGlobalContext()), TheModule(new llvm::Module("wspace-jit",llvm::getGlobalContext())) {}		

		llvm::Module* TheModule;
		llvm::IRBuilder<> Builder;
		std::vector<std::shared_ptr<DataSourceNode>> stack;
		std::map<llvm::Value*,std::shared_ptr<DataSourceNode>> cache;
};

#endif
