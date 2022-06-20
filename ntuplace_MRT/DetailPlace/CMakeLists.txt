ADD_LIBRARY (DetailPlace bbcellswap.cpp bbsolve.cpp cellmoving.cpp detail.cpp DPlace.cpp CellSlidingForCongestion.cpp CongMatching.cpp)
INCLUDE_DIRECTORIES (
${PROJECT_SOURCE_DIR}/GlobalRouting
${PROJECT_SOURCE_DIR}/Parser
${PROJECT_SOURCE_DIR}/PlaceCommon
${PROJECT_SOURCE_DIR}/PlaceDB
)
