#include "macrosp.h"
#include <queue>
#include "lp_lib.h"
#include "ParamPlacement.h"



CMacroSP::~CMacroSP()
{
}

CMacroSP::CMacroSP( CPlaceDB& pDB )
{
	m_pDB = &pDB;
	m_MacroAreaRatio = 1000;
	gArg.GetDouble("MacroAreaRatio",&m_MacroAreaRatio);
	//if ( gArg.Parse("BoundSpace",&m_MacroAreaRatio) == false) m_BoundSpace = m_pDB->m_sites[0].m_height;
	m_BoundSpace  = 0.0;
	gArg.GetDouble("BoundSpace",&m_MacroAreaRatio);
	m_binSize = 50;
	gArg.GetInt("MPbinsize",&m_binSize);
	 m_binMacro2Xratio = 1.0;
	gArg.GetDouble("MP2Xratio",&m_binMacro2Xratio);
	m_cur_reject_rate = 0.0;
	m_state = 0;
	m_max_state = 5;
	gArg.GetInt("MPMaxState",&m_max_state);
	A = 0.4;
	B = 0.3;
	C = 0.5;
	D = 0.05;
	gArg.GetDouble("MPSACostA", &A);
	gArg.GetDouble("MPSACostB", &B);
	gArg.GetDouble("MPSACostC", &C);
	gArg.GetDouble("MPSACostD", &D);
	ite = 0;
	
}

void CMacroSP::Place( )
{

	int _mid = 0;
	if (gArg.GetInt("FIXED", &_mid))
		m_pDB->m_modules[_mid].m_isFixed = true;
	CalcAvgArea();
	NodePreparation( );
	CalcStdGrid();
	OutputMacroGrid("StdCell.dat",m_binStd);

	if (n_movable == 0) return; 
	InitSP();
	CGConstruct();
	HCG.CGAnalyze();
	VCG.CGAnalyze();
 	CornerPacking();
	CalcPinCenter();
	
//	OutputGraph("MacroInitCP.plt");
// 	printf("HCG\n");
	HCG.CGAnalyze();
// 	printf("VCG\n");
	VCG.CGAnalyze();
	RCXPacking();
	string name = param.outFilePrefix + "-MacroInit.plt";
	OutputGraph(name.c_str());
	CalcRCX();
	Packing(); 
	SA();
	ApplyBestSP();

	bool succ = true; // kaie
	if (gArg.CheckExist("MPCP")){
		CornerPacking();	
		string name2 = param.outFilePrefix + "-MacroCP.plt";
		OutputGraph(name2.c_str());
	}else{
		succ = MacroLP();	
		SolutionPacking();
		string name2 = param.outFilePrefix + "-MacroMP.plt";
		OutputGraph(name2.c_str());
	}
	if(succ)	// kaie
	    ApplyPlaceDB_CX(true);
	else
	    ApplyPlaceDB_CX(false);
// 	setMacroSpacing();
// 	MacroLP();
	
// 	OutputGraph("MacroLP.plt");
	
// 	CalcMacroGrid();
// 	cout << "CalcDeadSpaceCost:" << CalcDeadSpaceCost() << endl;
// 	cout << "CalcBoundCost:" << CalcBoundCost() << endl;
	
	
	
	
	
}

bool CMacroSP::isMacro( int _moduleID )
{
	//return 	(m_pDB->m_modules[_moduleID].m_area > m_MacroAreaRatio * m_avgArea ) && (!m_pDB->m_modules[_moduleID].m_isOutCore );
	return 	(m_pDB->m_modules[_moduleID].m_area > m_MacroAreaRatio * m_avgArea ) && (!m_pDB->BlockOutCore(_moduleID) );

}	


void CMacroSP::CalcAvgArea( )
{
	double _area = 0.0;
	int _count = 0;
	 //cout << "Fixed:" << m_pDB->m_modules[0].m_isFixed << "m_isOutCore:" << m_pDB->m_modules[0].m_isOutCore <<endl;
	
	for (unsigned int i = 0 ; i < m_pDB->m_modules.size(); i++){
		//if (!m_pDB->m_modules[i].m_isOutCore && !m_pDB->m_modules[i].m_isFixed){
		if (!m_pDB->BlockOutCore(i) && !m_pDB->m_modules[i].m_isFixed){
			_count++;
			_area += m_pDB->m_modules[i].m_area;
		}
			 
	}
	m_avgArea = _area / _count;
}

void CMacroSP::NodePreparation( )
{ 
	m_MacroList.clear();
	m_MacroSet.clear();
	n_movable = 0; 
	
	// 
	m_SQ_p.clear();
	m_SQ_n.clear();
	for (unsigned int i = 0 ; i < m_pDB->m_modules.size(); i++){
		if(isMacro( i)){
			m_NodeID2ModuleID[m_MacroList.size()] = i;
			m_ModuleID2NodeID[i] = m_MacroList.size();
// 			m_SQ_p.push_back(m_MacroList.size());
// 			m_SQ_n.push_back(m_MacroList.size());
			m_SQ_orient.push_back(m_pDB->m_modules[i].m_orient);			
			m_MacroList.push_back(i);
			m_MacroSet.insert(i);	
			if (!m_pDB->m_modules[i].m_isFixed)
				n_movable++;
		}
	}
	if (n_movable == 0) return; 
	printf (" AvgArea: %f\n", m_avgArea);
	printf (" # of macros: %d\n", m_MacroList.size());


	
	HCG.m_nodes.clear();
	VCG.m_nodes.clear();
	for (unsigned int j = 0 ; j < m_MacroList.size(); j++){
		Module _mod = m_pDB->m_modules[m_MacroList[j]];
		printf (" MID: %d NID:%d, %f %f %f\n", m_NodeID2ModuleID[j], j, _mod.m_width,_mod.m_height,_mod.m_area);
		HCG.AddNodes( CSPCGraphNode(_mod.m_x, _mod.m_width, _mod.m_isFixed , j) );
		VCG.AddNodes( CSPCGraphNode(_mod.m_y, _mod.m_height, _mod.m_isFixed , j) );
		
	}
	
	
 	
	HCG.AddNodes(CSPCGraphNode(m_pDB->m_coreRgn.right, 0.0 , true, HCG.m_nodes.size()));
	HCG.SetSink(HCG.m_nodes.size()-1);
	HCG.AddNodes( CSPCGraphNode(m_pDB->m_coreRgn.left, 0.0, true, HCG.m_nodes.size()));
	HCG.SetSource(HCG.m_nodes.size()-1);

	VCG.AddNodes(CSPCGraphNode(m_pDB->m_coreRgn.top, 0.0 , true, VCG.m_nodes.size()));
	VCG.SetSink(VCG.m_nodes.size()-1);
	VCG.AddNodes( CSPCGraphNode(m_pDB->m_coreRgn.bottom, 0.0, true, VCG.m_nodes.size()));
	VCG.SetSource(VCG.m_nodes.size()-1);
	
	
	HCG.SetBound( m_pDB->m_coreRgn.left , m_pDB->m_coreRgn.right);
	VCG.SetBound( m_pDB->m_coreRgn.bottom , m_pDB->m_coreRgn.top);
	
	
	m_maxHPWL = HCG.GetUpperBound() - HCG.GetLowerBound() + VCG.GetUpperBound() - VCG.GetLowerBound(); 
	  
	
	
	
}


CSPCGraphNode::CSPCGraphNode( double value, double length, bool fixed , int nodeID)
{
	m_value = value; 

	m_length = length;
	m_cx = value + length * 0.5;	//for plot only  HCG only!!
	m_rcx = m_cx;
	m_outedge.clear();
	m_inedge.clear();
	m_isFixed = fixed;
	m_nodeID = nodeID;
	if (fixed){
		m_Left = m_cx;
		m_Right = m_cx;
	}
}

CSPCGraph::CSPCGraph( )
{
	
}


void CSPCGraph::CGAddSourceSink()
{	
	
// 	int b= SINK;
// 	m_ModuleID2NodeID[b]=m_nodes.size() -1;
	for (unsigned int i = 0 ; i < m_nodes.size()-2;i++){	 // for all nodes except SINK
		if (m_nodes[i].m_outedge.size() == 0 ){
			AddEdge(m_nodes[i],m_nodes[m_sink]);
// 			printf ("SINK: %d\n", i);
		}
	}
// 	m_ModuleID2NodeID[(int)SINK] = m_nodes.size() -1;
// 	



// 	int a = SOURCE;
// 	m_ModuleID2NodeID[a]=m_nodes.size() -1;
	for (unsigned int i = 0 ; i < m_nodes.size()-2;i++){	 // for all nodes except SOURCE&SINK
		if (m_nodes[i].m_inedge.size() == 0 ){
			AddEdge(m_nodes[m_source],m_nodes[i]);
// 			printf ("SOURCE: %d\n", i);
			
		}
	}
// 	m_ModuleID2NodeID[(int)SOURCE] = m_nodes.size() -1;	




}

void CMacroSP::CGConstruct( )
{
	for (unsigned int i = 0 ; i < HCG.m_nodes.size() ; i++) {
		HCG.m_nodes[i].m_inedge.clear();
		VCG.m_nodes[i].m_inedge.clear();
		HCG.m_nodes[i].m_outedge.clear();
		VCG.m_nodes[i].m_outedge.clear();

	}
	
	
	///This is somewhat dirty O(n^3)
	for (unsigned int i = 0 ; i < m_SQ_p.size() ; i++) {
		int _n1 = m_SQ_p[i];
		for(unsigned j = i+1 ; j < m_SQ_p.size() ; j++ ){
			int _n2 = m_SQ_p[j];	
			// (.... n1 .... n2 )
			for(unsigned k = 0 ; k < m_SQ_n.size() ; k++ ){
				if ( m_SQ_n[k] != _n1 && m_SQ_n[k] != _n2)
					continue;
				if (  m_SQ_n[k] == _n1 ){	// ( ... n1 ... n2 ) (...n1.... n2 )
					// n1 is on the left of n2  ==> n1-> n2
					HCG.AddEdge(HCG.m_nodes[_n1],HCG.m_nodes[_n2]);
// 					printf("HCG: %d=>%d\n",_n1,_n2);
					break;
				}else{			// ( ... n1 ... n2 ) ( ...n2.... n1 )
					// n1 is below n2  ==> n1-> n2
					VCG.AddEdge(VCG.m_nodes[_n1],VCG.m_nodes[_n2]);
// 					printf("VCG: %d=>%d\n",_n1,_n2);
					break;
				}
			}
		}
	}
	
	
	HCG.CGAddSourceSink();	
	VCG.CGAddSourceSink();

	
	
	
	
}





