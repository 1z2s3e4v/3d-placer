CC=g++
CG=gcc
#LIB=src/lib/liblpsolve55.a
LDFLAGS= -O3 -std=c++11 -lm -fomit-frame-pointer -funroll-loops -ffast-math -funit-at-a-time -fPIC -DNDEBUG -pthread
CLDFLAGS=-fomit-frame-pointer -funroll-loops -ffast-math -funit-at-a-time -fPIC -DYY_NEVER_INTERACTIVE -DPARSER_LP -DINVERSE_ACTIVE=INVERSE_LUSOL -DRoleIsExternalInvEngine
SOURCES=src/main/main.cpp src/parser/parser.cpp src/dataModel/dm.cpp src/dataModel/module.cpp src/dataModel/bin.cpp src/utility/paramHdl.cpp src/utility/message.cpp src/utility/drawHtml.cpp src/utility/aux.cpp src/utility/hgr.cpp src/placer/placer.cpp src/partitioner/partitioner.cpp
OBJECTS=$(SOURCES:.cpp=.o)
COBJECTS=$(CSOURCES:.c=.o)
EXECUTABLE=3d-placer
INCLUDES=src/parser/parser.h src/dataModel/dm.h src/dataModel/module.h src/dataModel/bin.h src/utility/paramHdl.h src/utility/message.h src/utility/drawHtml.h src/utility/aux.h src/utility/hgr.h src/utility/color.h src/placer/placer.h src/partitioner/cell.h src/partitioner/net.h src/partitioner/partitioner.h

all: $(CSOURCES) $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(COBJECTS) $(OBJECTS) 
	$(CC) -static $(LDFLAGS) $(COBJECTS) $(OBJECTS) $(LIB) -ldl -o $@
	mkdir -p output

.c.o: %.c ${CINCLUDES}
	$(CG) -static $(CLDFLAGS) $< -c -o $@
.cpp.o: %.cpp ${INCLUDES}
	$(CC) -static $(LDFLAGS) $< -c -o $@

t:
	bin/$(EXECUTABLE) testcase/toycase2.txt output/toycase2_output.txt
d:
	bin/$(EXECUTABLE) -d testcase/toycase2.txt output/toycase2_output.txt
c1:
	bin/$(EXECUTABLE) testcase/case1.txt output/case1_output.txt
c2: 
	bin/$(EXECUTABLE) testcase/case2.txt output/case2_output.txt
c3: 
	bin/$(EXECUTABLE) testcase/case3.txt output/case3_output.txt
c4: 
	bin/$(EXECUTABLE) testcase/case4.txt output/case4_output.txt
e1:
	evaluator/evaluator testcase/case1.txt output/case1_output.txt |& tee evaluator/case1_eva.log
e2:
	evaluator/evaluator testcase/case2.txt output/case2_output.txt |& tee evaluator/case2_eva.log
e3:
	evaluator/evaluator testcase/case3.txt output/case3_output.txt |& tee evaluator/case3_eva.log
e4:
	evaluator/evaluator testcase/case4.txt output/case4_output.txt |& tee evaluator/case4_eva.log

clean:
	rm -rf *.o src/*/*.o src/*/*/*/*.o src/*/*/*/*/*.o bin/$(EXECUTABLE) *.out bin/*.out log
