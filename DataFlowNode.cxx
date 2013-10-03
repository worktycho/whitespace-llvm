#include "DataFlowNode.hxx"

#include <iostream>

#include "asserts.hxx"
#include <valgrind/memcheck.h>
void DataSourceNode::createPushNode(std::shared_ptr<codegenState> state, llvm::Value* value){
	VALGRIND_CHECK_VALUE_IS_DEFINED(value);
	state->stack.push_back(std::shared_ptr<RegisterPushNode>(new RegisterPushNode(value)));
}
void DataSourceNode::createMemStore(std::shared_ptr<codegenState> state, llvm::Value* address, llvm::Value* value){
	if (llvm::isa<llvm::Constant>(address)) {
		state->cache[address] = std::shared_ptr<DataSourceNode>(new MemoryStoreNode(value));
	} else {
		state->cache.clear();
	}
	assertdefined(state->TheModule)
	assertdefined(address)
	assertdefined(value)
	llvm::Function* StoreInstr = state->TheModule->getFunction("StoreInstr");
	assertdefined(StoreInstr)
	state->Builder.CreateCall2(StoreInstr,address,value);
}
llvm::Value* DataConsumerNode::createPopNode(std::shared_ptr<codegenState> state) {
	if (state->stack.size() != 0) {
		std::shared_ptr<DataSourceNode> item = state->stack.back();
		assertdefined(item)
		state->stack.pop_back();
		llvm::Value* result = item->getValue(state);
		assertdefined(result)
		return result;
	} else {
		return StackPopNode::createPopNode(state);
	}
}

llvm::Value* DataConsumerNode::createMemRetrive(std::shared_ptr<codegenState> state, llvm::Value* address) {
	if (!state->cache.count(address)) {
		assertdefined(address)
		llvm::Function* RetriveInstr = state->TheModule->getFunction("RetriveInstr");
		assertdefined(RetriveInstr)
		llvm::Value* value = state->Builder.CreateCall(RetriveInstr,address,"retrivedval");
		assertdefined(value)
		return value;
	}
	return state->cache.at(address)->getValue(state);
}

void DataConsumerNode::SaveStack(std::shared_ptr<codegenState> state, bool cleargeninfo) {
	for(std::shared_ptr<DataSourceNode>& item : state->stack) {
		StackPushNode pusher(item->getValue(state));
		pusher.CodeGen(state);
	}
	if (cleargeninfo) {
		state->stack.clear();
		state->cache.clear();
	}
}

void StackPushNode::CodeGen(std::shared_ptr<codegenState> state){
	if (this->Codegened) return; //only codegen once
	llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
	state->Builder.CreateCall(PushInstr, this->value);
	this->Codegened = true;
}

llvm::Value* RegisterPushNode::getValue(std::shared_ptr<codegenState> state) {
	llvm::Value* result = this->value;
	assertdefined(result)
	return result;
}

llvm::Value* StackPopNode::createPopNode(std::shared_ptr<codegenState> state) {
	llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
	llvm::Value* result = state->Builder.CreateCall(PopInstr,"popedval");
	assertdefined(result)
	return result;
}
