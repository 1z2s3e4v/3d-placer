#ifndef PARSER_BOOKSHELF_H
#define PARSER_BOOKSHELF_H

#include "Floorplan.h"

class CParserBookshelf
{

	friend class CFloorplan;

public:
	CParserBookshelf(void);
	~CParserBookshelf(void);
	
	int ReadFile(const char* fileBlocks , 
				const char* fileNets,
				const char* filePL,
				CFloorplan& fplan);

private:
	int ReadBlocksFile( const char* file, CFloorplan& fplan );
	int ReadNetsFile( const char* file, CFloorplan& fplan );
	int ReadPLFile( const char* file, CFloorplan& fplan );
};

#endif