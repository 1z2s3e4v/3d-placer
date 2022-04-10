CC=g++
LDFLAGS=-std=c++11 -lm -g
SOURCES=src/main/main.cpp src/parser/parser.cpp src/dataModel/dm.cpp src/dataModel/module.cpp src/utility/paramHdl.cpp src/utility/message.cpp src/utility/drawHtml.cpp src/utility/aux.cpp src/placer/D2Dplacer.cpp src/placer/placer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=3d-placer
INCLUDES=src/parser/parser.h src/dataModel/dm.h src/dataModel/module.h src/utility/paramHdl.h src/utility/message.h src/utility/drawHtml.h src/utility/aux.h src/utility/color.h src/placer/D2Dplacer.h src/placer/placer.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	mkdir -p output

%.o:  %.cpp ${INCLUDES}
	$(CC) $(LDFLAGS) $< -c -o $@

t:
	bin/$(EXECUTABLE) testcase/toycase2.txt output/toycase2_output.txt
d:
	bin/$(EXECUTABLE) -d testcase/toycase2.txt output/toycase2_output.txt
c1:
	bin/$(EXECUTABLE) testcase/case1.txt output/case1_output.txt
c2: 
	bin/$(EXECUTABLE) testcase/case2.txt output/case2_output.txt
e1:
	evaluator/evaluator testcase/case1.txt output/case1_output.txt |& tee output/case1_eva.txt
e2:
	evaluator/evaluator testcase/case2.txt output/case2_output.txt |& tee output/case2_eva.txt

clean:
	rm -rf *.o src/*/*.o bin/$(EXECUTABLE) *.out bin/*.out log
