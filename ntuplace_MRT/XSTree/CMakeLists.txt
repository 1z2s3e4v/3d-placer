ADD_LIBRARY (XSTree XSteiner.cpp dc.cpp edge.cpp error.cpp memory.cpp sort.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/PlaceCommon
)
ADD_DEFINITIONS (-DPOWVFILE="/home/donnie/ntuplace_200605/ntuplace/GlobalRouting/POWV9.dat")
ADD_DEFINITIONS (-DPOWVFILE="/home/donnie/ntuplace_200605/ntuplace/GlobalRouting/PORT9.dat")
