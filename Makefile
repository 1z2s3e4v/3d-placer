CC=g++
LDFLAGS=-std=c++11 -lm
SOURCES=src/main/main.cpp src/parser/parser.cpp src/dataModel/dm.cpp src/paramHandler/paramhdl.cpp src/utility/message.cpp src/utility/drawHtml.cpp src/placer/3Dplacer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=3d-placer
INCLUDES=src/parser/parser.h src/dataModel/dm.h src/paramHandler/paramhdl.h src/utility/message.h src/utility/drawHtml.h src/placer/3Dplacer.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.cpp ${INCLUDES}
	$(CC) $(LDFLAGS) $< -c -o $@

t:
	mkdir -p output
	bin/$(EXECUTABLE) testcase/toycase.txt output/toycase_output.txt
c1:
	mkdir -p output
	bin/$(EXECUTABLE) testcase/case1.txt output/case1_output.txt
c2: 
	mkdir -p output
	bin/$(EXECUTABLE) testcase/case2.txt output/case2_output.txt
e1:
	evaluator/evaluator testcase/case1.txt output/case1_output.txt |& tee output/case1_eva.txt
e2:
	evaluator/evaluator testcase/case2.txt output/case2_output.txt |& tee output/case2_eva.txt

clean:
	rm -rf *.o src/*/*.o bin/$(EXECUTABLE) *.out bin/*.out log
