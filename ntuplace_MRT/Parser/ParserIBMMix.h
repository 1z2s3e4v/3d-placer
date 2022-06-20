#ifndef PARSER_IBM_MIX_H
#define PARSER_IBM_MIX_H

#include "placedb.h"

class CParserIBMMix
{
public:
	CParserIBMMix(void);
	~CParserIBMMix(void);
	int ReadFile( const char* file, CPlaceDB &fplan  );
	int ReadPLFile( const char *file, CPlaceDB& fplan );
	//Start:=====================(indark)==========================
	int ReadFile( const char* file, CPlaceDB &fplan ,const char* predef_nodefile  );
	//End:=====================(indark)==========================
private:
	int ReadNodesFile( const char *file, CPlaceDB& fplan );
	int ReadNetsFile( const char *file, CPlaceDB& fplan );
	int ReadSCLFile( const char *file, CPlaceDB& fplan );

	int stepNode;
	int stepNet;
};

#endif

