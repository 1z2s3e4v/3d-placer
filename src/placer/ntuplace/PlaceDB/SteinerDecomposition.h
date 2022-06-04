#ifndef _STEINER_DECOMPOSITION_
#define _STEINER_DECOMPOSITION_
#include "placedb.h"

namespace Jin
{
class CStoredModuleData
{
    public:
	int m_nPinsId;
	vector<int> m_netsId;
	
	CStoredModuleData( const Module& m );
	~CStoredModuleData(void){}
};
}

class CSteinerDecomposition
{
    //Member variables
    private:
	CPlaceDB& m_placedb;
	std::vector< Jin::CStoredModuleData > m_module_data;	
    public: 
	const int m_orig_nModules;
    private:
	const int m_orig_nPins;
	const std::vector<Net> m_orig_nets;
	//std::vector<int> m_orig_nPins_in_module;
    
    //Member functions
    private:
	//Add a pseudo-pin and a pseudo-module at the given ABSOLUTE position p
	//Return the added pin index
	int AddPseudoPinAndModule( const CPoint& p );

	//Add a pseudo-pin on a pseudo-module at the given ABSOLUTE position p
	//Return the added pin index
	int AddPseudoPinOnModule( const int& moduleId, const CPoint& p );

	//Add a net connecting pin i1 and i2
	//Also update the module information (m_netsId)
	//DO NOT return the net id since it may not add a new net
	//(won't add a new net if pin i1 and i2 lay on the same module)
	void AddPseudoNet( const int& i1, const int& i2 );	

    public:
	CSteinerDecomposition( CPlaceDB& placedb );
	~CSteinerDecomposition(void){}

	//Restore original placedb
	void Restore(void);

	//Decomposite nets to two-pin nets
	void Update(void);
};

#endif
