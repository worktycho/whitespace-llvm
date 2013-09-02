#include "APInt.hxx"

#include <iostream>
#include <limits>
#include <vector>

extern "C" {
	void PushInstr(JITAPInt value);
	JITAPInt PopInstr();
	void StoreInstr(JITAPInt address,JITAPInt value);
	JITAPInt RetriveInstr(JITAPInt address);
	JITAPInt AddInstr(JITAPInt value1,JITAPInt value2);
	JITAPInt MinusInstr(JITAPInt value1,JITAPInt value2);
	JITAPInt TimesInstr(JITAPInt value1,JITAPInt value2);
	bool CmpEqualInstr(JITAPInt value1,JITAPInt value2);
	void OutputNumInstr(JITAPInt value);
	void OutputCharInstr(JITAPInt value);
	JITAPInt ReadCharInstr();
	JITAPInt ReadNumInstr();
}

void inline error(std::string msg) {
	std::cerr << msg << std::endl;
    exit(1);
}

std::vector<JITAPInt> stack;

void PushInstr(JITAPInt value) {
	//std::cerr << "push " << (std::string) APInt(&value) << std::endl;
	stack.push_back(value);
}
JITAPInt PopInstr() {
	if (stack.size() == 0) error("can't pop empty stack");
	JITAPInt item = stack.back();
	stack.pop_back();
	//std::cerr << "pop " << (std::string) APInt(&item) << std::endl;
	return item;
}

std::map<APInt,APInt> heap;

void StoreInstr(JITAPInt structaddress,JITAPInt value){
	APInt address = APInt(&structaddress);
	//std::cerr << "store " << (std::string) APInt(&value) << " at address "<<  (std::string) address << std::endl;
	heap.erase(address);
	heap.insert(std::pair<APInt,APInt>(address,APInt(&value)));
}
JITAPInt RetriveInstr(JITAPInt structaddress){
	APInt address(&structaddress);
	//std::cerr << "retrive from " << (std::string) address << std::endl;
	//returns 0 (false) when there is no item present
	if (!heap.count(address)) {
		//return {0,nullptr};
		error(((std::string)"no value at address ").append(address));
	}
	return heap.at(address).GetJITAPInt();
}





JITAPInt AddInstr(JITAPInt value1,JITAPInt value2) {
	if (!value1.next && !value2.next) {
		if(value1.value >= 0 && value2.value >= 0 && STD_LONG_LONG_MAX - value1.value >= value2.value) {
			return {value1.value + value2.value,nullptr};
		} else {
			error("num to large in add");
		}
	} else {
		//TODO: dosomething about overflows
		error("num to large in add");
	}
}
JITAPInt MinusInstr(JITAPInt value1,JITAPInt value2) {
	if (!value1.next && !value2.next) {
		//by adding value 2 to the minium you get the smallest number
		//that value 2 can be subtracted from
		if(value2.value >= 0 && value1.value >= ((-STD_LONG_LONG_MAX)-1)+ value2.value) {
			return {value1.value - value2.value,nullptr};
		} else if (value2.value < 0){
			return AddInstr(value1,JITAPInt({-value2.value,nullptr}));
		} else {
			error("num to large in minus (single)");
		}
	} else {
		//TODO: dosomething about overflows
		error("num to large in minus (pointer)");
	}
}


JITAPInt TimesInstr(JITAPInt value1,JITAPInt value2) {
	return JITAPIntTimes(value1,value2);
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
JITAPInt ReadCharInstr(){
	char ch;
	std::cin >> ch;
	return {ch,nullptr};
}
	
JITAPInt ReadNumInstr(){
	char ch;
	APIntBuilder builder;
	if (std::cin.peek() < '0' || std::cin.peek() > '9') error("not a num");
	while (std::cin.peek() >= '0' && std::cin.peek() <= '9') {
		builder.multiplyby10();
		std::cin >> ch;
		ch -= '0';
		builder.add(ch);
	}
	return ((APInt)builder).GetJITAPInt();
}

