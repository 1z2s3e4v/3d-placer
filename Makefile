CC=g++
CG=gcc
#LIB=src/lib/liblpsolve55.a
LDFLAGS= -g -std=c++11 -lm -fomit-frame-pointer -funroll-loops -ffast-math -funit-at-a-time -fPIC -DNDEBUG -pthread
CLDFLAGS=-fomit-frame-pointer -funroll-loops -ffast-math -funit-at-a-time -fPIC -DYY_NEVER_INTERACTIVE -DPARSER_LP -DINVERSE_ACTIVE=INVERSE_LUSOL -DRoleIsExternalInvEngine
SOURCES=src/main/main.cpp src/parser/parser.cpp src/dataModel/dm.cpp src/dataModel/module.cpp src/dataModel/bin.cpp src/utility/paramHdl.cpp src/utility/message.cpp src/utility/drawHtml.cpp src/utility/aux.cpp src/utility/hgr.cpp src/placer/placer.cpp \
		src/placer/ntuplace/PlaceCommon/util.cpp src/placer/ntuplace/PlaceCommon/placebin.cpp src/placer/ntuplace/PlaceCommon/placeutil.cpp src/placer/ntuplace/PlaceCommon/ParamPlacement.cpp src/placer/ntuplace/PlaceDB/placedb.cpp \
		src/placer/ntuplace/Cluster/fccluster.cpp src/placer/ntuplace/Cluster/cluster.cpp src/placer/ntuplace/PlaceCommon/randomplace.cpp src/placer/ntuplace/Legalizer/TetrisLegal.cpp src/placer/ntuplace/NLPlace/mlnlp.cpp \
		src/placer/ntuplace/NLPlace/smooth.cpp src/placer/ntuplace/PlaceCommon/arghandler.cpp src/placer/ntuplace/PlaceCommon/lap.cpp src/placer/ntuplace/PlaceCommon/lap_wrapper.cpp src/placer/ntuplace/PlaceCommon/BlockMatching.cpp src/placer/ntuplace/lib/hlbfgs/HLBFGS_BLAS.cpp src/placer/ntuplace/lib/hlbfgs/LineSearch.cpp src/placer/ntuplace/lib/hlbfgs/ICFS.cpp src/placer/ntuplace/lib/hlbfgs/HLBFGS.cpp \
		src/placer/ntuplace/NLPlace/MyNLP.cpp src/partitioner/partitioner.cpp
LP=src/placer/ntuplace/lib/lpsolve55
#CSOURCES=$(wildcard src/placer/ntuplace/lib/lpsolve55/*.c)
CSOURCES=${LP}/colamd.c ${LP}/lp_Hash.c ${LP}/lp_mipbb.c ${LP}/lp_pricePSE.c ${LP}/lp_SOS.c ${LP}/myblas.c ${LP}/commonlib.c ${LP}/lp_lib.c ${LP}/lp_MPS.c ${LP}/lp_report.c ${LP}/lp_utils.c ${LP}/yacc_read.c ${LP}/fortify.c ${LP}/lp_LUSOL.c ${LP}/lp_params.c ${LP}/lp_rlp.c ${LP}/lp_wlp.c ${LP}/ini.c ${LP}/lp_matrix.c ${LP}/lp_presolve.c ${LP}/lp_scale.c ${LP}/lusol.c ${LP}/lp_crash.c ${LP}/lp_MDO.c ${LP}/lp_price.c ${LP}/lp_simplex.c ${LP}/mmio.c
OBJECTS=$(SOURCES:.cpp=.o)
COBJECTS=$(CSOURCES:.c=.o)
EXECUTABLE=3d-placer
INCLUDES=src/parser/parser.h src/dataModel/dm.h src/dataModel/module.h src/dataModel/bin.h src/utility/paramHdl.h src/utility/message.h src/utility/drawHtml.h src/utility/aux.h src/utility/hgr.h src/utility/color.h src/placer/placer.h \
		src/placer/ntuplace/PlaceCommon/util.h src/placer/ntuplace/PlaceCommon/placebin.h src/placer/ntuplace/PlaceCommon/placeutil.h src/placer/ntuplace/PlaceCommon/ParamPlacement.h src/placer/ntuplace/PlaceDB/placedb.h \
		src/placer/ntuplace/Cluster/fccluster.h src/placer/ntuplace/Cluster/cluster.h src/placer/ntuplace/PlaceCommon/randomplace.h src/placer/ntuplace/Legalizer/TetrisLegal.h src/placer/ntuplace/NLPlace/mlnlp.h \
		src/placer/ntuplace/NLPlace/smooth.h  src/placer/ntuplace/PlaceCommon/arghandler.h src/placer/ntuplace/PlaceCommon/lap.h src/placer/ntuplace/PlaceCommon/lap_wrapper.h src/placer/ntuplace/PlaceCommon/BlockMatching.h src/placer/ntuplace/lib/hlbfgs/HLBFGS_BLAS.h src/placer/ntuplace/lib/hlbfgs/LineSearch.h src/placer/ntuplace/lib/hlbfgs/ICFS.h src/placer/ntuplace/lib/hlbfgs/HLBFGS.h \
		src/placer/ntuplace/NLPlace/MyNLP.h src/partitioner/cell.h src/partitioner/net.h src/partitioner/partitioner.h
#CINCLUDES=$(wildcard src/placer/ntuplace/lib/lpsolve55/*.h)
CINCLUDES=${LP}/colamd.h ${LP}/lp_Hash.h ${LP}/lp_mipbb.h ${LP}/lp_pricePSE.h ${LP}/lp_SOS.h ${LP}/myblas.h ${LP}/commonlib.h ${LP}/lp_lib.h ${LP}/lp_MPS.h ${LP}/lp_report.h ${LP}/lp_utils.h ${LP}/yacc_read.h ${LP}/fortify.h ${LP}/lp_LUSOL.h ${LP}/lp_params.h ${LP}/lp_rlp.h ${LP}/lp_wlp.h ${LP}/ini.h ${LP}/lp_matrix.h ${LP}/lp_presolve.h ${LP}/lp_scale.h ${LP}/lusol.h ${LP}/lp_crash.h ${LP}/lp_MDO.h ${LP}/lp_price.h ${LP}/lp_simplex.h ${LP}/mmio.h

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
