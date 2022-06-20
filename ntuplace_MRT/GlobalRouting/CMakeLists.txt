ADD_LIBRARY (GlobalRouting bookshelf_IO.c flute.c  memAlloc.c  steiner.cpp MinimumSpanningTree.cpp GlobalRouting.cpp)
INCLUDE_DIRECTORIES (
${PROJECT_SOURCE_DIR}/lib/cgrip
${PROJECT_SOURCE_DIR}/Parser
${PROJECT_SOURCE_DIR}/PlaceCommon
${PROJECT_SOURCE_DIR}/PlaceDB
${PROJECT_SOURCE_DIR}/NLPlace
${PROJECT_SOURCE_DIR}/GlobalRouting
)
#ADD_DEFINITIONS(-DPOWVFILE="${PROJECT_SOURCE_DIR}/GlobalRouting/POWV9.dat")
#ADD_DEFINITIONS(-DPOSTFILE="${PROJECT_SOURCE_DIR}/GlobalRouting/POST9.dat")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --param inline-unit-growth=100")
