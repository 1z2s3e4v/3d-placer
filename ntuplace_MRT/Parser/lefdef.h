#ifndef LEFDEF_H
#define LEFDEF_H

#include "placedb.h"

#include <vector>
#include <cstdio>
#include <map>
using namespace std;


class CParserLEFDEF
{
    public:
	enum DIRECTION { NONE, HORIZONTAL, VERTICAL };
	
    public:
	CParserLEFDEF();
	int ReadLEF( char* file1, CPlaceDB& fplan );	// in lefrw.cpp
	int ReadDEF( char* file1, CPlaceDB& fplan );	// in defrw.cpp
        int WriteDEF( const char* oldFile, const char* newFile, CPlaceDB& fplan );    // def out (donnie, 2005-10-20)
	
	int AddPin( int moduleId, string pinName, float xOff, float yOff ); 
	void PrintMacros( bool showPins );

	//private:
	vector<Module>  m_modules;
	vector<Pin>     m_pins;
	map<string,int> m_moduleNameMap;
	double		m_minBlockHeight;   // 2005-12-09
	double          m_coreSiteWidth;
	double	        m_coreSiteHeight;
	double		m_lefUnit;
	double		m_defUnit;
	int             m_nComponents;	    // # components in DEF
	vector<double>  m_metalPitch;	    // 2006-07-18 (donnie) for CongMap
	vector<DIRECTION>  m_metalDir;	    // 2006-09-06 (donnie) preferred direction
	vector<double>  m_metalWidth;	    // Added by Jin 20081007
	vector<double>  m_unitResistance;   // Added by Jin 20081007
	vector<double>  m_unitCapacitance;  // Added by Jin 20081007
	int             m_capHorizontal;
	int             m_capVertical;

    private:
	void CreateModuleNameMap();
	void WriteComponents( ostream& out, CPlaceDB& fplan );    // for def out (donnie, 2005-10-20)
};

extern FILE* fout;
extern int userData;
extern CPlaceDB* g_fplan;  // Pointer to the fplan object to read DEF

void dataError();
void* mallocCB(int size); 
void* reallocCB(void* name, int size);
void freeCB(void* name);
void lineNumberCB(int lineNo);
char* orientStr(int orient);
int   orientInt(char* orient);

#endif
