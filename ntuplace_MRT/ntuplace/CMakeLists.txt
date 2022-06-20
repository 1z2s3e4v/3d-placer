SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__BUILD_DATE__=\\\"\"`LANG=;date`\"\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__BUILD_USER__=\\\"$(USER)\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__BUILD_HOST__=\\\"$(BUILD_HOST)\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__BUILD_ARCH__=\\\"\"`uname -mo`\"\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__CXXFLAGS__=\\\"'$(CXXFLAGS)'\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__LDFLAGS__=\\\"\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__CXX_VERSION__=\\\"\"`$(CXX) --version|head -n 1`\"\\\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__NTUPLACE_VERSION__=\\\"\"`date +V12.%m.%d`\"\\\"")

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/Cluster
	${PROJECT_SOURCE_DIR}/DetailPlace
	${PROJECT_SOURCE_DIR}/GlobalRouting
	${PROJECT_SOURCE_DIR}/Legalizer
	${PROJECT_SOURCE_DIR}/LibDB
	${PROJECT_SOURCE_DIR}/MacroPlace
	${PROJECT_SOURCE_DIR}/MinCutPlacer
	${PROJECT_SOURCE_DIR}/MPTree
	${PROJECT_SOURCE_DIR}/NLPlace
	${PROJECT_SOURCE_DIR}/Parser
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
	${PROJECT_SOURCE_DIR}/QPPlacer
	${PROJECT_SOURCE_DIR}/Timing
	${PROJECT_SOURCE_DIR}/XSTree
    ${PROJECT_SOURCE_DIR}/lib/liberty
)

# precompiled 64-bit libraries for ispd11 contest
LINK_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/cgrip)
LINK_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/cplex)

ADD_EXECUTABLE (ntuplace-r main.cpp)
TARGET_LINK_LIBRARIES (ntuplace-r NLPlace Legalizer Cluster DetailPlace GlobalRouting LibDB MacroPlace MPTree Parser PlaceCommon PlaceDB QPPlacer Timing XSTree)
TARGET_LINK_LIBRARIES (ntuplace-r laspack)
TARGET_LINK_LIBRARIES (ntuplace-r lpsolve55 dl)
TARGET_LINK_LIBRARIES (ntuplace-r metis)
TARGET_LINK_LIBRARIES (ntuplace-r def lef si2dr_liberty)
TARGET_LINK_LIBRARIES (ntuplace-r cgrip cplex)
TARGET_LINK_LIBRARIES (ntuplace-r m pthread)
TARGET_LINK_LIBRARIES (ntuplace-r hlbfgs)
