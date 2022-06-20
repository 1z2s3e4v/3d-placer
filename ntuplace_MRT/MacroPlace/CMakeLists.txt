ADD_LIBRARY (MacroPlace macrosp.cpp macrocluster.cpp)
INCLUDE_DIRECTORIES (
${PROJECT_SOURCE_DIR}/PlaceCommon
${PROJECT_SOURCE_DIR}/PlaceDB
)

# lpsolve library
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/lpsolve55)
