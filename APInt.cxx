#include "APInt.hxx"

#include <iostream>



void inline error(std::string msg) {
	std::cerr << msg << std::endl;
    exit(1);
}

void FreeJITAPInt(JITAPInt* value){
	if (!value) return;
	if (value->next) FreeJITAPInt(value->next);
	delete value;
	value = nullptr;
}

bool JITAPIntlessthan(const JITAPInt* const lhs, const JITAPInt* const rhs){
	if (!lhs) return false;
	if (!rhs) return false;
	//only lhs is null so shorter
	if (!lhs->next && rhs->next) return true;
	//only rhs is null so lhs is longer
	if (lhs->next && !rhs->next) return false;
	if (lhs->next && rhs->next) {
		if (!JITAPIntlessthan(lhs->next,rhs->next)) return false;
	}
	return lhs->value < rhs->value;
}


llvm::Constant* APInt::Serialize(std::shared_ptr<codegenState> state){
	return this->Serializehelper(state,&value);
}

llvm::Constant* APInt::Serializehelper(std::shared_ptr<codegenState> state, JITAPInt* num){
	llvm::Constant* child;
	llvm::StructType* APIntType = state->TheModule->getTypeByName("JITAPInt");
	if (num->next != nullptr) {
		child = Serializehelper(state,num->next);
	} else {
		child = llvm::ConstantPointerNull::get(APIntType->getPointerTo());
	}
	llvm::Constant* Value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),num->value,true);
	return llvm::ConstantStruct::get(APIntType,Value,child, nullptr);
}

APInt::operator std::string() const {
	if (!this->value.next) {
		return std::to_string(this->value.value);
	} else {
		//TODO: dosomething
		if (value.next) error("num to large");
	}
}

bool APInt::operator< (const APInt rhs) const{
	return JITAPIntlessthan(&this->value,&rhs.value);
}

APInt::APInt(JITAPInt* valuetocopy) {
	this->value.value = valuetocopy->value;
	JITAPInt *pos = &this->value;
	while((valuetocopy = valuetocopy->next)) {//while not null
		pos->next = new JITAPInt;
		pos = pos->next;
		pos->value = valuetocopy->value;
	}
	pos->next = nullptr; //null the last next
}

APInt::~APInt() {
	FreeJITAPInt(this->value.next);
}

void APIntBuilder::makeNegative(){
	JITAPInt *pos = &this->value;
	{
		pos->value = -pos->value;
	} while ( (pos = pos->next) != nullptr);
}

void APIntBuilder::doubleval(){
	//long long int defined as 64 bit in standard
	JITAPInt *pos = &this->value;
	bool carry = false; 
	{
		//make negative numbers tempoarliy positive so shifts are defined
		bool changedsign = (pos->value < 0);
		if (changedsign) pos->value = -pos->value;
		bool oldcarry = carry;
		//is the top bit set
		carry = (pos->value & LONG_LONG_HIGH_BIT) == LONG_LONG_HIGH_BIT;
		//discard top bit using a bit mask so left shift is defined
		pos->value &= ~LONG_LONG_HIGH_BIT;
		pos->value = pos->value << 1;
		if (oldcarry) this->addOne(pos);
		if (changedsign) pos->value = -pos->value;
	} while ( (pos = pos->next) != nullptr)
	if (carry) {
		pos->next = new JITAPInt;
		pos = pos->next;
		pos->value = 1;
		pos->next = nullptr;
	}
}

void APIntBuilder::addOne(){
	this->addOne(&this->value);
}

void APIntBuilder::addOne(JITAPInt *pos){
	while (!(pos->value < STD_LONG_LONG_MAX)) {
		pos->value = 0;
		pos = pos->next ? pos->next : new JITAPInt;
	}
	pos->value += 1;
}

APIntBuilder::operator APInt() {
	return APInt(&this->value);
}

APIntBuilder::~APIntBuilder() {
	//this->value is deleted with me
	FreeJITAPInt(this->value.next);
}




