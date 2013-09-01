#include "DataFlowNode.hxx"

void DataSourceNode::createPushNode(std::shared_ptr<codegenState> state, llvm::Value* value){
	state->stack.push_back(std::shared_ptr<RegisterPushNode>(new RegisterPushNode(value)));
}

llvm::Value* DataConsumerNode::createPopNode(std::shared_ptr<codegenState> state) {
	if (state->stack.size() != 0) {
		std::shared_ptr<DataSourceNode> item = state->stack.back();
		state->stack.pop_back();
		return item->getValue(state);
	} else {
		return StackPopNode::createPopNode(state);
	}
}

void DataConsumerNode::SaveStack(std::shared_ptr<codegenState> state) {
	for(std::shared_ptr<DataSourceNode>& item : state->stack) {
		StackPushNode pusher(item->getValue(state));
		pusher.CodeGen(state);
	}
	state->stack.clear();
}

void StackPushNode::CodeGen(std::shared_ptr<codegenState> state){
	if (this->Codegened) return; //only codegen once
	llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
	state->Builder.CreateCall(PushInstr, this->value);
	this->Codegened = true;
}

llvm::Value* RegisterPushNode::getValue(std::shared_ptr<codegenState> state) {
	return this->value;
}

llvm::Value* StackPopNode::createPopNode(std::shared_ptr<codegenState> state) {
		llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
		return state->Builder.CreateCall(PopInstr,"popedval"); 
}