double CSPCGraph::CGAnalyze( )
{
// 	double _bound_box= m_upper_bound -m_lower_bound;
	queue<CSPCGraphEdge> _queue;
	list<CSPCGraphEdge>::iterator ite;
	set<int> _nodeID_Set;
	vector<int> _nodeID_ED_count;
	_nodeID_ED_count.resize(m_nodes.size());
	//clean up
	for (unsigned int i = 0; i < m_nodes.size();i++){
		/*		cout <<m_nodes[i].m_moduleID << ":"  <<m_nodes[i].m_isFixed  << endl;*/
		if(!m_nodes[i].m_isFixed){
			m_nodes[i].m_Left = m_lower_bound;
			
		//m_nodes[i].m_Right = m_upper_bound;
			_nodeID_ED_count[i] = m_nodes[i].m_inedge.size();
		}
		else{
			_nodeID_ED_count[i] = 0;
		}
	}
	
	m_MaxLeft = -INT_MAX;
	
	// forward
	for (ite = m_nodes[m_source].m_outedge.begin() ; ite!= m_nodes[m_source].m_outedge.end() ; ite++ ){
			_queue.push(*ite); 
	}
	
	
	_nodeID_Set.insert(m_source);
	int _nodeID_i,_nodeID_j;
	double _weight;
	double _most_neg_slack =0.0;
	while (_queue.empty() != true){
		///FIXME: map are nologer avalible
		_nodeID_i = _queue.front().m_from_nodeID;
		_nodeID_j = _queue.front().m_to_nodeID;
		_weight = _queue.front().m_weight;
		
		//cout << "EDGE:" << m_nodes[_nodeID_i].m_moduleID << "->" << m_nodes[_nodeID_j].m_moduleID <<endl;

		//TODO: backtrace ability
		
		if (_nodeID_ED_count[_nodeID_i] > 0 ){ //no valid value
			_queue.push(_queue.front());
			_queue.pop();
		}else{
			_nodeID_ED_count[_nodeID_j]--;
			_queue.pop();
			if (m_nodes[_nodeID_j].m_isFixed ){
			}else{
			m_nodes[_nodeID_j].m_Left = max(m_nodes[_nodeID_j].m_Left , 
							m_nodes[_nodeID_i].m_Left + _weight  );
//  			cout << "\tNodeID:" << _nodeID_j<< "LEFT:" <<m_nodes[_nodeID_j].m_Left <<endl;
			m_MaxLeft = (m_nodes[_nodeID_j].m_Left > m_MaxLeft) ? m_nodes[_nodeID_j].m_Left : m_MaxLeft;
			}
			
			
	 		if (_nodeID_Set.find(_nodeID_j) == _nodeID_Set.end()){
					_nodeID_Set.insert(_nodeID_j);
				for (ite = m_nodes[_nodeID_j].m_outedge.begin() ;
					ite!= m_nodes[_nodeID_j].m_outedge.end() ; ite++ ){
					_queue.push(*ite); 
					}
				}
			}
		//cout << "forward---SIZE of Queue:" << _queue.size() <<endl;
		
		
	}
	
	_nodeID_Set.clear();
	// backward
	assert(_queue.empty());
	for (unsigned int i = 0; i < m_nodes.size();i++){
		if(m_nodes[i].m_isFixed){
			_nodeID_ED_count[i] = 0;
		}else{
			_nodeID_ED_count[i] = m_nodes[i].m_outedge.size();
	// m_nodes[i].m_Right = max(m_nodes[m_sink].m_Left,m_upper_bound); //redifne it
			m_nodes[i].m_Right = m_upper_bound;
		}
	
	}
	
	
	//m_nodes[m_sink].m_Right = max(m_nodes[m_sink].m_Left,_bound_box);// problem!!!
	//m_nodes[m_sink].m_Right = max(m_nodes[m_sink].m_Left,m_upper_bound);
	for (ite = m_nodes[m_sink].m_inedge.begin() ;
		ite!= m_nodes[m_sink].m_inedge.end() ; ite++ ){
// 			if ( !m_nodes[ite->m_from_nodeID].m_isFixed )
				_queue.push(*ite); 
	}
				_nodeID_Set.insert(m_sink);
// 	cout << "backward\n";
	while (_queue.empty() != true){
		_nodeID_i = _queue.front().m_to_nodeID;
		_nodeID_j = _queue.front().m_from_nodeID;
		
		//TODO: backtrace ability
		if (_nodeID_ED_count[_nodeID_i] > 0 ){
			_queue.push(_queue.front());
			_queue.pop();
		}else{
			_queue.pop();
			_nodeID_ED_count[_nodeID_j]--;
			if (m_nodes[_nodeID_j].m_isFixed ){
			}else{
			m_nodes[_nodeID_j].m_Right = min(m_nodes[_nodeID_j].m_Right , 
							m_nodes[_nodeID_i].m_Right -
							((m_nodes[_nodeID_j].m_length + m_nodes[_nodeID_i].m_length  ) / 2));
			}
// 			cout << "\tJ ModuleID:" << m_nodes[_nodeID_j].m_moduleID << "\tI ModuleID:" << m_nodes[_nodeID_i].m_moduleID << 
// 		"\tJ-Right:" << m_nodes[_nodeID_j].m_Right << "\tI-Right:" 
// 	        		<< m_nodes[_nodeID_i].m_Right 
// 				<< "\t RESULT:" << m_nodes[_nodeID_j].m_Right
// 				<<endl;
						//
//			ADD MAX negtive slack detection
// 			_most_neg_slack = min(_most_neg_slack,m_nodes[_nodeID_j].m_Right-
// 					m_nodes[_nodeID_j].m_Left);
			
			if (_nodeID_Set.find(_nodeID_j) == _nodeID_Set.end()){
				_nodeID_Set.insert(_nodeID_j);
				for (ite = m_nodes[_nodeID_j].m_inedge.begin() ;
					ite!= m_nodes[_nodeID_j].m_inedge.end() ; ite++ ){
					_queue.push(*ite); 
				}
			}
		}
		//cout << "backward---SIZE of Queue:" << _queue.size() <<endl;
		
		
	}

	//neg_slack should always less than 0
	CSPCGraphEdge _most_neg_slack_edge(-1,-1,-1);
	double sum_neg_slack = 0.0;
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++){
		for (ite = m_nodes[i].m_outedge.begin() ; ite!= m_nodes[i].m_outedge.end() ; ite++ ){
			ite->m_slack = m_nodes[ite->m_to_nodeID].m_Right -
					m_nodes[ite->m_from_nodeID].m_Left-
					ite->m_weight;
			if (ite->m_slack < _most_neg_slack)
				_most_neg_slack_edge = *ite;
			_most_neg_slack = min(_most_neg_slack,ite->m_slack);
			if (ite->m_slack < 0)
				sum_neg_slack += ite->m_slack;
		}
	}
	
	if (gArg.GetDebugLevel() >= 4 ) printf("MaxLeft%f\n", m_MaxLeft);
	if (gArg.GetDebugLevel() >= 4 ) printf("MostNegSlack:%f\n", _most_neg_slack);
	return _most_neg_slack;
// 	return sum_neg_slack;
	
// 	_most_neg_slack_edge.ShowEdge();
//	cout << "_most_neg_slack:" <<_most_neg_slack << endl;
// 				       for (unsigned int i = 0 ; i < m_nodes.size() ; i++){
// 		cout << "ModuleID:" << m_nodes[i].m_moduleID << "\tLeft:" << m_nodes[i].m_Left << "\tRight:" 
// 		<< m_nodes[i].m_Right <<endl;
// 				       }
				       
				       
#if 	0			       
				       if (_most_neg_slack > 0) 
					       _most_neg_slack = 0.0;
				       m_most_neg_slack = _most_neg_slack;
	//Slack Calculation
				       m_CriticalEdge.clear();
				       bool _bSetFound=false;
				       bool _anymovable;
				       set<int> _connected_set;
				       for (unsigned int j = 0 ; j < m_nodes.size() ; j++){
					       int i = (j+m_nodes.size()-1) % m_nodes.size();
					       for (ite = m_nodes[i].m_outedge.begin() ;
						    ite!= m_nodes[i].m_outedge.end() ; ite++ ){
							ite->m_slack = m_nodes[ite->m_to_nodeID].m_Right -
							m_nodes[ite->m_from_nodeID].m_Left-
							ite->m_weight;
							ite->m_isCritical = false;
			//assert(ite->m_slack >= 0);
		
							_anymovable = ( (!(m_nodes[i].m_isFixed)) || (!(m_nodes[ite->m_to_nodeID].m_isFixed)));
							if ( ite->m_slack -  _most_neg_slack  < 1E-4 && _anymovable  &&  ite->m_slack < 0)
			//if ( ite->m_slack < 0 && _anymovable )
							{
			       
  			//		ite->ShowEdge();
							if(!_bSetFound){	
//  					cout << "1st:" ;
//  					ite->ShowEdge();
// 					cout << "\tRight:" << m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right 
// 					<< "\tLeft:" << m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left <<endl;
										ite->m_isCritical = true;
										m_CriticalEdge.push_back(*ite);
					
										_bSetFound=true;
					//_connected_set.insert(ite->m_from_moduleID);
										_connected_set.insert(ite->m_to_moduleID);
										}else{
										if (_connected_set.find(ite->m_from_moduleID)!= _connected_set.end() 
					 // || _connected_set.find(ite->m_to_moduleID)!= _connected_set.end()			       
										){
  //						cout << "-" ;
 // 						ite->ShowEdge();
// 						cout << "\tRight:" << m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right 
// 						<< "\tLeft:" << m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left     <<endl;						
										ite->m_isCritical = true;
										m_CriticalEdge.push_back(*ite);
						
										_connected_set.insert(ite->m_from_moduleID);
										_connected_set.insert(ite->m_to_moduleID);
										}
					
										}
				
										}
// 			cout << "EDGE:" <<ite->m_from_moduleID << "->" << ite->m_to_moduleID <<  "\t SLACK:" << ite->m_slack 
// 			<< "\t WEIGHT:" << ite->m_weight 
// 			<< endl;
// 			cout <<  m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left << "<->" <<   m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right <<endl;
										}
										for (ite = m_nodes[i].m_inedge.begin() ;
										ite!= m_nodes[i].m_inedge.end() ; ite++ ){
										ite->m_slack = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right -
										m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left-
										ite->m_weight;
										ite->m_isCritical = false;
										if ( ite->m_slack -  _most_neg_slack  < 1E-4 )
										ite->m_isCritical = true;
						
										}
				       }
	



	
// 	cout << "Longest Path:" << m_nodes[m_sink].m_Left - m_nodes[m_source].m_Left << "\t BOUND:" << _bound_box <<endl;
				       if (m_nodes[m_sink].m_Left - m_nodes[m_source].m_Left > _bound_box){
					       OverLoad = (m_nodes[m_sink].m_Left - m_nodes[m_source].m_Left - _bound_box) 
		/_bound_box;
		
				       }else{
					       OverLoad = 0;
				       }
	//return (m_nodes[m_sink].m_Left - m_nodes[m_source].m_Left < _bound_box);
				       return m_CriticalEdge.empty();
	
#endif
	
	
	
}








void CSPCGraph::AddEdge( CSPCGraphNode & s, CSPCGraphNode & t )
{
	double _edgeweight = (s.m_length + t.m_length  ) / 2;
	s.m_outedge.push_back(CSPCGraphEdge(s.m_nodeID,t.m_nodeID,_edgeweight));
	t.m_inedge.push_back(CSPCGraphEdge(s.m_nodeID,t.m_nodeID,_edgeweight));
	s.m_edge_nodeID.insert(t.m_nodeID);
	t.m_edge_nodeID.insert(s.m_nodeID);
}

CSPCGraphEdge::CSPCGraphEdge( int from_nodeID, int to_nodeID, double weight )
{
	m_from_nodeID = from_nodeID;
	m_to_nodeID = to_nodeID;
	m_weight = weight;
		
}




void CMacroSP::OutputGraph( const char* filename){

	FILE* out = fopen( filename, "w" );
	if( !out )
	{
		cerr << "Error, cannot open output file: " << filename << endl;
		return;
	}
//     printf( "Output Constraint figure: %s\n", filename );

    // output title

	int _H_n_edges = 0;
	for( unsigned int i = 0 ; i < HCG.m_nodes.size() ; i++){
		_H_n_edges+= HCG.m_nodes[i].m_outedge.size();
	}
	int _V_n_edges = 0;
	for( unsigned int i = 0 ; i < VCG.m_nodes.size() ; i++){
		_V_n_edges+= VCG.m_nodes[i].m_outedge.size();
	}
	


	fprintf( out, "\nset title \" %s, block= %d, HCG edges= %d, VCG edges= %d   \" font \"Times, 22\"\n\n",
		 filename, (int)HCG.m_nodes.size(), _H_n_edges,_V_n_edges );

	fprintf( out, "set size ratio 1\n" );
	fprintf( out, "set nokey\n\n" ); 
	
	//double sx = 0;
	//double sy = 0;
	//double cx = (HCG.GetUpperBound() + HCG.GetLowerBound())/2;
	//double cy = (VCG.GetUpperBound() + VCG.GetLowerBound())/2;
	double r =  max( HCG.GetUpperBound() - HCG.GetLowerBound(), VCG.GetUpperBound() - VCG.GetLowerBound()) /2 ;
	r *= 0.8;
	
				
	//output ModuleID
	for( int i=0; i<(int)HCG.m_nodes.size()-2; i++ )
	{
// 		if (!HCG.m_nodes[i].m_isFixed)
		fprintf( out, "set label '%d' at %12.3f,%12.3f center\n", 
				 HCG.m_nodes[i].m_nodeID, HCG.m_nodes[i].m_cx, VCG.m_nodes[i].m_cx);
		if (gArg.CheckExist("MPOutNetCenter")){
			fprintf( out, "set label '%d' at %12.3f,%12.3f center\n", i, m_PinCenter_x[i],m_PinCenter_y[i]);
// 			CalcCirIntersec( cx, cy, r, 
// 					 HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx,
// 					 m_PinCenter_x[i], m_PinCenter_y[i], sx,sy);
// 
// 			
// 			fprintf( out, "set label '#%d' at %12.3f,%12.3f center\n", i,sx,sy);
			
		}
		
		
		
		
	}

    //output edge weight
//     if (__n_edges < 100){
// 		for(unsigned int i=0; (i<m_nodes.size() && ((int)i)-1 != NodeID ); i++ )
// 			{
// 				for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_outedge.begin(); 
// 					ite !=m_nodes[i].m_outedge.end();++ite ){
// 					CCGraphNode& t_node2 = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]];
// 					CCGraphNode& f_node2 = m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]];
// 					fprintf( out, "set label '%12.1f' at %12.3f,%12.3f center\n", 
// 			ite->m_weight,(t_node2.m_cx+f_node2.m_cx)/2. ,(t_node2.m_cy+f_node2.m_cy)/2.);
// 					}
// 			}
// 	}
    


	fprintf( out, "plot[:][:] '-' w l lt 4, '-' w l lt 6, '-' w l lt 2, '-' w l lt 3 ,'-' w l lt 4\n\n" ); 
 

    // output Core region
	fprintf( out, "\n# core region\n" ); 
	fprintf( out, "%12.3f, %12.3f\n", HCG.m_lower_bound, VCG.m_lower_bound );
	fprintf( out, "%12.3f, %12.3f\n", HCG.m_lower_bound, VCG.m_upper_bound );
	fprintf( out, "%12.3f, %12.3f\n", HCG.m_upper_bound, VCG.m_upper_bound );
	fprintf( out, "%12.3f, %12.3f\n", HCG.m_upper_bound, VCG.m_lower_bound ); 
	fprintf( out, "%12.3f, %12.3f\n\n", HCG.m_lower_bound, VCG.m_lower_bound);
	fprintf( out, "\nEOF\n\n" );







	// output degenerated edges
	fprintf( out, "\n# degenerated edges\n" ); 
#if 0
	double _s_cx,_s_cy;
	double _point_ratio = 0.01;
	double _point_width = (m_upper_bound - m_lower_bound) * _point_ratio;
	for(unsigned int i=0; (i<m_nodes.size() && ((int)i)-1 != NodeID ); i++ )
	{
		fprintf( out, "\n# NODE:%d  X: %f Y:%f\n",m_nodes[i].m_moduleID, m_nodes[i].m_cx,m_nodes[i].m_cy ); 
		if(NodeID >= 0 ){
			i = NodeID;
			cout << "Singlg Node:"<<i<<endl;
		}
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_outedge.begin(); 
				   ite !=m_nodes[i].m_outedge.end();++ite ){
    		

					   CCGraphNode& t_node = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]];
					   _s_cx = t_node.m_cx;
					   _s_cy = t_node.m_cy;
		//if(ite->m_priority > m_pri_thres){
					   if(ite->m_slack < 0  ){
// 			fprintf( out, "# LEFT:%f RIGHT%f WIDTH:%f ---> LEFT:%f RIGHT%f WIDTH:%f \n", m_nodes[i].m_Left, m_nodes[i].m_Right,m_nodes[i].m_length, t_node.m_Left,t_node.m_Right,t_node.m_length);
						   fprintf( out, "%12.3f, %12.3f\n", m_nodes[i].m_cx, m_nodes[i].m_cy );
						   fprintf( out, "%12.3f, %12.3f\n\n",_s_cx ,_s_cy);
					   }
	    
	     
				   }

	}
