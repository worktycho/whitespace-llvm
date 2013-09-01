#ifndef DataFlowNode_hxx
#define DataFlowNode_hxx

class DataSourceNode;

#include "state.hxx"


class DataSourceNode {
public:
	static void createPushNode(std::shared_ptr<codegenState> state, llvm::Value* value);
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state) = 0;
};

class DataConsumerNode {
public:
	static llvm::Value* createPopNode(std::shared_ptr<codegenState> state);
	static void SaveStack(std::shared_ptr<codegenState> state);
};

class RegisterPushNode : public DataSourceNode {
	llvm::Value* value;
public:
	RegisterPushNode(llvm::Value* val) : value(val) {}
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state);
};

class StackPopNode : public DataConsumerNode {
public:
	static llvm::Value* createPopNode(std::shared_ptr<codegenState> state);
};

class StackPushNode : public DataSourceNode {
private:
	llvm::Value* value;
	bool Codegened;
public:
	StackPushNode(llvm::Value* val) : value(val), Codegened(false) {}
	void CodeGen(std::shared_ptr<codegenState> state);
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state){
		this->CodeGen(state);
		return StackPopNode::createPopNode(state);
	}
};

#endif
