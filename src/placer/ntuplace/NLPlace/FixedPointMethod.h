#ifndef _FixedPointMethod_
#define _FixedPointMethod_
#include "placedb.h"
#include <vector>

class CFixedPointMethod
{
    public:
	CPlaceDB& m_placedb;

	std::vector<Module> m_orig_modules;
	std::vector<Net> m_orig_nets;
	std::vector<Pin> m_orig_pins;

	//Add a pseudo module and a pin on the given position
	//return the added pin id
	int AddPseudoPinAndModule( const CPoint& p );
	//Add a pseudo net between the given pins
	void AddPseudoNet( const int& i1, const int& i2 );

	//Add a pseudo pin on the specified module
	//return the added pin id
	int AddPseudoPinOnModule( const int& moduleId, const CPoint& p );
	
    public:
	CFixedPointMethod(CPlaceDB& placedb);
	~CFixedPointMethod(void){}

	void AddAllPseudoModuleAndNet( void );
	void RestoreAllPseudoModuleAndNet( void );
};

#endif