#endif	
	
	fprintf( out, "%12.3f, %12.3f\n\n",HCG.m_lower_bound, VCG.m_lower_bound); //dummy point 
	fprintf( out, "\nEOF\n\n" );







	// output effective edges
	fprintf( out, "\n# effective edges\n" ); 
#if 0    
	double _vec_x,_vec_y;
	double _arrow_vec_x1, _arrow_vec_y1;
	double _arrow_vec_x2, _arrow_vec_y2;
	double _angle;

	for(unsigned int i=0; (i<m_nodes.size() && ((int)i)-1 != NodeID ); i++ )
	{
		fprintf( out, "\n# NODE:%d  X: %f Y:%f\n",m_nodes[i].m_moduleID, m_nodes[i].m_cx,m_nodes[i].m_cy ); 
		if(NodeID >= 0 ){
			i = NodeID;
			cout << "Singlg Node:"<<i<<endl;
		}
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_outedge.begin(); 
				   ite !=m_nodes[i].m_outedge.end();++ite ){
    		

					   CCGraphNode& t_node = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]];
					   _s_cx = t_node.m_cx;
					   _s_cy = t_node.m_cy;

		//if(ite->m_priority <= m_pri_thres){

		
					   if(ite->m_isCritical){
		//if(ite->m_slack < 0){
						   _vec_x = _s_cx - m_nodes[i].m_cx;
						   _vec_y = _s_cy - m_nodes[i].m_cy;
						   _angle =  atan(_vec_y/ _vec_x);
						   _arrow_vec_x1 = _point_width * (cos(_angle + PI * 5/6));
						   _arrow_vec_y1 = _point_width * (sin(_angle + PI * 5/6));
						   _arrow_vec_x2 = _point_width * (cos(_angle - PI * 5/6));
						   _arrow_vec_y2 = _point_width * (sin(_angle - PI * 5/6));
						   fprintf( out, "%12.3f, %12.3f\n", m_nodes[i].m_cx, m_nodes[i].m_cy );
						   fprintf( out, "%12.3f, %12.3f\n",_s_cx ,_s_cy);
						   fprintf( out, "%12.3f, %12.3f\n",_s_cx + _arrow_vec_x1,_s_cy + _arrow_vec_y1);
						   fprintf( out, "%12.3f, %12.3f\n",_s_cx ,_s_cy);
						   fprintf( out, "%12.3f, %12.3f\n\n",_s_cx + _arrow_vec_x2,_s_cy + _arrow_vec_y2);
					   }
	     
				   }

	}
	
#endif
	fprintf( out, "%12.3f, %12.3f\n\n",HCG.m_lower_bound, VCG.m_lower_bound); //dummy point 
	fprintf( out, "\nEOF\n\n" );


    // output macro position
	fprintf( out, "\n# macros:%d\n", HCG.m_nodes.size()); 
	double x, y, w, h;
	for( int i=0; i<(int)HCG.m_nodes.size()-2; i++ )
	{
// 	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
// 	    continue;
	    
		if( HCG.m_nodes[i].m_isFixed == true )
			continue;
		fprintf( out, "# macro %d =%d=\n", HCG.m_nodes[i].m_nodeID, HCG.m_nodes[i].m_isFixed);

		x = HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_length*0.5;
		y = VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_length*0.5;
		w = HCG.m_nodes[i].m_length;
		h = VCG.m_nodes[i].m_length;


		fprintf( out, "%12.3f, %12.3f\n", x, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( out, "%12.3f, %12.3f\n\n", x, y );

	    //fprintf( out, "%12.3f, %12.3f\n", x+w*0.75, y+h );
	    //fprintf( out, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );

	}
	fprintf( out, "0.0,0.0 \n\n \nEOF\n\n" );



    
	fprintf( out, "\n# fixed macros:%d\n", HCG.m_nodes.size()); 
	for( int i=0; i<(int)HCG.m_nodes.size()-2; i++ )
	{

		if( HCG.m_nodes[i].m_isFixed == false )
			continue;
	    
		x = HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_length*0.5;
		y = VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_length*0.5;
		w = HCG.m_nodes[i].m_length;
		h = VCG.m_nodes[i].m_length;


		fprintf( out, "%12.3f, %12.3f\n", x, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y );
		fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( out, "%12.3f, %12.3f\n\n", x, y );

	    //fprintf( out, "%12.3f, %12.3f\n", x+w*0.75, y+h );
	    //fprintf( out, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );

	}
	fprintf( out, "0.0,0.0 \n\n \nEOF\n\n" );

    
    
    
    
    
    
    

	fprintf( out, "pause -1 'Press any key'" );
	fclose( out );

}

void CMacroSP::Packing( )
{
	for(unsigned int  i = 0 ; i < HCG.m_nodes.size(); i++){
		HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Left;
		VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_Left;
	}
	
	
	
}




void CMacroSP::SA( )
{
	vector<int> legaled_moduleID;

	
	while (!m_action.empty())
		m_action.pop();
	//Warm UP
	int n_warmup_run;
	n_warmup_run  =  m_MacroList.size() * 3;
// 	n_warmup_run = 0;
	
	m_infcost_avg = 0.0;
	m_discost_avg = 0.0;
	m_rndcost_avg = 0.0;
	m_wirecost_avg = 0.0;
	

	gArg.GetInt("MPSAwarmup", &n_warmup_run);
 	for (int i = 0 ; i< n_warmup_run ; i++ ){
 		Perturb( );
		CGConstruct();
		m_infcost_avg +=  -(HCG.CGAnalyze() + VCG.CGAnalyze());
		MacroLP(true);
		SolutionPacking();
		m_discost_avg +=  CalcDisplacement_CX() ;
		CalcMacroGrid();
		if ( !gArg.CheckExist("MPStdCost") )
			m_rndcost_avg += CalcRoundCost() ;
		else
			m_rndcost_avg += CalcStdCost() ;
		m_wirecost_avg	+= CalcWireCost();
		Restore();		 
 	}
	m_infcost_avg /= n_warmup_run;
	m_infcost_avg = max(m_infcost_avg, m_maxHPWL / 100 );
	m_discost_avg /= n_warmup_run;
	m_rndcost_avg /= n_warmup_run;
	m_wirecost_avg /= n_warmup_run;
	if (gArg.GetDebugLevel() >= 2 ) 
		printf(" AVG COST: A: %f B: %f C: %f D:%f \n", m_infcost_avg, m_discost_avg, m_rndcost_avg, m_wirecost_avg);
	
	if (gArg.GetDebugLevel() >= 2 ) cout << "Calculating Uphill cost...." << endl;

	m_state = 5;	
	double _cost;
	double init_cost = CalcCost();
	if (gArg.GetDebugLevel() >=  3) {
		cout << "Init Cost:" << init_cost << "  INF:" <<  m_infcost <<  "  DIS:" <<  m_discost << "  RND:" <<  m_rndcost << "  WIRE:" << m_wirecost <<endl;
						
						
	}
	
	//calculate Uphill cost
	double _uphill_cost = 0.0;
	int _uphill_count = 0;
	n_warmup_run  =  m_MacroList.size() * 3;
	gArg.GetInt("MPSAwarmup", &n_warmup_run);
	for (int i = 0 ; i< n_warmup_run ; i++ ){
		Perturb( );
		m_state = 5;
		_cost = CalcCost();
		if (gArg.GetDebugLevel() >= 4) cout << "Cost" << _cost << endl;
		_uphill_count++;
		_uphill_cost += fabs(_cost - init_cost);	
		Restore();		
	}
	_uphill_cost /= (double)_uphill_count;
	if (gArg.GetDebugLevel() >= 1 ) cout << "Avg_uphill_cost:" << _uphill_cost  << endl;
	// Caculate Init temp
	
	
	
	
	
	

	
	
	
	
	double P =0.05;
	double temp;
	gArg.GetDouble("MPSAinitrate", &P);
	temp = -(_uphill_cost/log(P));
	temp /= 1000; 
	if (gArg.GetDebugLevel() >= 2) cout << "Initial Temp:" << temp << endl;
	

	
	
	 
	if (gArg.GetDebugLevel() >= 2 ) CornerPacking();
	if (gArg.GetDebugLevel() >= 2 ) OutputGraph("SAInitCP.plt");
	
	
	
	
	//
	double best_cost = init_cost ;
	m_Best_SQ_p = m_SQ_p;
	m_Best_SQ_n = m_SQ_n;
	m_Best_SQ_orient = m_SQ_orient;
	double max_reject_rate = 0.95;
	gArg.GetDouble("MPSAmaxreject", &max_reject_rate);
	double min_temp = 1E-10;
	gArg.GetDouble("MPSAmintemp", &min_temp);
	double reject_rate = 0.0;
	double reject_count = 0;
	double temp_ratio = 0.85;
	gArg.GetDouble("MPSAtempratio", &temp_ratio);
	int 	per_temp_cycle  =  m_MacroList.size() * 20;
	gArg.GetInt("MPSAtempcycle", &per_temp_cycle);
	double prev_cost  = best_cost;
	
	double uphill_probe;
	double uphill_threshold;
	
	m_state = m_max_state;
	
	while ( temp > min_temp && reject_rate < max_reject_rate ){	// outter iteration 
		reject_count = 0;
		cout.flush();
		for ( int i = 0 ; i < per_temp_cycle ; i++){		//inner iteration
			Perturb( );
			_cost = CalcCost();
			if (gArg.GetDebugLevel() >=  3) {
				cout << "State: " << m_state << "\tCur Cost:" << _cost << "\tLP:" << m_LP_cost << "\tCP:" << m_CP_cost << endl;
				cout <<  "\t\tINF:" <<  m_infcost <<  "\tDIS:" <<  m_discost << "\tRND:" <<  m_rndcost <<endl;
						
						
			}
//  			cout << "Cost" << _cost << endl;
			uphill_probe = ((double)random()/(double)RAND_MAX);
			uphill_threshold = exp(-(_cost -  prev_cost)/temp);
			if (gArg.GetDebugLevel() >=  3)printf("Temp: %f Cost: %f -> %f  %f -- %7.2f \t", temp,prev_cost,_cost,uphill_probe,uphill_threshold);
			if ( uphill_probe < uphill_threshold && uphill_threshold != 1.0 ){
				Accept();
 				if (gArg.GetDebugLevel() >=  3) printf( "Accpeted!\n");
				if (_cost < best_cost ){ 
					best_cost = _cost;
					m_Best_SQ_p = m_SQ_p;
					m_Best_SQ_n = m_SQ_n;
					m_Best_SQ_orient = m_SQ_orient;
					
					if (gArg.GetDebugLevel() >= 1 ) {
						cout << "ITE:" << ite << " Best Cost:" << best_cost << "  INF:" <<  m_infcost <<  "  DIS:" <<  m_discost << "  RND:" <<  m_rndcost << "  WIRE:" << m_wirecost <<endl;
						
						
					}
					else
						cout << "-";
					if (gArg.GetDebugLevel() >= 2 ) Packing();
					if (gArg.GetDebugLevel() >= 2 ) OutputGraph("BestSP.plt");
					if (gArg.GetDebugLevel() >= 2 ) CornerPacking();
					if (gArg.GetDebugLevel() >= 2 ) OutputGraph("BestCP.plt");
//  					MacroLP();
  					if (gArg.GetDebugLevel() >= 2 ) SolutionPacking();
  					if (gArg.GetDebugLevel() >= 2 ) OutputGraph("BestMP.plt");
					
// 					if (gArg.GetDebugLevel() >= 2 ) RightPacking();
// 					if (gArg.GetDebugLevel() >= 2 ) OutputGraph("BestSP_R.plt");

					if (gArg.GetDebugLevel() >= 3 ){
						printf("SQ_P:");
						for (unsigned i = 0 ; i < m_SQ_p.size(); i++ ){
							printf(" %d", m_SQ_p[i]); 
						}
						printf("\nSQ_N:");
						for (unsigned i = 0 ; i < m_SQ_n.size(); i++ ){
							printf(" %d", m_SQ_n[i]); 
						}
						printf("\n");
						
					}
					
				}
				
					
			
			prev_cost = _cost;
			}else{
			//	cout << endl;
 				Restore();
 				if (gArg.GetDebugLevel() >=  3) cout << "Rejected:" << _cost << endl;
				reject_count++;
			}
			ite++;
			

		}
		reject_rate = reject_count / per_temp_cycle;
		m_cur_reject_rate = reject_rate;
		if (gArg.GetDebugLevel() >= 1 )
			cout << "Current Temp:" << temp <<  "\t Reject Rate:" << reject_rate <<  endl;
		else
			cout << "|";

		temp *= temp_ratio;
		A *= 2;
	}

	
	return;

	
}

