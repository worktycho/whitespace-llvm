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

std::pair<long long int, long long int> lladdwithcarry(long long int value1, long long int value2) {
	if (!(value1 >= 0 && value2 >= 0)) error("negative num in add");
	if (STD_LONG_LONG_MAX - value1 >= value2) return std::pair<long long int, long long int>(0,value1+value2);
	long long int space = STD_LONG_LONG_MAX - value1;
	long long int result = value2 - space;
	return std::pair<long long int, long long int>(1,result);
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
	if (!value1.next && !value2.next) {
		if (value1.value != 0 && (STD_LONG_LONG_MAX / value1.value >= value2.value
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
		error("input num to large times");
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
JITAPInt ReadCharInstr(){
	char ch;
	std::cin >> ch;
	return {ch,nullptr};
}
	
JITAPInt ReadNumInstr(){
	char ch;
	std::cin >> ch;
	//convert ascii code to num
	if (ch < '0' || ch > '9') error("not a num");
	ch -= '0';
	return {ch,nullptr};
}

