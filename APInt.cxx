#include "APInt.hxx"

#include <iostream>
#include <valgrind/memcheck.h>


void inline error(std::string msg) {
	std::cerr << msg << std::endl;
    exit(1);
}

std::pair<long long int, long long int> lladdwithcarry(long long int value1, long long int value2) {
	if (!(value1 >= 0 && value2 >= 0)) error("negative num in add");
	if (STD_LONG_LONG_MAX - value1 >= value2) return std::pair<long long int, long long int>(0,value1+value2);
	long long int space = STD_LONG_LONG_MAX - value1;
	long long int result = value2 - space;
	return std::pair<long long int, long long int>(1,result);
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

JITAPInt JITAPIntTimes(JITAPInt value1,JITAPInt value2) {
	if (!value1.next && !value2.next) {
		if (value1.value == 0 || value2.value == 0 ) {
			return {0,nullptr};
		} else if ((STD_LONG_LONG_MAX / value1.value >= value2.value
  				|| STD_LONG_LONG_MAX / value1.value <= -value2.value)) {
			return {value1.value * value2.value,nullptr};
		} else {
			bool value1Neg, value2Neg;
			value1Neg = value1.value < 0;
			value2Neg = value2.value < 0;
			if (value1Neg) value1.value = -value1.value;
			if (value2Neg) value2.value = -value2.value;
			unsigned long long int unsignvalue1, unsignvalue2;
			unsignvalue1 = value1.value;
			unsignvalue2 = value2.value;
			unsigned long int lowerhalf1, lowerhalf2, upperhalf1, upperhalf2;
			lowerhalf1 = unsignvalue1 & LONG_LONG_LOWER_HALF;
			lowerhalf2 = unsignvalue2 & LONG_LONG_LOWER_HALF;
			upperhalf1 = (unsignvalue1 & LONG_LONG_UPPER_HALF) >> 32;
			upperhalf2 = (unsignvalue2 & LONG_LONG_UPPER_HALF) >> 32;
			unsigned long long int lowerlower,lowerupper,upperlower,upperupper;
			lowerlower = (unsigned long long int) lowerhalf1 * (unsigned long long int) lowerhalf2;
			lowerupper = (unsigned long long int) lowerhalf1 * (unsigned long long int) upperhalf2;
			upperlower = (unsigned long long int) upperhalf1 * (unsigned long long int) lowerhalf2;
			upperupper = (unsigned long long int) upperhalf1 * (unsigned long long int) upperhalf2;
			JITAPInt result;
			bool lowerlowertopbitset = (lowerlower & UNSIGN_LONG_LONG_HIGH_BIT) == UNSIGN_LONG_LONG_HIGH_BIT;
			result.value = lowerlower & STD_LONG_LONG_MAX;
			result.next = new JITAPInt;
			result.next->next = nullptr;
			result.next->value = lowerlowertopbitset ? 1 : 0;
			//32 bit shift and add lower upper
			lowerhalf1 = (lowerupper & LONG_LONG_LOWER_31) << 32;
			lowerhalf2 = (upperlower & LONG_LONG_LOWER_31) << 32;
			upperhalf1 = (lowerupper & UNSIGN_LONG_LONG_UPPER_33) >> 31;
			upperhalf2 = (upperlower & UNSIGN_LONG_LONG_UPPER_33) >> 31;
			std::pair<long long int, long long int> lowerhalfaddresult = lladdwithcarry(result.value,lowerhalf1);
			result.next->value += lowerhalfaddresult.first;
			lowerhalfaddresult = lladdwithcarry(lowerhalfaddresult.second,lowerhalf2);
			result.next->value += lowerhalfaddresult.first;
			result.value = lowerhalfaddresult.second;
			//cant overflow as 32 bit;
			result.next->value += upperhalf1;
			result.next->value += upperhalf2;
			//64 bit shift and add upper upper
			std::pair<long long int, long long int> upperhalfaddresult = lladdwithcarry(result.next->value,upperupper);
			result.next->value = upperhalfaddresult.second;
			if (upperhalfaddresult.first != 0) {
				result.next->next = new JITAPInt;
				result.next->next->next = nullptr;
				result.next->next->value = upperhalfaddresult.first;
			} else if (result.next->value == 0) {
				delete result.next;
				result.next = nullptr; 
			}
			bool neg = (value1Neg != value2Neg);
			if (neg) {
				JITAPInt *pos = &result;
				{
					pos->value = -pos->value;
				} while ( (pos = pos->next) != nullptr);
			}
		}
	} else {
		//TODO: dosomething about overflows
		error("input num to large in times");
	}
}
llvm::Constant* APInt::Serialize(std::shared_ptr<codegenState> state){
	llvm::Constant* tmp = this->Serializehelper(state,&this->value);
	VALGRIND_CHECK_VALUE_IS_DEFINED(tmp);
	return tmp;
}

llvm::Constant* APInt::Serializehelper(std::shared_ptr<codegenState> state, JITAPInt* num){
	llvm::Constant* child;
	VALGRIND_CHECK_VALUE_IS_DEFINED(state);
	VALGRIND_CHECK_VALUE_IS_DEFINED(state->TheModule);
	llvm::StructType* APIntType = state->TheModule->getTypeByName("JITAPInt");
	VALGRIND_CHECK_VALUE_IS_DEFINED(APIntType);
	if (num->next != nullptr) {
		child = Serializehelper(state,num->next);
	} else {
		child = llvm::ConstantPointerNull::get(APIntType->getPointerTo());
	}
	llvm::Constant* Value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),num->value,true);
	VALGRIND_CHECK_VALUE_IS_DEFINED(Value);
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

void APIntBuilder::multiplyby10() {
	this->value = JITAPIntTimes(this->value,JITAPInt({10,nullptr}));
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

void APIntBuilder::add(char value){
	if (!(this->value.value <= (STD_LONG_LONG_MAX - value))) {
		this->value.next = this->value.next ? this->value.next : new JITAPInt;
		this->value.value = (STD_LONG_LONG_MAX - value)  - this->value.value;
		this->addOne(this->value.next);
	} else {
		this->value.value += value;
	}
}

APIntBuilder::operator APInt() {
	return APInt(&this->value);
}

bool APIntBuilder::operator< (const APInt rhs) const{
	JITAPInt temp = rhs.GetJITAPInt();
	bool returnval = JITAPIntlessthan(&this->value,&temp);
	FreeJITAPInt(temp.next);
	return returnval;
}

APIntBuilder::~APIntBuilder() {
	//this->value is deleted with me
	FreeJITAPInt(this->value.next);
}