void CMacroSP::Perturb( )
{
	int _type;
	
	_type = random() % 2;
//   	_type = 2;
	if ( _type == 0 ){	//rotation
		int _nodeID = m_SQ_p[random() % m_SQ_p.size()];
		while ( HCG.m_nodes[_nodeID].m_isFixed)
			_nodeID = m_SQ_p[random() % m_SQ_p.size()];
		

		m_action.push(0);
		m_action.push(_nodeID);

		Rotate(_nodeID);
	
// 		Orientation( );

	
	
	}else{
		// move
		int _SQ;
		int _nodeID1;
		int _nodeID2;


		_SQ = random() % 2;
		_nodeID1 = m_SQ_p[random() % m_SQ_p.size()];
// 		if (_nodeID1 == 0 ){
// 			_nodeID2 = 1;
// 		}else if (_nodeID1 == m_SQ_p.size() -1 ){
// 			_nodeID2 = m_SQ_p.size() -2;
// 		}else{
// 			_nodeID2 = ((random() % 2) == 0 )? _nodeID1 -1 : _nodeID1 + 1;
// 		}
		
		int _try = 0;		
 		_nodeID2 = m_SQ_p[random() % m_SQ_p.size()];
		while ( _nodeID1 == _nodeID2){
			_try++;
			_nodeID2 = m_SQ_p[random() % m_SQ_p.size()];		
			if ( _try > 100 ){
				break;
				_nodeID2 = _nodeID1;
			}

		}
		
		m_action.push(1);
		m_action.push(_SQ);
		m_action.push(_nodeID1);
		m_action.push(_nodeID2);
		
		

		SPSwap(_SQ,_nodeID1,_nodeID2);
	}
	
	

}

double CMacroSP::CalcCost( )
{
	

	double cost = 0.0;
	CGConstruct();
// 	HCG.CGAnalyze();
// 	VCG.CGAnalyze();
//   	return -(HCG.CGAnalyze() + VCG.CGAnalyze());
//   	return (HCG.m_MaxLeft + VCG.m_MaxLeft);
// 	m_infcost = -(HCG.CGAnalyze() + VCG.CGAnalyze());

// 	cost =  m_MacroList.size() * maxHPWL * m_infcost / 10000 * (1 - m_cur_reject_rate);
//  	cost +=  CalcDisplacement();
	
// 	CornerPacking();
// 	ApplyPlaceDB_CX();
// 	CalcMacroGrid( m_binMacro , m_binSize ); 
// 	cost += CalcDeadSpaceCost();

		
	m_infcost = -(HCG.CGAnalyze() + VCG.CGAnalyze()) / m_infcost_avg;
	
	
/*			
// 	m_infcost = (2 , m_infcost);
	if (gArg.CheckExist("MPLPOnly") ){
		MacroLP(true);
		SolutionPacking();
	}else{
		if (m_infcost == 0.0 && !gArg.CheckExist("MPCP") ) {	
			MacroLP();
			SolutionPacking();
		}else{
			CornerPacking();
			
	// 		m_discost += CalcWireCost() /  m_MacroList.size() / maxHPWL /10 ;
			
		}
	}


*/
	CornerPacking();
	
	m_discost = CalcDisplacement_CX() /  m_discost_avg;
	CalcMacroGrid();
	if ( !gArg.CheckExist("MPStdCost") )
		m_rndcost = CalcRoundCost() / m_rndcost_avg;
	else
		m_rndcost = CalcStdCost() / m_rndcost_avg;
	m_wirecost= CalcWireCost() / m_wirecost_avg;
	
	m_infcost *= A; 
	m_discost *= B;
	m_rndcost *= C;
	m_wirecost *= D;

	cost =  m_infcost + m_discost + m_rndcost + m_wirecost;
	m_CP_cost = cost;


	MacroLP(true);
	SolutionPacking();
	m_discost = CalcDisplacement_CX() /  m_discost_avg;
	CalcMacroGrid();
	if ( !gArg.CheckExist("MPStdCost") )
		m_rndcost = CalcRoundCost() / m_rndcost_avg;
	else
		m_rndcost = CalcStdCost() / m_rndcost_avg;
	m_wirecost= CalcWireCost() / m_wirecost_avg;
			
	m_infcost *= A; 
	m_discost *= B;
	m_rndcost *= C;
	m_wirecost *= D;
	
	
	cost =  m_infcost + m_discost + m_rndcost + m_wirecost;
	m_LP_cost = cost;

	if (gArg.CheckExist("MPLPOnly") ){
                cost = m_LP_cost;
	}else if( gArg.CheckExist("MPCP")){
		cost = m_CP_cost;
	}else{
		if (m_infcost == 0.0){
			cost = GetSmoothedCost	( false);	
		}else{
			cost = GetSmoothedCost ( true);

		}
	}

// 	cout << "  INF:" <<  m_infcost <<  "  DIS:" <<  m_discost << "  RND:" <<  m_rndcost << "  WIRE:" << m_wirecost <<endl;
	
/*	
	if (m_infcost == 0.0){
// 		cost+=MacroLP();
//   		cost += CalcDeadSpaceCost();
// 		cost += CalcBoundCost();
// 		cost += CalcDisplacement();
	}else{
// 		cost += 1E7;
	}
//	cout << "Cost:" << cost << endl;*/

	return cost;


}

void CMacroSP::Rotate( int _nodeID )
{
	
	if ( m_SQ_orient[_nodeID] % 2 == 0){
		m_SQ_orient[_nodeID] = 1;
	}else{
		m_SQ_orient[_nodeID] = 0;
	}
	
	swap(HCG.m_nodes[_nodeID].m_length,VCG.m_nodes[_nodeID].m_length );
	
	
	
}

void CMacroSP::Restore( )
{
	int _nodeID1;
	int _nodeID2;
	int _SQ;
	if (m_action.front() == 0){
		m_action.pop();
		_nodeID1 = m_action.front();
		m_action.pop();
		assert(m_action.empty());
		Rotate(_nodeID1);
	}else{
		m_action.pop();
		_SQ = m_action.front();
		m_action.pop();
		_nodeID1 = m_action.front();
		m_action.pop();
		_nodeID2 = m_action.front();
		m_action.pop();
		assert(m_action.empty());
		SPSwap(_SQ,_nodeID1,_nodeID2);
	}
	
}


void CMacroSP::SPSwap( int _SQ, int _nodeID1, int _nodeID2 )
{
	vector<int>& SQ = ( _SQ == 0 ) ? m_SQ_p : m_SQ_n;
	
	swap(SQ[_nodeID1], SQ[_nodeID2]);
		
	
}

void CMacroSP::Accept( )
{
	while (!m_action.empty())
		m_action.pop();	
}

void CMacroSP::RightPacking( )
{
	for(unsigned int  i = 0 ; i < HCG.m_nodes.size() -2; i++){
			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Right;
			VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_Right;
			if (HCG.m_nodes[i].m_isFixed){
				printf("FIXED: %d H:%f V:%f\n", i,HCG.m_nodes[i].m_Right, VCG.m_nodes[i].m_Right);
			}
		}
}


