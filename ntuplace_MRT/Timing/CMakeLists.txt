ADD_LIBRARY (Timing liberty.cpp heap.cpp netlist.cpp sta.cpp timing.cpp CriticalPath.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/Parser
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
)

# liberty library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/liberty)
