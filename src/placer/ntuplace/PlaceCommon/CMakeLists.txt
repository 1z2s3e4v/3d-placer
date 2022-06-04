ADD_LIBRARY (PlaceCommon placeutil.cpp util.cpp ParamPlacement.cpp placebin.cpp verify.cpp randomplace.cpp CongMap.cpp arghandler.cpp GRouteMap.cpp lap.cpp lap_wrapper.cpp BlockMatching.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/GlobalRouting
	${PROJECT_SOURCE_DIR}/Parser
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
)