bool/*double*/ CMacroSP::MacroLP( bool ignore ) // kaie return false if no solution
{
	BoundaryGravityRecover();
// 	SmartBoundaryGravity();
	lprec *lp;
	int Ncol;
	int *colno = NULL;
	int j;
	int ret = 0;
	char var_name[1000];
	REAL *row = NULL;
	
	Ncol = (HCG.m_nodes.size()-2) * 4 ; // exclude SOURCE & SINK
			//for each macro cx,cy,dx,dy
	//create lp_sovler
	lp = make_lp(0,Ncol);
	if (lp == NULL)
		ret = 1;
	//allocate memory
	int nodeID;
	const int _cx =1;
	const int _cy =2;
	const int _dx =3 ;
	const int _dy =4;
	if(ret == 0){
		for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
			nodeID = HCG.m_nodes[i].m_nodeID;
			sprintf(var_name, "%d_cx",nodeID);
			set_col_name(lp,i*4+_cx,var_name);
			sprintf(var_name, "%d_cy",nodeID);
			set_col_name(lp,i*4+_cy,var_name);
			sprintf(var_name, "%d_dx",nodeID);
			set_col_name(lp,i*4+_dx,var_name);
			sprintf(var_name, "%d_dy",nodeID);
			set_col_name(lp,i*4+_dy,var_name);
		}
		
		colno = (int *)malloc(Ncol * sizeof(*colno));
		row = (REAL *)malloc(Ncol * sizeof(*row));

		if ((colno == NULL) || (row == NULL))
			ret = 2;
		
	}

	list<CSPCGraphEdge>::iterator ite ;
	int _NodeID;

	//Add constraints
	if (ret == 0 ){
		set_add_rowmode(lp,TRUE);
		// x first!!
		//TODO:reduce bound constrain!

		for (unsigned int i = 0 ; (i < HCG.m_nodes.size()-2 && ret == 0 );i++){

			// i stands for NodeID
			//dummy dx constraint
			j = 0;
			colno[j] = i*4 +_cx;
			row[j++] = 1;
			colno[j] = i*4 +_dx;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,GE,HCG.m_nodes[i].m_rcx))
				ret = 3;
			j = 0;
			colno[j] = i*4 +_cx;
			row[j++] = 1;
			colno[j] = i*4 +_dx;
			row[j++] = -1;
			if(!add_constraintex(lp,j,row,colno,LE,HCG.m_nodes[i].m_rcx))
				ret = 3;
				
			//chip bound constraint	
			if(HCG.m_nodes[i].m_isFixed){
				j = 0;
				colno[j] = i*4 +_cx;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,EQ,
				    HCG.m_nodes[i].m_rcx + 0.5 *  HCG.m_nodes[i].m_length))
					ret = 3;	
			}else{

				j = 0;
				colno[j] = i*4 +_cx;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,GE,
				    HCG.GetLowerBound() + 0.5 * HCG.m_nodes[i].m_length + m_BoundSpace))
					ret = 3;	
				
				j = 0;
				colno[j] = i*4 +_cx;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,LE,
				    HCG.GetUpperBound()- 0.5 * HCG.m_nodes[i].m_length - m_BoundSpace))
					ret = 3;	
			}
			for ( ite = HCG.m_nodes[i].m_outedge.begin(); 	//edges
				  ite !=HCG.m_nodes[i].m_outedge.end() && ret ==0;++ite ){

					  if (ite->m_to_nodeID < (int)HCG.m_nodes.size() -2  ){ // not point so SINK
					_NodeID = ite->m_to_nodeID;
// 					cout << "\tH-EDGE:" << i << "->" << _NodeID <<endl;
					j = 0;
					colno[j] = _NodeID*4 +_cx;
					row[j++] = 1;
					colno[j] = i*4 +_cx;
					row[j++] = -1;
					if (ite->m_slack >= 0 || ignore == false ){
 						if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight) )
// 					  if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight + max (HCG.m_nodes[_NodeID].m_back_spacing, HCG.m_nodes[i].m_front_spacing) )) //NOTE!!!
							ret = 3;
					}
					
				}
				
			 }
			
		}
		
		// then y!!
		for (unsigned int i = 0 ; (i < VCG.m_nodes.size()-2 && ret == 0 );i++){
			
			
			_NodeID = i;
			// i stands for NodeID
			//dummy dy constraint
			j = 0; 
			colno[j] = _NodeID*4 +_cy;
			row[j++] = 1;
			colno[j] = _NodeID*4 +_dy;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,GE,VCG.m_nodes[i].m_rcx))
				ret = 3;
			j = 0;
			colno[j] = _NodeID*4 +_cy;
			row[j++] = 1;
			colno[j] = _NodeID*4 +_dy;
			row[j++] = -1;
			if(!add_constraintex(lp,j,row,colno,LE,VCG.m_nodes[i].m_rcx))
				ret = 3;
				
			//chip bound constraint	
			if (VCG.m_nodes[i].m_isFixed){
				j = 0;
				colno[j] = _NodeID*4 +_cy;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,EQ,VCG.m_nodes[i].m_rcx + 0.5 *  VCG.m_nodes[i].m_length))
					ret = 3;	

			}else{
				
				j = 0;
				colno[j] = _NodeID*4 +_cy;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,GE,VCG.GetLowerBound()
								+ 0.5 * VCG.m_nodes[i].m_length + m_BoundSpace))
					ret = 3;	
				
				j = 0;
				colno[j] = _NodeID*4 +_cy;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,LE,VCG.GetUpperBound()
								- 0.5 * VCG.m_nodes[i].m_length - m_BoundSpace))
					ret = 3;	
			}
				
			for ( ite = VCG.m_nodes[i].m_outedge.begin(); 	//edges
				 ite !=VCG.m_nodes[i].m_outedge.end() && ret ==0;++ite ){
				if (ite->m_to_nodeID < (int)VCG.m_nodes.size() -2 ){ // not point so SINK
					_NodeID = ite->m_to_nodeID;	//lp use HCG nodeID!!
// 					cout << "\tV-EDGE:" << i << "->" << _NodeID <<endl;
					j = 0;
					colno[j] = _NodeID*4 +_cy;
					row[j++] = 1;
					colno[j] = i*4 +_cy;
					row[j++] = -1;
					if (ite->m_slack >= 0 || ignore == false ){
						if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight) )
// 					if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight + max (VCG.m_nodes[_NodeID].m_back_spacing, VCG.m_nodes[i].m_front_spacing) ))
						ret = 3;
					}
				}
			}
			
		}
		
	}
	
	set_add_rowmode(lp,FALSE);
	if(ret == 0){
		j = 0;
		for (unsigned int i = 0 ; (i < HCG.m_nodes.size()-2 && ret == 0 );i++){
		
// 		colno[j] = i*4 + _dx;
// 		row[j++] = 1;
// 		colno[j] = i*4 + _dy;
// 		row[j++] = 1;
/// 		weighted objective function
			colno[j] = i*4 + _dx;
			row[j++] = HCG.m_nodes[i].m_length;
			colno[j] = i*4 + _dy;
			row[j++] = VCG.m_nodes[i].m_length;
		}
		if(!set_obj_fnex(lp,j,row,colno))
			ret = 4;
	}
	
	 
	
	if(ret == 0){
		set_minim(lp);
//  		write_LP(lp,stdout);
//  		print_lp(lp);
	}
	set_verbose(lp,IMPORTANT);
	
	
	ret = solve(lp);
	if (ret == OPTIMAL || ret == SUBOPTIMAL){ // kaie add suboptimal
//		cout << "OPTIMAL SOLUTION FOUND!!!" <<endl;
		if(ret == SUBOPTIMAL)
			cout << "SUBOPTIMAL SOLUTION FOUND!!" << endl;
		ret = 0;
	
	}
	else{
		cout << "NO SOLUTION FESIBAL!!" <<endl;
		cout << "LP-FAILS!" << endl;
		ret = 5;
	}
	double obj =get_objective(lp) ;
	if (ret == 0 ){
		//cout << "Objective value:" << get_objective(lp) << endl;
// 		cout << get_objective(lp) << endl;
		get_variables(lp,row);
		for ( j = 0 ; j < Ncol ; j++){
			//cout << get_col_name(lp,j+1) << ":" << row[j] <<endl;
		}
		for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
			HCG.m_nodes[i].m_solution_cx = row[i*4+_cx-1];
			HCG.m_nodes[i].m_solution_cy = row[i*4+_cy-1];
			HCG.m_nodes[i].m_solution_x = row[i*4+_cx-1] - 0.5 * HCG.m_nodes[i].m_length ;
			HCG.m_nodes[i].m_solution_y = row[i*4+_cy-1] - 0.5 * VCG.m_nodes[i].m_length ;
			VCG.m_nodes[i].m_solution_cy = row[i*4+_cx-1];
			VCG.m_nodes[i].m_solution_cx = row[i*4+_cy-1];
			VCG.m_nodes[i].m_solution_x = VCG.m_nodes[i].m_solution_cy - 0.5 * VCG.m_nodes[i].m_length ;
			VCG.m_nodes[i].m_solution_y = VCG.m_nodes[i].m_solution_cy - 0.5 * HCG.m_nodes[i].m_length;

			
		}
		
	}

	free(row);
	free(colno);
	delete_lp(lp);
	

	//obj = 0.0;
	if (ret == 0 )// kaie return false if no solution
		return true;//return obj;
	else 
		return false;//return -1;
	
}






void CMacroSP::SmartBoundaryGravity( )
{

	
	int d_count[4] ;
	d_count[0] = 0;
	d_count[1] = 0;
	d_count[2] = 0;
	d_count[3] = 0;
	
	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
		int _moduleID = m_NodeID2ModuleID[i];
		int _VNodeID = i;
		double _move[4]; // NW NE SE SW
		_move[0]=0;
		_move[0] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Left);
		_move[0] += fabs(VCG.m_nodes[_VNodeID].m_cx - VCG.m_nodes[_VNodeID].m_Right);
		_move[1]=0;
		_move[1] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Right);
		_move[1] += fabs(VCG.m_nodes[_VNodeID].m_cx - VCG.m_nodes[_VNodeID].m_Right);
		_move[2]=0;
		_move[2] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Right);
		_move[2] += fabs(VCG.m_nodes[_VNodeID].m_cx - VCG.m_nodes[_VNodeID].m_Left);
		_move[3]=0;
		_move[3] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Left);
		_move[3] += fabs(VCG.m_nodes[_VNodeID].m_cx - VCG.m_nodes[_VNodeID].m_Left);
		
		
		double _min = 1E22;
		int _min_index = -1;
		for (int j = 0 ; j <  4 ; j++ ){
			if (gArg.GetDebugLevel() >= 4 ) printf (" %5f",_move[j]);
			if (_move[j] <  _min ){
				_min_index = j;
				_min = _move[j];
			}
			
			
		}
		if (gArg.GetDebugLevel() >= 4 )  printf ("\n");
		
		if (_min_index == 0 ){			//NW
			
			HCG.m_nodes[i].m_cx = HCG.GetLowerBound();
			VCG.m_nodes[_VNodeID].m_cx = VCG.GetUpperBound();
			d_count[0]++;
			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[_VNodeID].m_cx);
		}else if (_min_index == 1){		//NE
			HCG.m_nodes[i].m_cx = HCG.GetUpperBound();
			VCG.m_nodes[_VNodeID].m_cx = VCG.GetUpperBound();
			d_count[1]++;
			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[_VNodeID].m_cx);
		}else if (_min_index == 2){
			HCG.m_nodes[i].m_cx = HCG.GetUpperBound();
			VCG.m_nodes[_VNodeID].m_cx = VCG.GetLowerBound();
			d_count[2]++;
			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[_VNodeID].m_cx);
		}else if (_min_index == 3){
			HCG.m_nodes[i].m_cx = HCG.GetLowerBound();
			VCG.m_nodes[_VNodeID].m_cx = VCG.GetLowerBound();
			d_count[3]++;
			if (gArg.GetDebugLevel() >= 4 ) printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[_VNodeID].m_cx);
		}
		
		
	}
	printf("D: %d %d %d %d \n" , d_count[0],d_count[1],d_count[2],d_count[3]);

	// 	
// 		if((min((HCG.m_nodes[i].m_cy - HCG.m_lower_sidebound), 
// 		    (HCG.m_upper_sidebound - HCG.m_nodes[i].m_cy)) >
// 				  min((HCG.m_nodes[i].m_cx - HCG.m_lower_bound), 
// 				       (HCG.m_upper_bound - HCG.m_nodes[i].m_cx)) ) ||
// 				  LineGravity == false
// 		  ){
// 			if (HCG.m_nodes[i].m_cx > HCG.GetCenter())
// 				HCG.m_nodes[i].m_cx = HCG.GetUpperBound();
// 			else
// 				HCG.m_nodes[i].m_cx = HCG.GetLowerBound();
// 			HGravity.insert(HCG.m_nodes[i].m_moduleID);
// // 			cout << "Adding H-gravity " << HCG.m_nodes[i].m_moduleID << endl;
// 		  }
// 	}
// 	for (unsigned int i = 0 ; i < VCG.m_nodes.size();i++){
// 		if(HGravity.find(VCG.m_nodes[i].m_moduleID) == HGravity.end() ||
// 				 LineGravity == false
// 		  ){
// // 			cout << "Adding V-gravity " << VCG.m_nodes[i].m_moduleID << endl;
// 			if (VCG.m_nodes[i].m_cx > VCG.GetCenter())
// 				VCG.m_nodes[i].m_cx = VCG.GetUpperBound();
// 			else
// 				VCG.m_nodes[i].m_cx = VCG.GetLowerBound();
// 		  }
// 	}

}


void CMacroSP::BoundaryGravity()
{
	for (unsigned int i = 0 ; i < HCG.m_nodes.size();i++){
		if (HCG.m_nodes[i].m_cx > HCG.GetCenter())
			HCG.m_nodes[i].m_cx = HCG.GetUpperBound();
		else
	 		HCG.m_nodes[i].m_cx = HCG.GetLowerBound();
	}
	for (unsigned int i = 0 ; i < VCG.m_nodes.size();i++){
		if (VCG.m_nodes[i].m_cx > VCG.GetCenter())
			VCG.m_nodes[i].m_cx = VCG.GetUpperBound();
		else
			VCG.m_nodes[i].m_cx = VCG.GetLowerBound();
	}

}

void CSPCGraph::setNodeSpacing( int nodeID, double front_spacing, double back_spacing )
{
	
	m_nodes[nodeID].m_front_spacing = front_spacing;
	m_nodes[nodeID].m_back_spacing = back_spacing;
}



