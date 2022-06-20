ADD_LIBRARY (QPPlacer PlaceDBQP.cpp qpforce.cpp solverQP.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
)

# laspack library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/laspack)
