ADD_LIBRARY (NLPlace mlnlp.cpp MyNLP.cpp smooth.cpp mlqp.cpp MyQP.cpp FixedPointMethod.cpp WhiteSpaceAllocation.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/Cluster
	${PROJECT_SOURCE_DIR}/GlobalRouting
	${PROJECT_SOURCE_DIR}/Legalizer
	${PROJECT_SOURCE_DIR}/MacroPlace
	${PROJECT_SOURCE_DIR}/MPTree
	${PROJECT_SOURCE_DIR}/Parser
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
	${PROJECT_SOURCE_DIR}/QPPlacer
	${PROJECT_SOURCE_DIR}/Timing
        ${PROJECT_SOURCE_DIR}/NLPlace
)

# liberty library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/liberty)

# lpsolve library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/lpsolve55)

# HLBFGS library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/hlbfgs)
