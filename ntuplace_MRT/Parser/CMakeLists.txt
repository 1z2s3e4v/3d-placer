ADD_LIBRARY (Parser defrw.cpp lefdef.cpp lefrw.cpp ParserIBMMix.cpp)
INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/PlaceCommon
	${PROJECT_SOURCE_DIR}/PlaceDB
)

# lef & def libraries
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/lef)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/lib/def)
