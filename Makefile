
CXX=clang++
CXX_FLAGS=-c `/home/tycho/llvm-3.2-debug+asserts/Release+Asserts/bin/llvm-config --cxxflags --cppflags` -g -std=c++11 -fexceptions -Wno-unused-value


debug: main.o APInt.o tokeniser.o DataFlowNode.o JITSupport.o parser.o
	$(CXX) main.o APInt.o tokeniser.o DataFlowNode.o JITSupport.o parser.o -L/home/tycho/llvm-3.2-debug+asserts/Release+Asserts/lib `/home/tycho/llvm-3.2-debug+asserts/Release+Asserts/bin/llvm-config --libs core jit native ipo` `/home/tycho/llvm-3.2-debug+asserts/Release+Asserts/bin/llvm-config --ldflags` -rdynamic -g -O0 -o wspace-jit

main.o: main.cxx tokeniser.hxx parser.hxx state.hxx
	$(CXX) main.cxx $(CXX_FLAGS)

APInt.o: APInt.cxx APInt.hxx
	$(CXX) APInt.cxx $(CXX_FLAGS)

tokeniser.o: tokeniser.cxx tokeniser.hxx
	$(CXX) tokeniser.cxx $(CXX_FLAGS)

DataFlowNode.o: DataFlowNode.cxx DataFlowNode.hxx asserts.hxx
	$(CXX) DataFlowNode.cxx $(CXX_FLAGS)

JITSupport.o: JITSupport.cxx APInt.hxx
	$(CXX) JITSupport.cxx $(CXX_FLAGS)

parser.o: parser.cxx parser.hxx
	$(CXX) parser.cxx $(CXX_FLAGS)

#tokeniser.hxx: no deps

parser.hxx: tokeniser.hxx instruction.hxx APInt.hxx

#state.hxx: no deps

APInt.hxx: state.hxx

DataFlowNode.hxx: state.hxx asserts.hxx

#asserts.hxx: no deps

instruction.hxx: asserts.hxx APInt.hxx state.hxx DataFlowNode.hxx


