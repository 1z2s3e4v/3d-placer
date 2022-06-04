ADD_LIBRARY (Legalizer legal.cpp macrolegal.cpp mptreelegal.cpp Prelegalizer.cpp TetrisLegal.cpp DiamondLegalizer.cpp DiamondSearch.cpp boost_maxflow.cpp LpLegal.cpp SingleRowPlacement.cpp SpareCellInsertion.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/DetailPlace
	${PROJECT_SOURCE_DIR}/GlobalRouting
	${PROJECT_SOURCE_DIR}/MPTree
	${PROJECT_SOURCE_DIR}/NLPlace
	${PROJECT_SOURCE_DIR}/Parser
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
)
ADD_DEFINITIONS(-DPOWVFILE="/home/crazying/Placer/ntuplace/GlobalRouting/POWV9.dat")
ADD_DEFINITIONS(-DPOSTFILE="/home/crazying/Placer/ntuplace/GlobalRouting/POST9.dat")

# metis library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/metis)

# lpsolve library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/lpsolve55)