void CMacroSP::setMacroSpacing( )
{
	for ( unsigned int i = 0 ; i < HCG.m_nodes.size() - 2; i++ ){
		int _moduleID = m_NodeID2ModuleID[i];
		int n_pin_bottom = 0;
		int n_pin_top = 0;
		int n_pin_left = 0;
		int n_pin_right = 0;
		
		double _bottom = 0;
		double _top = 0;
		double _left = 0;
		double _right = 0;
		
		
		
		Module& _module = m_pDB->m_modules[_moduleID];
		for (unsigned int j = 0 ; j < _module.m_pinsId.size() ; j++ ){
			int _pinID = _module.m_pinsId[j];
			Pin& _pin = m_pDB->m_pins[_pinID];
			if ( _pin.xOff > _pin.yOff ){	//       /.
				if ( -_pin.xOff < _pin.yOff ){ 	// \'
					// right
					n_pin_right++;
				}else{
					// bottom
					n_pin_bottom++;
				}
					
			}else{			//       '/
				if ( -_pin.xOff < _pin.yOff ){ 	// \'
					
					//top
					n_pin_top++;
					
				}else{
					//left
					n_pin_left++;
				}

			}
			
		}
		
		
		int congestion_threshold = 30;
		 gArg.GetInt("MLspacingth",&congestion_threshold);
		
		
		double _low_spacing;
		double _high_spacing;
		double _rowheight = m_pDB->m_sites[0].m_height;
		_low_spacing = 1 * _rowheight;
		gArg.GetDouble("MLlowspacing",&_low_spacing);	
		_high_spacing = 7 * _rowheight;			
		gArg.GetDouble("MLhighspacing",&_high_spacing); 
		
		
		
/*		_top =  ( n_pin_top > congestion_threshold) ? _high_spacing : _low_spacing;
		_bottom =  ( n_pin_bottom > congestion_threshold) ? _high_spacing : _low_spacing;
		_left =  ( n_pin_left > congestion_threshold) ? _high_spacing : _low_spacing;
		_right =  ( n_pin_right > congestion_threshold) ? _high_spacing : _low_spacing;*/
		
		_top =   _low_spacing;
		_bottom =  _low_spacing;
		_left =  _low_spacing;
		_right = _low_spacing;
		
		if (_module.m_orient % 4 == 0 )	// N
			_bottom =  _high_spacing;
		if (_module.m_orient % 4 == 1 )	// OR_W
			_right = _high_spacing;
		if (_module.m_orient % 4 == 2 )	// OR_S
			_top = _high_spacing;
		if (_module.m_orient % 4 == 3 )	// OR_E
			_left = _high_spacing;
		
		HCG.setNodeSpacing( i, _right, _left );
		VCG.setNodeSpacing( i, _top, _bottom );
		
// 		HCG.setNodeSpacing( _moduleID, _left, _right );
// 		VCG.setNodeSpacing( _moduleID, _bottom, _top );
// 		printf("%d ----- %f %f %f %f \n", _moduleID, _right,_left,_top,_bottom);
		
		
		
			
		
		
		
		
		
		
		
	}
	
}

void CMacroSP::BoundaryGravityRecover( )
{
	for (unsigned int i = 0 ; i < HCG.m_nodes.size();i++){

		HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_value + 0.5* HCG.m_nodes[i].m_length;
	}
	for (unsigned int i = 0 ; i < VCG.m_nodes.size();i++){
		VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_value + 0.5* VCG.m_nodes[i].m_length;
		
	}
}

void CMacroSP::ApplyBestSP( )
{
	m_SQ_p = m_Best_SQ_p;
	m_SQ_n = m_Best_SQ_n;
	for (unsigned int i = 0 ; i < m_SQ_p.size(); i++ ){
		if (m_Best_SQ_orient[i] != m_SQ_orient[i]){
			Rotate(i);
		}
	}
	CGConstruct();
	HCG.CGAnalyze();
	VCG.CGAnalyze();

	
}



void CMacroSP::SolutionPacking( )
{
	for(unsigned int  i = 0 ; i < HCG.m_nodes.size(); i++){
		HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_solution_cx;
		VCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_solution_cy;
	}
	
	
	
}

void CMacroSP::CalcMacroGrid( vector< vector< double> >& _binMacro ,int _binSize)
{
	//set up bin structure
	_binMacro.resize(_binSize);
	for(unsigned int i = 0 ; i < _binMacro.size() ; i++ ){
		_binMacro[i].resize(_binSize);
		for(unsigned int j = 0 ; j < _binMacro[i].size() ; j++ ){
			_binMacro[i][j] = 0.0;			
		}
		
	}
	
	int _bin_x_s;
	int _bin_y_s;
	int _bin_x_e;
	int _bin_y_e;
	
	double _binWidth = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / _binSize;
	double _binHeight = (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom ) / _binSize;
	double _binArea = _binWidth * _binHeight;
	

	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2; i++){
	//	Module& mod = m_pDB->m_modules[m_MacroList[i]];
	 
		_bin_x_s = GetBinIndexX( HCG.m_nodes[i].m_cx -  HCG.m_nodes[i].m_length /2 - m_pDB->m_coreRgn.left, _binWidth);
		_bin_y_s = GetBinIndexY( VCG.m_nodes[i].m_cx -  VCG.m_nodes[i].m_length /2 - m_pDB->m_coreRgn.bottom, _binHeight );
		_bin_x_e = GetBinIndexX( HCG.m_nodes[i].m_cx +  HCG.m_nodes[i].m_length /2 - m_pDB->m_coreRgn.left,_binWidth );
		_bin_y_e = GetBinIndexY( VCG.m_nodes[i].m_cx +  VCG.m_nodes[i].m_length /2 - m_pDB->m_coreRgn.bottom,_binHeight );
//   		printf ("Macro:%d %d-%d %d-%d\n",i,_bin_x_s,_bin_x_e,_bin_y_s,_bin_y_e);
		if(_bin_x_s < 0 ) _bin_x_s = 0;
		if(_bin_x_e >= _binSize ) _bin_x_e =  _binSize -1;
		if(_bin_y_s < 0 ) _bin_y_s = 0;
		if(_bin_y_e >= _binSize ) _bin_y_e =  _binSize -1;
		
		if ( _bin_x_s < 0 || _bin_x_e > _binSize || _bin_y_s < 0 || _bin_y_e > _binSize)
			exit(0);
		for ( int j = _bin_x_s ; j <= _bin_x_e ; j++ ){
			for ( int k = _bin_y_s ; k <= _bin_y_e ; k++ ){	
//  				printf("%d %d == %d-%d\n",_binMacro.size(),_binMacro[j].size(),j,k);
// 				printf("MOD: %f %f %f %f \n", mod.m_cx,mod.m_cy,mod.m_width,mod.m_height);
				_binMacro[j][k] += getOverlapArea( j * _binWidth + m_pDB->m_coreRgn.left,
						k * _binHeight + m_pDB->m_coreRgn.bottom ,
						(j + 1 )* _binWidth + m_pDB->m_coreRgn.left,
						(k +1 )* _binHeight + m_pDB->m_coreRgn.bottom ,
						HCG.m_nodes[i].m_cx -  HCG.m_nodes[i].m_length /2 ,
						VCG.m_nodes[i].m_cx -  VCG.m_nodes[i].m_length /2 ,
					        HCG.m_nodes[i].m_cx +  HCG.m_nodes[i].m_length /2 ,
						VCG.m_nodes[i].m_cx +  VCG.m_nodes[i].m_length /2
						) / _binArea;
				
			}
		}
		
	}
// 	MoreContract( _binMacro );
	
	
}

void CMacroSP::ApplyPlaceDB( )
{
// 	m_pDB->m_modules_bak = m_pDB->m_modules;
	for (unsigned int i = 0 ; i < HCG.m_nodes.size() -2 ; i++ ){
		m_pDB->MoveModuleCenter( m_NodeID2ModuleID[i] , HCG.m_nodes[i].m_solution_cx ,HCG.m_nodes[i].m_solution_cy);
	}
	
}

void CMacroSP::ApplyPlaceDB_CX( )
{
// 	m_pDB->m_modules_bak = m_pDB->m_modules;
	for (unsigned int i = 0 ; i < HCG.m_nodes.size() - 2 ; i++ ){
		m_pDB->MoveModuleCenter( m_NodeID2ModuleID[i] , HCG.m_nodes[i].m_cx ,VCG.m_nodes[i].m_cx);
	}
	
}


void CMacroSP::ApplyPlaceDB_CX( bool fixed )
{
// 	int _pre = 0;
	int old_orient;
	int new_orient;
	int try_orient;
	double Best_HPWL;
	double _HPWL;
	int Best_orient;
		
	vector<int>::iterator ite;
	for (unsigned int i = 0 ; i < HCG.m_nodes.size() - 2 ; i++ ){
// 		mptree_module& mptmod = mpt.m_modules[i];		//update placedb
		int modid =  m_NodeID2ModuleID[i];
		Best_HPWL = 1E37;
		Best_orient = -1;


		old_orient = m_pDB->GetModuleOrient(modid);
// 		if (m_SQ_orient[i] = 1)
// 			new_orient = (old_orient - (old_orient % 2) + ((old_orient +1)  % 2));
// 		else
// 			new_orient = old_orient;
		new_orient = m_SQ_orient[i];
		for( int j = 0 ; j < 4 ;j++){
			try_orient = (new_orient + j * 2) % 8;
			m_pDB->SetModuleOrientation(modid, try_orient);
			_HPWL = m_pDB->CalcHPWL();
// 			cout << "Trying:" << try_orient << " " << _HPWL << endl;
			if(  _HPWL < Best_HPWL ){
				Best_HPWL = _HPWL;
				Best_orient = try_orient;
				
			}
						
			m_pDB->SetModuleOrientation(modid, Best_orient);
			
			
//			cout << m_pDB->m_modules[mptmod.placedb_moduleID].GetName() << ": ### " <<
//					m_pDB->GetModuleOrient(mptmod.placedb_moduleID) << endl;
		}
		m_pDB->MoveModuleCenter(modid, HCG.m_nodes[i].m_cx ,VCG.m_nodes[i].m_cx);
		if(fixed)
			m_pDB->SetModuleFixed(modid);
		
// 		m_pPart->fixModuleList.push_back(mpt.m_modules[i].placedb_moduleID);	//update fix module list
		
// 		ite = find(m_pPart->moduleList.begin(),m_pPart->moduleList.end(), mpt.m_modules[i].placedb_moduleID);
// 		if (ite != m_pPart->moduleList.end()){
// 			cout << "To find:"<<*ite << endl;
// 			m_pPart->moduleList.erase(ite);
// 		}
		
	}

	
}
	

	

		
void CMacroSP::MoreContract( vector< vector< double> >& _binMacro )
{
	for (unsigned  int i = 0 ; i < _binMacro.size() ; i++){
		for(unsigned  int j = 0 ; j < _binMacro[i].size() ; j++){
			if (_binMacro[i][j] > 0 )
				_binMacro[i][j] = 1.0;
		}
		
	}
}



void CMacroSP::SpaceDetect( vector< vector< double> >& _binMacro )
{
	
	
	m_SpaceArea.clear();
// 	for (unsigned int i = 0; i < m_SpaceArea.size(); i++)
// 		m_SpaceArea[i] = 0.0;
			
	
	int _current_space = 0;
	m_SpaceArea.push_back(0.0);
	m_binDead.resize(_binMacro.size());
	for (unsigned int i = 0; i < _binMacro.size(); i++){
		m_binDead[i].resize(_binMacro.size());
		_current_space = 0;
		for (unsigned int j = 0; j < _binMacro[i].size(); j++){
			if ( _binMacro[i][j] == 0.0 ){ 		// is a std-cell region
				if (_current_space == 0 ){	// found a new region

					if (i != 0 ){// check if it's a connected region
						if(m_binDead[i-1][j] != 0 ){	// there's a connected resion
							
							_current_space = static_cast<int>(m_binDead[i-1][j]);
							IncSpace( _current_space );
							m_binDead[i][j] = _current_space;
						}else{			// it's a new region

							_current_space = AddSpace();
							m_binDead[i][j] = _current_space;
							
						}
					}else{		// it's the first row and its a new region
						_current_space = AddSpace();
						m_binDead[i][j] = _current_space;
					}
					
				}else{				// ongoing 
					if (i != 0 ){// check if it's a connected region
						if(m_binDead[i-1][j] != 0 ){	// there's a connected resion
							//change space!
							if (static_cast<int>(m_binDead[i-1][j]) != _current_space){
								TermSpace( static_cast<int>(m_binDead[i-1][j]), _current_space );
							}
							
							IncSpace(_current_space);
							m_binDead[i][j] = _current_space;						
							
							
						}else{				// no connectted region
							IncSpace(_current_space);
							m_binDead[i][j] = _current_space;
						}
						
						
					}else{		// it's the first row 
						IncSpace(_current_space);
						m_binDead[i][j] = _current_space;
						
					}
				}
			}
			else{
				if (_current_space == 0 ) {	// Nothing found
					m_binDead[i][j] = 0.0;
				}else{				// ending a space
					_current_space = 0;
					m_binDead[i][j] = 0.0;
				}
			}
		}
	}
	
	
// 	for (unsigned int i = 0 ; i < m_SpaceArea.size() ; i++)
// 		printf ( " Space %d: %f \n" , i ,m_SpaceArea[i]);
		    
		    
	
			
// 	OutputMacroGrid( "DeadSpace.dat" ,m_binDead);		
	
	
}

