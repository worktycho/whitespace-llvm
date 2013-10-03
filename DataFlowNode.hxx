#ifndef DataFlowNode_hxx
#define DataFlowNode_hxx

class DataSourceNode;

#include "state.hxx"
#include "asserts.hxx"


class DataSourceNode {
//clang inheritance seems buggy
/*protected:
	llvm::Value* value;
public:
	DataSourceNode(llvm::Value* val) : value(val) { assertdefined(this->value)}*/
public:
	static void createPushNode(std::shared_ptr<codegenState> state, llvm::Value* value);
	static void createMemStore(std::shared_ptr<codegenState> state, llvm::Value* address, llvm::Value* value);
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state) = 0;
};

class DataConsumerNode {
public:
	static llvm::Value* createPopNode(std::shared_ptr<codegenState> state);
	static llvm::Value* createMemRetrive(std::shared_ptr<codegenState> state, llvm::Value* address);
	static void SaveStack(std::shared_ptr<codegenState> state, bool cleargeninfo = true);
};

class RegisterPushNode : public DataSourceNode {
private:
	llvm::Value* value;
public:
	RegisterPushNode(llvm::Value* val) : value(val) {VALGRIND_CHECK_VALUE_IS_DEFINED(this->value);}
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state);
};

class StackPopNode : public DataConsumerNode {
public:
	static llvm::Value* createPopNode(std::shared_ptr<codegenState> state);
};

class StackPushNode : public DataSourceNode {
private:
	bool Codegened;
	llvm::Value* value;
public: 
	StackPushNode(llvm::Value* val) : value(val), Codegened(false) { assertdefined(this->value)}
	void CodeGen(std::shared_ptr<codegenState> state);
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state){
		this->CodeGen(state);
		return StackPopNode::createPopNode(state);
	}
};

class MemoryStoreNode : public DataSourceNode {
private:
	llvm::Value* value;
public:
	MemoryStoreNode(llvm::Value* val) : value(val) { assertdefined(this->value)}
	virtual llvm::Value* getValue(std::shared_ptr<codegenState> state) {return(this->value);}
};

#endif
