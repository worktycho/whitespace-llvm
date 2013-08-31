#include "APInt.hxx"

#include <iostream>
#include <limits>
#include <vector>

extern "C" {
	void PushInstr(JITAPInt value);
	JITAPInt PopInstr();
	void PushCallStackInstr(long long int value);
	long long int PopCallStackInstr();
	JITAPInt AddInstr(JITAPInt value1,JITAPInt value2);
	JITAPInt MinusInstr(JITAPInt value1,JITAPInt value2);
	bool CmpEqualInstr(JITAPInt value1,JITAPInt value2);
	void OutputNumInstr(JITAPInt value);
	void OutputCharInstr(JITAPInt value);
}

void inline error(std::string msg) {
	std::cerr << msg << std::endl;
    exit(1);
}

std::vector<JITAPInt> stack;

void PushInstr(JITAPInt value) {
	stack.push_back(value);
}
JITAPInt PopInstr() {
	JITAPInt item = stack.back();
	stack.pop_back();
	return item;
}

std::vector<long long int> callstack;

void PushCallStackInstr(long long int value) {
	callstack.push_back(value);
}
long long int PopCallStackInstr() {
	long long int item = callstack.back();
	callstack.pop_back();
	return item;
}

JITAPInt AddInstr(JITAPInt value1,JITAPInt value2) {
	if (!value1.next && !value2.next) {
		return {value1.value + value2.value,nullptr};
	} else {
		//TODO: dosomething about overflows
		error("num to large");
	}
}
JITAPInt MinusInstr(JITAPInt value1,JITAPInt value2) {
	if (!value1.next && !value2.next) {
		return {value1.value - value2.value,nullptr};
	} else {
		//TODO: dosomething about overflows
		error("num to large");
	}
}
bool CmpEqualInstr(JITAPInt value1,JITAPInt value2) {
	return !JITAPIntlessthan(&value1,& value2) && !JITAPIntlessthan(& value2,&value1);
}
void OutputNumInstr(JITAPInt value) {
	if (!value.next) {
		std::cout << std::to_string(value.value);
	} else {
		//TODO: dosomething
		if (value.next) error("num to large");
	}
}
void OutputCharInstr(JITAPInt value) {
	if (value.next) error("Char to large");
	if (value.value > (long long int) std::numeric_limits<char>::max()) error("Char to large");
	std::cout << (char) value.value;
}