double CMacroSP::CalcDeadSpaceCost( )
{
	
	SpaceDetect(m_binMacro);
	double _max_area = 0.0;
	for (unsigned int i = 0 ; i < m_SpaceArea.size() ; i++)
		if ( m_SpaceArea[i] > _max_area ) _max_area =  m_SpaceArea[i];
	double core = (HCG.GetUpperBound() - HCG.GetLowerBound())* (VCG.GetUpperBound() - VCG.GetLowerBound());
	return  core - _max_area;
	
}


double CMacroSP::GetGrad( int x, int y )
{
	double _val = 0;
	_val += (x < 1)? 0  :fabs(m_binMacro[x-1][y] - m_binMacro[x][y]);
	_val += (x >= m_binSize -1 )?  0 :fabs(m_binMacro[x+1][y] - m_binMacro[x][y]);
	_val += (y < 1) ? 0  :fabs(m_binMacro[x][y - 1] - m_binMacro[x][y]);
	_val += (y >= m_binSize -1 )?  0 :fabs(m_binMacro[x][y+1] - m_binMacro[x][y]);
	_val /= 4;
	return _val;
}

double CMacroSP::GetGrad( int x, int y , vector< vector< double> >& _binMacro , int _binSize )
{
	double _val = 0;
	_val += (x < 1)? 0  :fabs(_binMacro[x-1][y] - _binMacro[x][y]);
	_val += (x >= _binSize -1 )?  0 :fabs(_binMacro[x+1][y] - _binMacro[x][y]);
	_val += (y < 1) ? 0  :fabs(_binMacro[x][y - 1] - _binMacro[x][y]);
	_val += (y >= _binSize -1 )?  0 :fabs(_binMacro[x][y+1] - _binMacro[x][y]);
	_val /= 4;
	return _val;
}


double CMacroSP::CalcBoundCost( )
{
 	
	double _cost = 0.0;
	m_binBnd.resize(m_binSize);
	for(unsigned int i = 0 ; i < m_binBnd.size() ; i++ ){
		m_binBnd[i].resize(m_binSize);
		for(unsigned int j = 0 ; j < m_binBnd[i].size() ; j++ ){
			m_binBnd[i][j] = GetGrad(i,j);
			m_binBnd[i][j] += m_binMacro2Xratio * GetGrad(2*i,2*j, m_binMacro2X, m_binSize * 2);
			m_binBnd[i][j] += m_binMacro2Xratio * GetGrad(2*i+1 ,2*j, m_binMacro2X, m_binSize * 2);
			m_binBnd[i][j] += m_binMacro2Xratio * GetGrad(2*i,2*j+1, m_binMacro2X, m_binSize * 2);
			m_binBnd[i][j] += m_binMacro2Xratio * GetGrad(2*i+1,2*j+1, m_binMacro2X, m_binSize * 2);
			
			_cost += m_binBnd[i][j];
		}
		
	}
// 	OutputBndGrid( "BndCost.dat");
	return _cost;
	
}

double CMacroSP::CalcDisplacement( )
{
	
// 	CornerPacking();
	double cost = 0.0;
// 	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
// 		cost += fabs((HCG.m_nodes[i].m_value + 0.5* HCG.m_nodes[i].m_length) - HCG.m_nodes[i].m_cx);
// 		cost += fabs((VCG.m_nodes[i].m_value + 0.5* VCG.m_nodes[i].m_length) - VCG.m_nodes[i].m_cx);
// 		
// 	}
// 	return cost;
	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
		double rx = HCG.m_nodes[i].m_rcx ;
		double ry = VCG.m_nodes[i].m_rcx ;

		double L = HCG.m_nodes[i].m_Left;
		double R = HCG.m_nodes[i].m_Right;
		double T = VCG.m_nodes[i].m_Right;
		double B = VCG.m_nodes[i].m_Left;
		if ( rx <= R && rx >= L){
		}else{
 			cost += min (fabs(rx - L), fabs(rx - R ) );
		}
		if ( ry <= T && ry >= B){
		}else{
 			cost += min (fabs(ry - T), fabs(ry - B ) );
		}
		
 	}
	return cost;
	
}

void CMacroSP::CornerPacking( )
{
	
		
	int d_count[4] ;
	d_count[0] = 0;
	d_count[1] = 0;
	d_count[2] = 0;
	d_count[3] = 0;
	
// 	 printf("CornerPacking!!!!!!!!!!!!!!!!!!!!!\n");
	//double _move[4]; // NW NE SE SW
	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
// 		int _moduleID = m_NodeID2ModuleID[i];
#if 0
		_move[0] = 0.0;
		_move[0] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Left);
		_move[0] += fabs(VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_Right);
		_move[1] = 0.0;
		_move[1] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Right);
		_move[1] += fabs(VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_Right);
		_move[2] = 0.0;
		_move[2] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Right);
		_move[2] += fabs(VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_Left);
		_move[3] = 0.0;
		_move[3] = fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[i].m_Left);
		_move[3] += fabs(VCG.m_nodes[i].m_cx - VCG.m_nodes[i].m_Left);
		
	
	
		double _min = 1E22;
		int _min_index = -1;
		for (int j = 0 ; j <  4 ; j++ ){
			if (gArg.GetDebugLevel() >= 4 ) printf (" %5f",_move[j]);
			if (_move[j] <  _min ){
				_min_index = j;
				_min = _move[j];
			}
				
				
		}
#endif	
		
		int _min_index = 0;
								
		if ( HCG.m_nodes[i].m_Left > 0.5 * HCG.m_MaxLeft )
			_min_index += 1;
		if ( VCG.m_nodes[i].m_Left < 0.5 * VCG.m_MaxLeft )
			_min_index += 2;
		
		if (_min_index == 0 ){			//NW
				
			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Left;
			VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_Right;
			d_count[0]++;
// 			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx);
		}else if (_min_index == 1){		//NE
			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Right;
			VCG.m_nodes[i].m_cx =  VCG.m_nodes[i].m_Right;
			d_count[1]++;
// 			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx);
		}else if (_min_index == 2){		//SW
			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Left;
			VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_Left;
			d_count[2]++;
// 			if (gArg.GetDebugLevel() >= 4 )  printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx);
		}else if (_min_index == 3){		//SE
			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_Right;
			VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_Left;
			d_count[3]++;
// 			if (gArg.GetDebugLevel() >= 4 ) printf (" Macro %d Center (%f,%f)\n", _moduleID,HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx);
		}
	 
	}
// 	printf("D: %d %d %d %d \n" , d_count[0],d_count[1],d_count[2],d_count[3]);

	
}

double CMacroSP::CalcRoundCost( )
{
// 	double cx = HCG.GetUpperBound() - HCG.GetLowerBound();
// 	cx /= 2;
// 	double cy =  + VCG.GetUpperBound() - VCG.GetLowerBound();
// 	cy /= 2;
	
	
	
	double cost = 0 ;
	double CircleRatio = 0.8;
	gArg.GetDouble("MPCircleRatio", &CircleRatio);
	for(unsigned int i = 0 ; i < m_binMacro.size() ; i++ ){
		for(unsigned int j = 0 ; j < m_binMacro[i].size() ; j++ ){
			if ( sqrt( pow( i - m_StdRgn_x_idx , 2.0) + pow( j -  m_StdRgn_y_idx , 2.0)) <  
						  (  m_binSize / 2 * CircleRatio) ){
				cost += m_binMacro[i][j];
			}
		}
	}
	
	cost /= (m_binSize  * m_binSize / 4 * CircleRatio) ;
	return cost;
}

double CMacroSP::CalcDisplacement_CX( )
{
	double cost = 0.0;
 	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
		cost += fabs(HCG.m_nodes[i].m_rcx - HCG.m_nodes[i].m_cx);
		cost += fabs(VCG.m_nodes[i].m_rcx - VCG.m_nodes[i].m_cx);
 		
 	}
 	return cost;
}

double CMacroSP::CalcDisplacement_SCX( )
{
	double cost = 0.0;
 	for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
		cost += fabs(HCG.m_nodes[i].m_solution_cx - HCG.m_nodes[i].m_rcx);
		cost += fabs(HCG.m_nodes[i].m_solution_cy - VCG.m_nodes[i].m_rcx);
 		
 	}
 	return cost;
}



void CMacroSP::RCXPacking( )
{
	for(unsigned int  i = 0 ; i < HCG.m_nodes.size(); i++){
		HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_rcx;
		VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_rcx;
	}

}

void CMacroSP::CalcPinCenter( )
{
	m_PinCenter_x.resize(m_MacroList.size());
	m_PinCenter_y.resize(m_MacroList.size());
	m_MacroNet.resize(m_MacroList.size());
	m_MacroNetWeight.resize(m_MacroList.size());
	set<int> con_macros;
	vector<int> AffMacros;
	vector<int> Aff;
	for(unsigned int i=0; i < m_MacroList.size(); i++ )
	{
		int modid =  m_MacroList[i];
		Module& mod = m_pDB->m_modules[modid];
		double x = 0;
		double y = 0;
		double count = 0;
		AffMacros.clear();
		Aff.clear();
		printf("== MOD: %d == \n",m_ModuleID2NodeID[modid] );
		for (unsigned int j = 0 ;  j < mod.m_netsId.size(); j++ ){
			int netid = mod.m_netsId[j];
			Net& net = m_pDB->m_nets[netid];
// 			printf("\t NET: %d - ", netid );
			con_macros.clear();
			for (unsigned int k = 0 ; k < net.size(); k++ ){
				Pin& pin = m_pDB->m_pins[net[k]];
				if (m_MacroSet.find(pin.moduleId) == m_MacroSet.end()){
// 				if (pin.moduleId != modid){
					x += pin.absX;
					y += pin.absY;
					count++; 
				}else{
					if (pin.moduleId != modid)
						con_macros.insert(m_ModuleID2NodeID[pin.moduleId]);
// 					printf(" %d ", m_ModuleID2NodeID[pin.moduleId] );
				}
			}
// 			printf("\n");
			set<int>::iterator ite;
			for (ite = con_macros.begin() ; ite != con_macros.end() ; ite++){
				int found = -1;
				for (unsigned int l = 0 ; l < AffMacros.size() ; l++){
					if (AffMacros[l] == *ite){
						found = l;
						break;	
					}
				}
				if (found >= 0 ){
					Aff[found]++;
				}else{
					AffMacros.push_back(*ite);
					Aff.push_back(1);
				}
					
				 
			}
			

			
			
		}
		for (unsigned int k = 0 ; k < AffMacros.size() ; k++){
			if(Aff[k] > 5)
				printf(" %d=%d \t",	AffMacros[k], Aff[k]);			
		}
		printf("\n");
		x /= count;
		y /= count;
		m_PinCenter_x[i] = x;
		m_PinCenter_y[i] = y;
		
		m_MacroNet[m_ModuleID2NodeID[modid]] = AffMacros;
		m_MacroNetWeight[m_ModuleID2NodeID[modid]] = Aff;
	}
	
}

void CMacroSP::InitSP( )
{
	vector< vector<int> > P_Graph;
	vector< vector<int> > N_Graph;

	/// Graph[a][b] = 1:  a->b
	P_Graph.resize(m_MacroList.size());
	N_Graph.resize(m_MacroList.size());
	for(unsigned int  i = 0 ; i < m_MacroList.size(); i++){
		N_Graph[i].resize(m_MacroList.size());
		P_Graph[i].resize(m_MacroList.size());
		for(unsigned int j = 0 ; j < m_MacroList.size(); j++ ){
			N_Graph[i][j] = 0;
			P_Graph[i][j] = 0;
		}
	}
	
	for(unsigned int  i = 0 ; i < m_MacroList.size(); i++){
		for(unsigned int j = i+1 ; j < m_MacroList.size(); j++ ){
				if ( fabs( HCG.m_nodes[i].m_rcx -  HCG.m_nodes[j].m_rcx ) >
					fabs( VCG.m_nodes[i].m_rcx -  VCG.m_nodes[j].m_rcx ))// H or V
				{			//H
					if (HCG.m_nodes[i].m_rcx >   HCG.m_nodes[j].m_rcx) {	// j -> i
						// ( ... n1 ... n2 ) (...n1.... n2 ) n1 LEFT n2
// 						printf("H %d -> %d \n", j ,i );
						P_Graph[j][i] = 1;
						N_Graph[j][i] = 1;
					}else{							// i-> j
// 						printf("H %d -> %d \n", i ,j );
						P_Graph[i][j] = 1;
						N_Graph[i][j] = 1;
					}
				}
				else
				{
					if (VCG.m_nodes[i].m_rcx >   VCG.m_nodes[j].m_rcx) {	// i
												// j
					// ( ... n1 ... n2 ) ( ...n2.... n1 ) n1 is below n2  ==> n1-> n2
						P_Graph[j][i] = 1;
						N_Graph[i][j] = 1;
					}else{							// j
												// i
						P_Graph[i][j] = 1;
						N_Graph[j][i] = 1;
					}

					
				}
				
		}
	}
	
	ConstractSP(P_Graph,m_SQ_p);
	ConstractSP(N_Graph,m_SQ_n);
	
	
	
	
	
	
}

void CMacroSP::ConstractSP( vector< vector < int > > & Graph, vector< int > & SQ )
{
	SQ.clear();
	set<int> inserted;
	inserted.clear();
	unsigned int i = 0;
	printf("SQ:");
	while(SQ.size() < m_MacroList.size() ){
		if (i >= m_MacroList.size() ){
			cout << "ERROR!" << endl;
			exit(0);	
		}
// 		printf("%d IND: %d\n", i, InDeg(Graph, i ));
		if (InDeg(Graph, i ) == 0 &&  inserted.find(i) == inserted.end()){
			SQ.push_back(i);
			RemoveNode(Graph,i);
			printf("% 2d ", i);
			inserted.insert(i);
			i = 0; 
		}else{
			i++;
		}
			
	}
	printf("\n");

	
}

int CMacroSP::InDeg( vector< vector < int > > & Graph, int index )
{
	int count = 0;
	for(unsigned int i = 0 ; i < Graph.size() ; i++){
		count += Graph[i][index];
		
	}
	return count;
}

void CMacroSP::RemoveNode( vector< vector < int > > & Graph, int index )
{
	for(unsigned int i = 0 ; i < Graph.size() ; i++){
		 Graph[index][i] = 0;
	}
	
}

void CMacroSP::CalcRCX( )
{
	double expand_rate =1.0;
	
	gArg.GetDouble("MPMLEXP", &expand_rate);
// 	double xmid = (HCG.GetUpperBound() + HCG.GetLowerBound())* 0.5 ;
// 	double ymid = (VCG.GetUpperBound() + VCG.GetLowerBound())* 0.5 ;
	double xmid = m_StdRgn_x;
	double ymid = m_StdRgn_y;
	double expand;
	for(unsigned int  i = 0 ; i < HCG.m_nodes.size()-2; i++){
		expand = (HCG.m_nodes[i].m_rcx - xmid) * expand_rate;
		HCG.m_nodes[i].m_rcx = xmid + expand;
		expand = (VCG.m_nodes[i].m_rcx - ymid) * expand_rate;
		VCG.m_nodes[i].m_rcx = ymid + expand;
	}
		
	
// 	double sx = 0;
// 	double sy = 0;
// 	double cx = (HCG.GetUpperBound() + HCG.GetLowerBound())/2;
// 	double cy = (VCG.GetUpperBound() + VCG.GetLowerBound())/2;
// 	double r =  max( HCG.GetUpperBound() - HCG.GetLowerBound(), VCG.GetUpperBound() - VCG.GetLowerBound()) /2 ;
// 	r *= 0.8;
// 	
// 	for(unsigned int  i = 0 ; i < HCG.m_nodes.size()-2; i++){
// 		CalcCirIntersec( cx, cy, r, 
//  					 HCG.m_nodes[i].m_cx,VCG.m_nodes[i].m_cx,
//  					 m_PinCenter_x[i], m_PinCenter_y[i], sx,sy);
// 		HCG.m_nodes[i].m_rcx = sx;
// 		VCG.m_nodes[i].m_rcx = sy;
// 	}
}





void CMacroSP::CalcStdGrid( )
{
	//set up bin structure
	m_binStd.resize(m_binSize);
	for(unsigned int i = 0 ; i < m_binStd.size() ; i++ ){
		m_binStd[i].resize(m_binSize);
		for(unsigned int j = 0 ; j < m_binStd[i].size() ; j++ ){
			m_binStd[i][j] = 0.0;			
		}
		
	}
	
	//int bin_x, bin_y;
	
	double _binWidth = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_binSize;
	double _binHeight = (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom ) / m_binSize;
	double _binArea = _binWidth * _binHeight;
	
	int _bin_x_s, _bin_y_s, _bin_x_e,_bin_y_e;
	

	for (unsigned int i = 0 ; i < m_pDB->m_modules.size(); i++){
	
		Module& mod = m_pDB->m_modules[i];
	 	
// 		if (m_MacroSet.find(i) == m_MacroSet.end() && !(mod.m_isOutCore) ){
// 			bin_x = GetBinIndexX( mod.m_cx , _binWidth);
// 			bin_y = GetBinIndexY( mod.m_cy , _binHeight );
// 			if(bin_x < 0 ) bin_x = 0;
//  			if(bin_x >= m_binSize ) bin_x =  m_binSize -1;
// 	 		if(bin_y < 0 ) bin_y = 0;
//  			if(bin_y >= m_binSize ) bin_y =  m_binSize -1;
//  	 		if ( bin_x < 0 || bin_x > m_binSize || bin_y < 0 || bin_y > m_binSize)
// 				exit(0);
// 			
// 			m_binStd[bin_x][bin_y] += mod.m_area;
// 		}
		//if (m_MacroSet.find(i) == m_MacroSet.end() && !(mod.m_isOutCore) ){
		if (m_MacroSet.find(i) == m_MacroSet.end() && !( m_pDB->BlockOutCore( i )) ){
			_bin_x_s = GetBinIndexX( mod.m_cx - mod.m_width /2 - m_pDB->m_coreRgn.left, _binWidth);
			_bin_y_s = GetBinIndexY( mod.m_cy - mod.m_height /2 - m_pDB->m_coreRgn.bottom, _binHeight );
			_bin_x_e = GetBinIndexX( mod.m_cx + mod.m_width /2 - m_pDB->m_coreRgn.left,_binWidth );
			_bin_y_e = GetBinIndexY( mod.m_cy + mod.m_height /2 - m_pDB->m_coreRgn.bottom,_binHeight );
			if(_bin_x_s < 0 ) _bin_x_s = 0;
			if(_bin_x_e >= m_binSize ) _bin_x_e =  m_binSize -1;
			if(_bin_y_s < 0 ) _bin_y_s = 0;
			if(_bin_y_e >= m_binSize ) _bin_y_e =  m_binSize -1;
		
			if ( _bin_x_s < 0 || _bin_x_e > m_binSize || _bin_y_s < 0 || _bin_y_e > m_binSize)
				exit(0);
			for ( int j = _bin_x_s ; j <= _bin_x_e ; j++ ){
				for ( int k = _bin_y_s ; k <= _bin_y_e ; k++ ){	
//  					printf("%d %d == %d-%d\n",_binMacro.size(),_binMacro[j].size(),j,k);
// 					printf("MOD: %f %f %f %f \n", mod.m_cx,mod.m_cy,mod.m_width,mod.m_height);
					m_binStd[j][k] += getOverlapArea( j * _binWidth + m_pDB->m_coreRgn.left,
							k * _binHeight + m_pDB->m_coreRgn.bottom ,
							(j + 1 )* _binWidth + m_pDB->m_coreRgn.left,
							(k +1 )* _binHeight + m_pDB->m_coreRgn.bottom ,
							mod.m_cx - mod.m_width /2 ,
							mod.m_cy - mod.m_height /2,
							mod.m_cx + mod.m_width /2 ,
							mod.m_cy + mod.m_height /2 );
					
				}
			}
		
		
		}

	
	}
	
	m_StdRgn_x = 0.0;
	m_StdRgn_y = 0.0;
	int count = 0;
	
	for(unsigned int i = 0 ; i < m_binStd.size() ; i++ ){
		for(unsigned int j = 0 ; j < m_binStd[i].size() ; j++ ){
// 			printf(" %d %d %f \n", i, j,m_binStd[i][j]);
			m_binStd[i][j] /= _binArea;
			if ( m_binStd[i][j] > 0.5){
			m_StdRgn_x += (_binWidth * ( (double)i  + 0.5 ) + m_pDB->m_coreRgn.left) ;
			m_StdRgn_y += (_binHeight * ( (double)j  + 0.5 ) + m_pDB->m_coreRgn.bottom)  ;
			count++;
			}
		}
	}
	 
	m_StdRgn_x /= count;
	m_StdRgn_y /= count;
	m_StdRgn_x_idx = GetBinIndexX( m_StdRgn_x - m_pDB->m_coreRgn.left, _binWidth);
	m_StdRgn_y_idx = GetBinIndexY( m_StdRgn_y - m_pDB->m_coreRgn.bottom, _binHeight );
	
	
	printf(" Std Cell Region Center: %f %f %d %d \n", m_StdRgn_x, m_StdRgn_y, m_StdRgn_x_idx,m_StdRgn_y_idx  );
	

	
}





void CMacroSP::OutputMacroGrid( string filename, vector< vector< double> >& _binMacro  )
{
	FILE* out = fopen( filename.c_str(), "w" );
	for( unsigned int j=0; j < _binMacro.size(); j++)
	{
		for( unsigned int i=0; i< _binMacro[j].size() ; i++ )
			fprintf( out, "%.03f ", _binMacro[i][j]) ;
		fprintf( out, "\n" );
	}
	fprintf( out, "\n" );
	fclose( out );
}

double CMacroSP::CalcStdCost( )
{
	// 	double cx = HCG.GetUpperBound() - HCG.GetLowerBound();
// 	cx /= 2;
// 	double cy =  + VCG.GetUpperBound() - VCG.GetLowerBound();
// 	cy /= 2;
	double cost = 0;
	for(unsigned int i = 0 ; i < m_binStd.size() ; i++ ){
		for(unsigned int j = 0 ; j < m_binStd[i].size() ; j++ ){
			cost += m_binStd[i][j] *  m_binMacro[i][j];
		}
	}
	
	cost /= m_binMacro.size() * m_binMacro.size() /2;
			
	return cost ;
}

void CMacroSP::CalcCirIntersec( double cx, double cy, double r, double x1, double y1, double x2, double y2, double & x, double & y )
{
	
// 	printf(" %f %f %f       %f %f     %f %f \n",cx, cy, r, x1,y1,x2,y2);
		
	
	
	double A1 = y1-y2;
	double A2 = x2-x1;
	double B1 = x1 * y1 - x1 * y2 - x1 * y1 + x2 * y1;
	double K1 = B1 / A1 - cx ;
	double K2 = -(A2/A1);
	double A = 1 + K2*K2;
	double B = 2 * K1 *K2 - 2 * cy ;
	double C = K1 * K1 + cy * cy - r * r;
	double D = B *B - 4 * A * C;
	
	if (D < 0 ) {
		cout << "Neg Root!" << endl;
		exit(0);
	}
	
	double Y1 = (- B + sqrt(D) )  / 2 / A;;
	double Y2 = (- B - sqrt(D) )  / 2 / A;;
	
	double X1 =  ( B1 - A2 * Y1 ) / A1;
	double X2 =  ( B1 - A2 * Y2 ) / A1;
	
	if ( Dist(X1,Y1,x1,y1) + Dist(X1,Y1,x2,y2) <  Dist(X2,Y2,x1,y1) + Dist(X2,Y2,x2,y2) ){
		x = X1;
		y = Y1;
	}else{
		x = X2;
		y = Y2;	
	}
		
	
	
}

double CMacroSP::CalcWireCost( )
{
	double cost = 0;
	for (unsigned int i = 0 ; i < m_MacroNet.size(); i++){
		for (unsigned int j = 0 ; j < m_MacroNet[i].size(); j++){
			cost +=  fabs(HCG.m_nodes[i].m_cx - HCG.m_nodes[m_MacroNet[i][j]].m_cx) * m_MacroNetWeight[i][j];
			cost +=  fabs(VCG.m_nodes[i].m_cx - VCG.m_nodes[m_MacroNet[i][j]].m_cx) * m_MacroNetWeight[i][j];

		}
	}
	
	return cost;
}



double CMacroSP::GetSmoothedCost( bool direction ){

//	printf(" m_state:%d\t%d\n", m_state, direction ); 
	double cost;
	if ( m_state > 0 ) {
		cost = ( m_LP_cost * (m_max_state - m_state) +  m_CP_cost * m_state  ) / m_max_state;
	} else if ( m_state < 0 ) {
		cost = ( m_CP_cost * (m_max_state + m_state) -  m_LP_cost * m_state  ) / m_max_state;
	}else{
		cost = ( m_LP_cost + m_CP_cost ) / 2.0;
	}
	
	if (direction)
		m_state++;
	else
		m_state--;
	
	if ( m_state > m_max_state )
		m_state = m_max_state;
	if ( m_state < -(m_max_state) )
		m_state = -(m_max_state);

	return cost;
	
	
}



