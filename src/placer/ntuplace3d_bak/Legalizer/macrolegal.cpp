#include "macrolegal.h"
#include "libmetis.h"
#include "TetrisLegal.h" 
#include <algorithm>
#include <list>
#include <string>
#include <stdio.h>
#include <queue>
#include "lp_lib.h"
#include <cassert>
#include <set>
#include <cmath>






bool global_lesser(const CCGraphNode & s1,const CCGraphNode & s2 )
{
// 	cout << "s1.m_moduleID:" << s1.m_moduleID <<endl;
// 	cout << "s2.m_moduleID:" << s2.m_moduleID <<endl;
		return (s1.m_value  + 0.5 * s1.m_length ) < (s2.m_value  + 0.5 * s2.m_length );
               // return s1.m_cx < s2.m_cx;

}

CMacroLegal::~CMacroLegal()
{
}

CMacroLegal::CMacroLegal( CPlaceDB& db,double thresh_row_high , int _adj_run)
{
	m_pDB = &db;
	m_MacroRowHeight = thresh_row_high;
	m_AdjRun = _adj_run;
//	m_DML = new CLegalizer(db);
	m_offset_X = m_pDB->m_coreRgn.left;
	m_offset_Y = m_pDB->m_coreRgn.bottom;
	isOffset = false;
}


void CMacroLegal::Init( )
{
	OffsetRelocate();
//	double _Rgn_width =m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left;
//	double _Rgn_height =m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom;
	
// 	HCG.AddNodes( m_pDB->m_coreRgn.left, HCG.SOURCE ,m_pDB->m_coreRgn.bottom ,m_pDB->m_coreRgn.top, 0.0);
// 	HCG.AddNodes( m_pDB->m_coreRgn.right, HCG.SINK,m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.top ,0.0);
// 	VCG.AddNodes( m_pDB->m_coreRgn.top, VCG.SINK , m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.right,0.0);
// 	VCG.AddNodes( m_pDB->m_coreRgn.bottom, VCG.SOURCE,m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.right,0.0);
	HCG.Init();
	VCG.Init();
	m_ModuleID.clear();
	m_ModuleID_set.clear();
	HCG.SetBound( m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.right);
	HCG.SetSideBound(m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.top);
	VCG.SetBound( m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.top);
	VCG.SetSideBound( m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.right);
	
	double sum_area = 0.0;
	int n_movable =0;
	for(unsigned int i = 0 ; i < m_pDB->m_modules.size() ; i++){
		//if(m_pDB->m_modules[i].m_isOutCore == false)
		if(m_pDB->BlockOutCore( i ) == false)
			m_pDB->m_modules[i].m_isFixed = false;
		//if ((!m_pDB->m_modules[i].m_isFixed) && (!m_pDB->m_modules[i].m_isOutCore))
		if ((!m_pDB->m_modules[i].m_isFixed) && (!m_pDB->BlockOutCore(i) ))
		{
			sum_area += m_pDB->m_modules[i].m_area;
			n_movable++;
		}
	}
	avg_area = sum_area / n_movable;
			
	for(unsigned int i = 0; i < m_pDB->m_modules.size() ; i++ ){
// 		if ( m_pDB->m_modules[i].GetWidth() > theshold * _Rgn_width ||  //found a macro
// 			   m_pDB->m_modules[i].GetHeight() > theshold * _Rgn_height     )	
	//if(( (m_pDB->m_modules[i].m_isFixed ) && (!m_pDB->m_modules[i].m_isOutCore )) || 
	if(( (m_pDB->m_modules[i].m_isFixed ) && (!m_pDB->BlockOutCore( i ) )) || 
		(isMacro(i) &&(!m_pDB->m_modules[i].m_isCluster)))
		{
			VCG.AddNodes( m_pDB->m_modules[i].GetY(),i,
				      m_pDB->m_modules[i].GetX(),m_pDB->m_modules[i].GetX() + m_pDB->m_modules[i].GetWidth(),
				      m_pDB->m_modules[i].GetHeight(),
				      m_pDB->m_modules[i].m_isFixed
				    );

			m_ModuleID.push_back(i);
			if(!m_pDB->m_modules[i].m_isFixed )
				m_ModuleID_set.insert(i);
			HCG.AddNodes( m_pDB->m_modules[i].GetX(),i,
				      m_pDB->m_modules[i].GetY(),m_pDB->m_modules[i].GetY() + m_pDB->m_modules[i].GetHeight(),
					m_pDB->m_modules[i].GetWidth(),
					m_pDB->m_modules[i].m_isFixed
				    );
			//HCG.m_ModuleID.push_back(i);
			
		}
	}
	HCG.SortNodes();
	VCG.SortNodes();
	HCG.ReserveMemory();
	VCG.ReserveMemory();
	HCG.m_pri_thres = 0.2;
	VCG.m_pri_thres = 0.2;
	
}




void CConstraintGraph::AddNodes( double value, int moduleID, double low, double high ,double length, bool fixed)
{
	m_nodes.push_back(CCGraphNode(value,moduleID , low,  high ,length, fixed));
	
}
CCGraphNode::CCGraphNode(){		//null constructor
};

CCGraphNode::CCGraphNode( double value, int moduleID,double low, double high ,double length , bool fixed)
{
	m_moduleID = moduleID;
	m_value = value; 
	m_low = low;
	m_high = high;
	m_length = length;
	m_cx = value + length * 0.5;	//for plot only  HCG only!!
	m_cy = (low+high)/2.;	//for plot only		HCG only!!
	m_outedge.clear();
	m_inedge.clear();
	m_outedge.clear();
	m_isFixed = fixed;
	if (fixed){
		m_Left = m_cx;
		m_Right = m_cx;
	}
	m_irresolvable = false;

// 	//m_edgeweight.clear();
}


bool CConstraintGraph::Nodes_Greater(const CCGraphNode & s1,const CCGraphNode & s2 )
{
	cout << "s1.m_moduleID:" << s1.m_moduleID <<endl;
	cout << "s2.m_moduleID:" << s2.m_moduleID <<endl;
//         if (s1.m_moduleID == SINK ){
//                 return false;
//         }else if (s2.m_moduleID == SINK){
//                 return true;
//         }else if (s1.m_moduleID == SOURCE){
//                 return true;
//         }else if (s2.m_moduleID == SOURCE){
//                 return false;
//         }
//         else
//         {
                //return s1.m_value > s2.m_value;
                return s1.m_cx > s2.m_cx;
//         }

}
	
void CConstraintGraph::SortNodes( )
{

	sort(m_nodes.begin(),m_nodes.end(), global_lesser);
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++ ){
		m_ModuleID2NodeID[m_nodes[i].m_moduleID] = i;
	}
}

bool CCGraphNode::Greater(const CCGraphNode & s1,const CCGraphNode & s2 )
{
	return s1.m_value > s2.m_value;
}


void CConstraintGraph::CGConstruct( )
{
	double _x_overlap;
	double _y_dist;
	bool _anymovable;
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++ ){
		//printf("Processing : %d\n",m_nodes[i].m_moduleID);
		for (unsigned int j = i+1; j < m_nodes.size() ; j++ ){

				_x_overlap = m_nodes[i].m_value + m_nodes[i].m_length - m_nodes[j].m_value;
				_y_dist = DistanceNodeRow(m_nodes[i],m_nodes[j]);
				_anymovable = ( (!(m_nodes[i].m_isFixed)) || (!(m_nodes[j].m_isFixed)));
				if (_anymovable){
					if (NodeIntersect(m_nodes[i] ,m_nodes[j])){		// inter-row-section
//						printf("\tIntersection : %d -> %d \n",m_nodes[i].m_moduleID,m_nodes[j].m_moduleID);
//						printf("\tDistance to NodeRow : %f \n", DistanceNodeRow(m_nodes[i],m_nodes[j]));
						
						if( _y_dist >= _x_overlap  )
							
							if (_x_overlap > 0)
							//if (_x_overlap < 0)
								AddEdge(m_nodes[i],m_nodes[j],0.0); //overlap
							else
								AddEdge(m_nodes[i],m_nodes[j],(-_x_overlap)/m_nodes[i].m_length);
					}else{		//outside the row;
//						printf("NotIntersection : %d -> %d \n",m_nodes[i].m_moduleID,m_nodes[j].m_moduleID);
//						printf("\tDistance to NodeRow : %d \t", DistanceNodeRow(m_nodes[i],m_nodes[j]));
//						printf("\tDistance to Node : %d \n", m_nodes[j].m_value -( m_nodes[i].m_value + m_nodes[i].m_length  ));
						if(
						(_x_overlap < 0) && 
						(  _y_dist < (-_x_overlap)  ) ){
							
							AddEdge(m_nodes[i],m_nodes[j],(-_x_overlap)/m_nodes[i].m_length );
							
						}
	
					}
				}
		}
	}
}



bool CMacroLegal::Legalize( vector<int>& legaled_vector)
{
	bool _sat_H,_sat_V,_fix_type = false;
	int level =0;
	double obj = 1E20;
	double pre_obj = 1E20;
	int _run = 0;
	const int max_level = 15;
	int max_adj = m_AdjRun;
	bool adj_fail = false;
	bool lp_fail = false;
	bool ml_fail = false;
	const int esc = 27;
	const int column  = max_adj+5;
	int _try_DML = 0;
	bool bDML=true;
	char filname[100];
	CTetrisLegal* _TDML  = new CTetrisLegal(*m_pDB);
	AutoRowHeight();
// 	for(unsigned int z =0 ; z < ; z++)
	while((obj != 0 ) && !(pre_obj - obj == 0 && pre_obj < 1))
	{
		cout << "Macro Legalization -- level:" << level;
		
		Init();	
 		cout << "\t macro rowheight: " << m_MacroRowHeight;
 		cout << "\t # of macros: " << HCG.m_nodes.size() <<endl;
		if (level == 0 & param.bPlot){
			sprintf(filname,"%s_ML-Init.plt",param.outFilePrefix.c_str());
			HCG.OutputGraph(filname,-1);
		}
		if (_try_DML == 1){
			m_pDB->RemoveFixedBlockSite();
			bDML = _TDML->MacroShifter(m_MacroRowHeight, false);
// 			m_DML->macroLegal(-1,m_ModuleID_set);				//dimond search macro legalizer
			ReloadModulePosition(); 		//update Legalized macro position
			if (bDML== false){
				ml_fail = true;
			}
				break;
		}
 		VCG.m_pri_thres  = 1E22;
 		HCG.m_pri_thres  = 1E22;
		HCG.nCGConstruct( );
//		VCG.nCGConstruct( );
		VCG.CGExclusive(HCG);
//		HCG.RemoveDummyEdge();
//		VCG.RemoveDummyEdge();
		CRect Rect0(m_pDB->m_coreRgn.top,m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.right);
		HCG.CGAddSourceSink(Rect0);
		CRect Rect1(m_pDB->m_coreRgn.right,m_pDB->m_coreRgn.left,m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.top);
		VCG.CGAddSourceSink(Rect1);
//		cout << "Analyzing HCG:\t";
		_sat_H = HCG.CGAnalyze();
//		cout << "Analyzing VCG:\t";
		_sat_V = VCG.CGAnalyze();
// 		sprintf(filname,"%s-%d.plt","HCG",level);
// 		sprintf(filname2,"%s-%d.plt","VCG",level);
// 		HCG.OutputGraph(filname,-100);
// 		VCG.OutputGraph(filname2,-100);


	
		HCG.m_pri_thres  *= pow(0.8,level);
		VCG.m_pri_thres  *= pow(0.8,level);
//		cout << "Effective Edge Weight:" <<HCG.m_pri_thres <<endl;
		_run = 0;
		if (param.bShow)
			printf("HCG:%c[%dCVCG:",esc,column-4);
		else
			printf("\t===");
		fflush(0);
// 			sprintf(filname,"%s-%d-%d.plt","HCG",level,_run);
// 			sprintf(filname2,"%s-%d-%d.plt","VCG",level,_run);
// 			HCG.OutputGraph(filname,-100);
// 			VCG.OutputGraph(filname2,-100);
			_run++;
		while (_sat_V == false ||_sat_H == false ){	//_fix_type true for VCG
			if(HCG.m_most_neg_slack < VCG.m_most_neg_slack){
				_fix_type = false;
			}else{
				_fix_type = true;
			}
			if (_fix_type == true){ 
// 				cout << "Analyzing & modifing VCG ....\n";

				bFixEdge(VCG,HCG,false);	
			}
			else{
// 				cout << "Analyzing & modifing HCG ....\n";

				bFixEdge(HCG,VCG,false);	
			}
// 			cout << "HCG:" << HCG.m_most_neg_slack <<endl;	
 //			cout << "VCG:" << VCG.m_most_neg_slack <<endl;
//			cout << "\nHCG-" ;
				_sat_H = HCG.CGAnalyze();
				if (param.bShow)  printf("%c[%dD%c[30;%dm%c%c[m",esc,column,esc,HCG.GetANSI(),(_fix_type)?' ':'-',esc);
//			cout << "VCG-" ;
				_sat_V = VCG.CGAnalyze();
				if (param.bShow) printf("%c[%dC%c[30;%dm%c%c[m",esc,column-1,esc,VCG.GetANSI(),(_fix_type)?'-':' ',esc);
			//output progress bar...
			fflush(0);


// 			sprintf(filname,"%s-%d-%d.plt","HCG",level,_run);
// 			HCG.OutputGraph(filname,-100);
// 			sprintf(filname,"%s-%d-%d.plt","VCG",level,_run);
// 			VCG.OutputGraph(filname,-100);
// 			HCG.ShowGraph();
// 			cout << "===============================================================\n" ;
// 			VCG.ShowGraph();
			_run++;
			if (_run == max_adj){
				adj_fail= true;
				break;
					
			}

		}
		if (param.bShow) printf("%c[m%c[200;%dH",esc,esc,max_adj*2 + 10);
		if (adj_fail){
			if (_try_DML == 0){
				printf("try DML!\n");
				adj_fail = false;
				_try_DML++;
			}else{
				ml_fail = true;
				lp_fail = true;
				printf("FAILS!\n");
				break;
			}
		}else{
			if (_try_DML== 1)
				_try_DML++;
		}
			
		
		
//		cout << "Finishing....\t";
		_sat_V = VCG.CGAnalyze();
		_sat_H = HCG.CGAnalyze();
//		sprintf(filname,"%s-FIXED.plt","HCG");
//		sprintf(filname2,"%s-FIXED.plt","VCG");
		if(level == 0){
// 			HCG.OutputGraph(filname.c_str(),-100);
// 			VCG.OutputGraph(filname2.c_str(),-100);
	 	}
	
		if (_try_DML != 1 || true){
			pre_obj = obj;
			obj = MacroLP();
			if (obj < 0){
				ml_fail = true;
				break;
			}
					
		}
 		sprintf(filname, "%s_MLSol_level%d.plt",param.outFilePrefix.c_str(),level);
		if(param.bPlot){
 			HCG.OutputSolutionGraph(filname,true);
			//m_pDB->OutputGnuplotFigure(filname3,false,false);
		}
		
	// 	while (pack_count > 0 ){
	// 		cout << "Relaxed " << pack_count << "edges\n";
	// 		pack_count = -1000000;
	// 		cout << "TDSR HCG\n";
	// 		pack_count += TestDeadSpaceRemove( HCG,VCG);
	// 		cout << "TDSR VCG\n";
	// 		pack_count += TestDeadSpaceRemove( VCG,HCG);
	// 		MacroLP();
	// 	}
	
		
// 	 	HCG.OutputSolutionGraph("MLSol-Final.plt",true);
		//if (_try_DML == 1){
			ApplyPlaceDB(false);
		//}

		level++;
		if (level == max_level){
			ml_fail=true;
			break;	
		}

/*		HCG.ReInit();
		VCG.ReInit();*/
	
	}
 	if(param.bPlot)
 	 	HCG.OutputSolutionGraph("ML-Final.plt",true);
	
	
	
	if (ml_fail){
		cout << "\tMacro Legalization Fails!\n" ;
	}
	else{
		for (set<int>::iterator ite = m_ModuleID_set.begin(); ite != m_ModuleID_set.end(); ++ite){
			legaled_vector.push_back(*ite);
		}
		ApplyPlaceDB(false); // (kaie) 2009-07-23
		//ApplyPlaceDB(true);

	}
	OffsetRestore();
	return !ml_fail;

}

bool CConstraintGraph::NodeIntersect( CCGraphNode & n1, CCGraphNode & n2 )
{
	bool __inter = true;
	if( n1.m_low > n2.m_high  || n2.m_low > n1.m_high )
		__inter = false;	
	return __inter;
	
}

bool CConstraintGraph::NodeOverlap( CCGraphNode & n1, CCGraphNode & n2 )
{
	bool __overlap = true;
	if( n1.m_low >= n2.m_high  || n2.m_low >= n1.m_high )
		__overlap = false;	
	if (n1.m_value >= n2.m_value + n2.m_length || n2.m_value >= n1.m_value + n1.m_length)
		__overlap = false;
	return __overlap ;

	
}

void CConstraintGraph::AddEdge( CCGraphNode & s, CCGraphNode & t, double priority )
{
	double _edgeweight = (s.m_length + t.m_length  ) / 2;
	s.m_outedge.push_back(CCGraphEdge(s.m_moduleID,t.m_moduleID,_edgeweight,priority));
	t.m_inedge.push_back(CCGraphEdge(s.m_moduleID,t.m_moduleID,_edgeweight,priority));
	s.m_edge_moduleID.insert(t.m_moduleID);
	t.m_edge_moduleID.insert(s.m_moduleID);
	//s.m_edgeweight.push_back((s.m_length + t.m_length  ) / 2 );
	
}

void CConstraintGraph::CGExclusive( CConstraintGraph & CG )
{
	int __CG_nodeID;
	double _y_overlap;
	CCGraphNode* __CG_node;
	bool _anymovable;
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++){
		for (unsigned int j= i+1 ; j < m_nodes.size() ;j++){
			__CG_nodeID = CG.m_ModuleID2NodeID[m_nodes[j].m_moduleID];
			__CG_node = &(CG.m_nodes[__CG_nodeID]);
			 _y_overlap = m_nodes[i].m_value + m_nodes[i].m_length - m_nodes[j].m_value;
			_anymovable = ( (!(m_nodes[i].m_isFixed)) || (!(m_nodes[j].m_isFixed)));
			if (__CG_node->m_edge_moduleID.find(m_nodes[i].m_moduleID) == __CG_node->m_edge_moduleID.end() &&  _anymovable)
				if (NodeOverlap(m_nodes[i],m_nodes[j])){
					AddEdge(m_nodes[i],m_nodes[j],0.0);
				}else{
					AddEdge(m_nodes[i],m_nodes[j],(-_y_overlap)/m_nodes[i].m_length);
				}
			
		}
	}	
}

double CConstraintGraph::DistanceNodeRow( CCGraphNode & node_row, CCGraphNode & n )
{
	double __dist;
	double __value;
	if (NodeIntersect(node_row,n)){
		if (n.m_low < node_row.m_low){
			__dist = n.m_high - node_row.m_low;
			__value = (__dist > node_row.m_high-node_row.m_low )? node_row.m_high-node_row.m_low : __dist;
			
			
		}else{
			__dist =  node_row.m_high - n.m_low;
			__value = (__dist > n.m_high-n.m_low )? n.m_high-n.m_low : __dist;
			
		}
	}else{
		if (n.m_low > node_row.m_high){
			__value =  n.m_low - node_row.m_high;
		}else{
			__value = node_row.m_low - n.m_high;
		}
			
	}
	
	return __value;
	
}

void CConstraintGraph::CGAddSourceSink(CRect _rect )
{	
	//AddNodes( m_pDB->m_coreRgn.right, SINK,m_pDB->m_coreRgn.bottom,m_pDB->m_coreRgn.top ,0.0 , true);
	AddNodes(_rect.right, SINK,_rect.bottom,_rect.top ,0.0 , true);
	int b= SINK;
	m_ModuleID2NodeID[b]=m_nodes.size() -1;
	for (unsigned int i = 0 ; i < m_nodes.size()-1;i++){	 // for all nodes except SINK
		if (m_nodes[i].m_outedge.size() == 0 ){
			AddEdge(m_nodes[i],m_nodes[m_nodes.size() -1],0);
		}
	}
	m_ModuleID2NodeID[(int)SINK] = m_nodes.size() -1;
	m_sink = m_nodes.size()-1;


	AddNodes( _rect.left, SOURCE ,_rect.bottom ,_rect.top, 0.0, true);
	int a = SOURCE;
	m_ModuleID2NodeID[a]=m_nodes.size() -1;
	for (unsigned int i = 0 ; i < m_nodes.size()-2;i++){	 // for all nodes except SOURCE&SINK
		if (m_nodes[i].m_inedge.size() == 0 ){
			AddEdge(m_nodes[m_nodes.size() -1],m_nodes[i],0);
			
		}
	}
	m_ModuleID2NodeID[(int)SOURCE] = m_nodes.size() -1;
	m_source = m_nodes.size() -1;



}

void CConstraintGraph::OutputGraph( const char* filename, int NodeID){
	
    FILE* out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Error, cannot open output file: " << filename << endl;
	return;
    }

//     printf( "Output Constraint figure: %s\n", filename );

    // output title
    int __n_edges = 0;
    for( unsigned int i = 0 ; i < m_nodes.size() ; i++){
    	__n_edges+= m_nodes[i].m_outedge.size();
    }
    fprintf( out, "\nset title \" %s, block= %d, edges= %d  \" font \"Times, 22\"\n\n",
	    filename, (int)m_nodes.size(), __n_edges );

    fprintf( out, "set size ratio 1\n" );
    fprintf( out, "set nokey\n\n" ); 


	//output ModuleID
    for( int i=0; i<(int)m_nodes.size(); i++ )
    {
	if (!m_nodes[i].m_isFixed)
    	fprintf( out, "set label '%d' at %12.3f,%12.3f center\n", 
    	m_nodes[i].m_moduleID,m_nodes[i].m_cx,m_nodes[i].m_cy);
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
    


    fprintf( out, "plot[:][:] '-' w l lt 4, '-' w l lt 6, '-' w l lt 2, '-' w l lt 3\n\n" ); 
 

    // output Core region
    fprintf( out, "\n# core region\n" ); 
    fprintf( out, "%12.3f, %12.3f\n", m_lower_bound, m_lower_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_lower_bound, m_upper_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_upper_bound, m_upper_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_upper_bound, m_lower_sidebound ); 
    fprintf( out, "%12.3f, %12.3f\n\n",m_lower_bound, m_lower_sidebound);
    fprintf( out, "\nEOF\n\n" );







	// output degenerated edges
    fprintf( out, "\n# degenerated edges\n" ); 
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
    fprintf( out, "%12.3f, %12.3f\n\n",m_lower_bound, m_lower_sidebound); //dummy point 
    fprintf( out, "\nEOF\n\n" );







	// output effective edges
    fprintf( out, "\n# effective edges\n" ); 
    
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
    fprintf( out, "%12.3f, %12.3f\n\n",m_lower_bound, m_lower_sidebound); //dummy point 
    fprintf( out, "\nEOF\n\n" );


    // output macro position
    fprintf( out, "\n# macros\n" ); 
    double x, y, w, h;
    for( int i=0; i<(int)m_nodes.size()-2; i++ )
    {
// 	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
// 	    continue;

	x = m_nodes[i].m_value;
	y = m_nodes[i].m_low;
	w = m_nodes[i].m_length;
	h = m_nodes[i].m_high - m_nodes[i].m_low;


	    fprintf( out, "%12.3f, %12.3f\n", x, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
	    fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
	    fprintf( out, "%12.3f, %12.3f\n\n", x, y );

	    //fprintf( out, "%12.3f, %12.3f\n", x+w*0.75, y+h );
	    //fprintf( out, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );

    }
    fprintf( out, "\nEOF\n\n" );




    fprintf( out, "pause -1 'Press any key'" );
    fclose( out );

}


bool CCGraphNode::Lesser(const CCGraphNode& s1, const CCGraphNode& s2){
	return !Greater(s1,s2);
}

bool CConstraintGraph::Nodes_Lesser(const CCGraphNode& s1 ,const CCGraphNode& s2){
	 return !Nodes_Greater(s1,s2);
}

void CConstraintGraph::ReserveMemory( )
{
	m_nodes.reserve(m_nodes.size()+2);//for SOURCE and SINK
}

void CConstraintGraph::ShowGraph( )
{
	for(unsigned int i = 0 ; i < m_nodes.size(); i++){
		cout << "Node:" << i << " moduleid:" << m_nodes[i].m_moduleID << "\tvalue:" << m_nodes[i].m_value << "\tlength:" << m_nodes[i].m_length <<endl;
		cout << "\tLeft:" <<  m_nodes[i].m_Left << "\tRight" <<  m_nodes[i].m_Right  << endl;
		cout << "=====out-edges========" <<endl;
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_outedge.begin(); ite !=m_nodes[i].m_outedge.end();++ite ){
			cout << "\t" << ite->m_to_moduleID;
		}

		cout <<endl <<  "======in-edges========" <<endl;;
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_inedge.begin(); 
			ite !=m_nodes[i].m_inedge.end();++ite ){
			cout << "\t" << ite->m_from_moduleID;
		}
		cout <<endl<<endl;

	}
}

double CMacroLegal::MacroLP( )
{
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
	int __moduleID;
	const int _cx =1;
	const int _cy =2;
	const int _dx =3 ;
	const int _dy =4;
	if(ret == 0){
		for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
			__moduleID = HCG.m_nodes[i].m_moduleID;
			sprintf(var_name, "%d_cx",__moduleID);
			set_col_name(lp,i*4+_cx,var_name);
			sprintf(var_name, "%d_cy",__moduleID);
			set_col_name(lp,i*4+_cy,var_name);
			sprintf(var_name, "%d_dx",__moduleID);
			set_col_name(lp,i*4+_dx,var_name);
			sprintf(var_name, "%d_dy",__moduleID);
			set_col_name(lp,i*4+_dy,var_name);
		}
		
		colno = (int *)malloc(Ncol * sizeof(*colno));
		row = (REAL *)malloc(Ncol * sizeof(*row));

		if ((colno == NULL) || (row == NULL))
			ret = 2;
		
	}

	list<CCGraphEdge>::iterator ite ;
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
			if(!add_constraintex(lp,j,row,colno,GE,HCG.m_nodes[i].m_cx))
				ret = 3;
			j = 0;
			colno[j] = i*4 +_cx;
			row[j++] = 1;
			colno[j] = i*4 +_dx;
			row[j++] = -1;
			if(!add_constraintex(lp,j,row,colno,LE,HCG.m_nodes[i].m_cx))
				ret = 3;
				
			//chip bound constraint	
			if(HCG.m_nodes[i].m_isFixed){
				j = 0;
				colno[j] = i*4 +_cx;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,EQ,
				    HCG.m_nodes[i].m_cx))
					ret = 3;	
			}else{
			j = 0;
			colno[j] = i*4 +_cx;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,GE,
			HCG.GetLowerBoound() + 0.5 * HCG.m_nodes[i].m_length))
				ret = 3;	
				
			j = 0;
			colno[j] = i*4 +_cx;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,LE,
			HCG.GetUpperBoound()- 0.5 * HCG.m_nodes[i].m_length))
				ret = 3;	
			}
			
			for ( ite = HCG.m_nodes[i].m_outedge.begin(); 	//edges
					ite !=HCG.m_nodes[i].m_outedge.end() && ret ==0;++ite ){

				if (ite->m_to_moduleID != HCG.SINK && ite->m_priority <= HCG.m_pri_thres && ite->m_slack >= 0){ // not point so SINK
					_NodeID = HCG.m_ModuleID2NodeID[ite->m_to_moduleID];
					//cout << "\tEDGE:" << HCG.m_nodes[i].m_moduleID << "->" << *ite <<endl;
					j = 0;
					colno[j] = _NodeID*4 +_cx;
					row[j++] = 1;
					colno[j] = i*4 +_cx;
					row[j++] = -1;
					if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight))
						ret = 3;
				}
				
				
			}
			
		}
		
		// then y!!
		int _hNodeID;
		for (unsigned int i = 0 ; (i < VCG.m_nodes.size()-2 && ret == 0 );i++){
			_hNodeID = HCG.m_ModuleID2NodeID[VCG.m_nodes[i].m_moduleID];
			// i stands for NodeID
			//dummy dy constraint
			j = 0;
			colno[j] = _hNodeID*4 +_cy;
			row[j++] = 1;
			colno[j] = _hNodeID*4 +_dy;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,GE,VCG.m_nodes[i].m_cx))
				ret = 3;
			j = 0;
			colno[j] = _hNodeID*4 +_cy;
			row[j++] = 1;
			colno[j] = _hNodeID*4 +_dy;
			row[j++] = -1;
			if(!add_constraintex(lp,j,row,colno,LE,VCG.m_nodes[i].m_cx))
				ret = 3;
				
			//chip bound constraint	
			if (VCG.m_nodes[i].m_isFixed){
				j = 0;
				colno[j] = _hNodeID*4 +_cy;
				row[j++] = 1;
				if(!add_constraintex(lp,j,row,colno,EQ,VCG.m_nodes[i].m_cx))
					ret = 3;	

			}else{
			j = 0;
			colno[j] = _hNodeID*4 +_cy;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,GE,VCG.GetLowerBoound()
			+ 0.5 * VCG.m_nodes[i].m_length))
				ret = 3;	
				
			j = 0;
			colno[j] = _hNodeID*4 +_cy;
			row[j++] = 1;
			if(!add_constraintex(lp,j,row,colno,LE,VCG.GetUpperBoound()
			- 0.5 * VCG.m_nodes[i].m_length))
				ret = 3;	
			}
				
			for ( ite = VCG.m_nodes[i].m_outedge.begin(); 	//edges
					ite !=VCG.m_nodes[i].m_outedge.end() && ret ==0;++ite ){
				//if(0){
				if (ite->m_to_moduleID != VCG.SINK && ite->m_priority <= VCG.m_pri_thres
						&& ite->m_slack >= 0){ // not point so SINK
					_NodeID = HCG.m_ModuleID2NodeID[ite->m_to_moduleID];	//lp use HCG nodeID!!
					j = 0;
					colno[j] = _NodeID*4 +_cy;
					row[j++] = 1;
					colno[j] = _hNodeID*4 +_cy;
					row[j++] = -1;
					if(!add_constraintex(lp,j,row,colno,GE,ite->m_weight))
						ret = 3;
				}
				
				
			}
			
		}
		
	}
	
	set_add_rowmode(lp,FALSE);
	int _hNodeID;
	if(ret == 0){
		j = 0;
		for (unsigned int i = 0 ; (i < HCG.m_nodes.size()-2 && ret == 0 );i++){
		
// 		colno[j] = i*4 + _dx;
// 		row[j++] = 1;
// 		colno[j] = i*4 + _dy;
// 		row[j++] = 1;
/// 		weighted objective function
		_hNodeID = HCG.m_ModuleID2NodeID[VCG.m_nodes[i].m_moduleID];
 		colno[j] = i*4 + _dx;
 		row[j++] = HCG.m_nodes[i].m_length;
 		colno[j] = _hNodeID*4 + _dy;
 		row[j++] = VCG.m_nodes[i].m_length;
		}
		if(!set_obj_fnex(lp,j,row,colno))
			ret = 4;
	}
	
	
	
	if(ret == 0){
		set_minim(lp);
// 		write_LP(lp,stdout);
// 		print_lp(lp);
	}
	set_verbose(lp,IMPORTANT);
	
	
	ret = solve(lp);
	if (ret == OPTIMAL){
//		cout << "OPTIMAL SOLUTION FOUND!!!" <<endl;
		ret = 0;
	
	}
	else{
//		cout << "NO SOLUTION FESIBAL!!" <<endl;
		cout << "LP-FAILS!" << endl;
		ret = 5;
	}
	int _vNodeID;
	double obj =get_objective(lp) ;
	if (ret == 0 ){
		//cout << "Objective value:" << get_objective(lp) << endl;
		cout << get_objective(lp) << endl;
		get_variables(lp,row);
		for ( j = 0 ; j < Ncol ; j++){
			//cout << get_col_name(lp,j+1) << ":" << row[j] <<endl;
		}
		for (unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
			HCG.m_nodes[i].m_solution_cx = row[i*4+_cx-1];
			HCG.m_nodes[i].m_solution_cy = row[i*4+_cy-1];
			HCG.m_nodes[i].m_solution_x = row[i*4+_cx-1] - 0.5 * HCG.m_nodes[i].m_length ;
			HCG.m_nodes[i].m_solution_y = row[i*4+_cy-1] - 0.5 * (HCG.m_nodes[i].m_high - HCG.m_nodes[i].m_low);
			_vNodeID = VCG.m_ModuleID2NodeID[HCG.m_nodes[i].m_moduleID];
			VCG.m_nodes[_vNodeID].m_solution_cy = row[i*4+_cx-1];
			VCG.m_nodes[_vNodeID].m_solution_cx = row[i*4+_cy-1];
			VCG.m_nodes[_vNodeID].m_solution_x = VCG.m_nodes[_vNodeID].m_solution_cy - 0.5 * VCG.m_nodes[_vNodeID].m_length ;
			VCG.m_nodes[_vNodeID].m_solution_y = VCG.m_nodes[_vNodeID].m_solution_cy - 0.5 * (VCG.m_nodes[_vNodeID].m_high - VCG.m_nodes[_vNodeID].m_low);

			
		}
		
	}

	free(row);
	free(colno);
	delete_lp(lp);
	if (ret == 0 )
	return obj;
	else 
	return -1;
	
}

void CConstraintGraph::OutputSolutionGraph( const char * filename, bool bWithMove )
{

	FILE* out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Error, cannot open output file: " << filename << endl;
	return;
    }

    printf( "Output Macro Legal Solution figure: %s\n", filename );

    // output title
    int __n_edges = 0;
    for( unsigned int i = 0 ; i < m_nodes.size() ; i++){
    	__n_edges+= m_nodes[i].m_outedge.size();
    }
    fprintf( out, "\nset title \" %s, block= %d, edges= %d  \" font \"Times, 22\"\n\n",
	    filename, (int)m_nodes.size(), __n_edges );

    fprintf( out, "set size ratio 1\n" );
    fprintf( out, "set nokey\n\n" ); 


	//output ModuleID
    for( int i=0; i<(int)m_nodes.size()-2; i++ )
    {
	    if(!m_nodes[i].m_isFixed)
    	fprintf( out, "set label '%d' at %12.3f,%12.3f center\n", 
    	m_nodes[i].m_moduleID,m_nodes[i].m_solution_cx,m_nodes[i].m_solution_cy);
    }

	if (bWithMove)
    		fprintf( out, "plot[:][:] '-' w l lt 4, '-' w l lt 3, '-' w l lt 7\n\n" ); 
    	else
 		fprintf( out, "plot[:][:] '-' w l lt 4, '-' w l lt 3\n\n" ); 

    // output Core region
    fprintf( out, "\n# core region\n" ); 
    fprintf( out, "%12.3f, %12.3f\n", m_lower_bound, m_lower_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_lower_bound, m_upper_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_upper_bound, m_upper_sidebound );
    fprintf( out, "%12.3f, %12.3f\n", m_upper_bound, m_lower_sidebound ); 
    fprintf( out, "%12.3f, %12.3f\n\n",m_lower_bound, m_lower_sidebound);
    fprintf( out, "\nEOF\n\n" );



    // output macro position
    fprintf( out, "\n# macros\n" ); 
    double x, y, w, h;
    for( int i=0; i<(int)m_nodes.size()-2; i++ )
    {

	x = m_nodes[i].m_solution_x;
	y = m_nodes[i].m_solution_y;
	w = m_nodes[i].m_length;
	h = m_nodes[i].m_high - m_nodes[i].m_low;

		fprintf( out, "#%d\t%12.3f, %12.3f %12.3f - %12.3f\n",m_nodes[i].m_moduleID, x, y ,w,h);
	    fprintf( out, "%12.3f, %12.3f\n", x, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y );
	    fprintf( out, "%12.3f, %12.3f\n", x+w, y+h );
	    fprintf( out, "%12.3f, %12.3f\n", x, y+h ); 
	    fprintf( out, "%12.3f, %12.3f\n\n", x, y );

    }
    fprintf( out, "\nEOF\n\n" );
    
    
        // output move
    fprintf( out, "\n# macro legal move\n" ); 
    double x1, y1, x2, y2;
    for( int i=0; i<(int)m_nodes.size()-2; i++ )
    {
	x1 = m_nodes[i].m_cx;
	y1 = m_nodes[i].m_cy;
	x2 = m_nodes[i].m_solution_cx;
	y2  = m_nodes[i].m_solution_cy;
	fprintf( out, "%12.3f, %12.3f\n", x1, y1 );
	fprintf( out, "%12.3f, %12.3f\n\n", x2, y2 );



    }
    fprintf( out, "\nEOF\n\n" );
    
    
    fprintf( out, "pause -1 'Press any key'" );
    fclose( out );
}

void CMacroLegal::Apply( )
{	
	int _moduleID;
	double _x,_y;
	int _h_nid, _v_nid;
	for (unsigned int i = 0 ; i < m_ModuleID.size(); i++){
		_moduleID = m_ModuleID[i];
		_h_nid = HCG.m_ModuleID2NodeID[_moduleID];
		_v_nid = VCG.m_ModuleID2NodeID[_moduleID];
		_x = HCG.m_nodes[_h_nid].m_solution_x;
		_y = HCG.m_nodes[_h_nid].m_solution_y;
		HCG.m_nodes[_h_nid].m_value = _x;
		HCG.m_nodes[_h_nid].m_low = _y;
		HCG.m_nodes[_h_nid].m_high = _y + VCG.m_nodes[_v_nid].m_length;
		
		VCG.m_nodes[_v_nid].m_value = _y;
		VCG.m_nodes[_v_nid].m_low = _x;
		VCG.m_nodes[_v_nid].m_high = _x + HCG.m_nodes[_h_nid].m_length;
	
	}
}

void CMacroLegal::ApplyPlaceDB(bool setfixed )
{	
	for(unsigned int i = 0 ; i < HCG.m_nodes.size()-2;i++){
		m_pDB->SetModuleLocation(HCG.m_nodes[i].m_moduleID,
		HCG.m_nodes[i].m_solution_x,HCG.m_nodes[i].m_solution_y);
		if (setfixed)
			m_pDB->SetModuleFixed(HCG.m_nodes[i].m_moduleID);
	}
}

CCGraphEdge::CCGraphEdge(int from_moduleID, int to_moduleID, double weight, double priority )
{
	m_from_moduleID = from_moduleID;
	m_to_moduleID = to_moduleID;
	m_weight = weight;
	m_priority = priority;
}

bool CConstraintGraph::CGAnalyze( )
{
	double _bound_box= m_upper_bound -m_lower_bound;
	queue<CCGraphEdge> _queue;
	list<CCGraphEdge>::iterator ite;
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
	
	// forward
	for (ite = m_nodes[m_source].m_outedge.begin() ;
		ite!= m_nodes[m_source].m_outedge.end() ; ite++ ){
		_queue.push(*ite); 
	}
	_nodeID_Set.insert(m_source);
	int _nodeID_i,_nodeID_j;
	double _weight;
	double _most_neg_slack =0.0;
	while (_queue.empty() != true){
		_nodeID_i = m_ModuleID2NodeID[_queue.front().m_from_moduleID];
		_nodeID_j = m_ModuleID2NodeID[_queue.front().m_to_moduleID];
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
		//	cout << "\tModuleID:" << m_nodes[_nodeID_j].m_moduleID<< "LEFT:" <<m_nodes[_nodeID_j].m_Left <<endl;
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
		_queue.push(*ite); 
	}
	_nodeID_Set.insert(m_sink);
// 	cout << "backward\n";
	while (_queue.empty() != true){
		_nodeID_i = m_ModuleID2NodeID[_queue.front().m_to_moduleID];
		_nodeID_j = m_ModuleID2NodeID[_queue.front().m_from_moduleID];
		
		//TODO: backtrace ability
		if (_nodeID_ED_count[_nodeID_i] > 0 ){
			_queue.push(_queue.front());
			_queue.pop();
		}else{
			_queue.pop();
			_nodeID_ED_count[_nodeID_j]--;
			m_nodes[_nodeID_j].m_Right = min(m_nodes[_nodeID_j].m_Right , 
					m_nodes[_nodeID_i].m_Right -
					((m_nodes[_nodeID_j].m_length + m_nodes[_nodeID_i].m_length  ) / 2));
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
 	CCGraphEdge _most_neg_slack_edge(-1,-1,-1,-1);

	for (unsigned int i = 0 ; i < m_nodes.size() ; i++){
		for (ite = m_nodes[i].m_outedge.begin() ; ite!= m_nodes[i].m_outedge.end() ; ite++ ){
			ite->m_slack = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right -
					m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left-
					ite->m_weight;
			if (ite->m_slack < _most_neg_slack)
				_most_neg_slack_edge = *ite;
			_most_neg_slack = min(_most_neg_slack,ite->m_slack);
		}
	}

// 	_most_neg_slack_edge.ShowEdge();
//	cout << "_most_neg_slack:" <<_most_neg_slack << endl;
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++){
// 		cout << "ModuleID:" << m_nodes[i].m_moduleID << "\tLeft:" << m_nodes[i].m_Left << "\tRight:" 
// 		<< m_nodes[i].m_Right <<endl;
	}
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
			ite->m_slack = m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_Right -
					m_nodes[m_ModuleID2NodeID[ite->m_from_moduleID]].m_Left-
					ite->m_weight;
			ite->m_isCritical = false;
			//assert(ite->m_slack >= 0);
		
			_anymovable = ( (!(m_nodes[i].m_isFixed)) || (!(m_nodes[m_ModuleID2NodeID[ite->m_to_moduleID]].m_isFixed)));
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
	
	
	
	
	
}

void CMacroLegal::FixEdge(CConstraintGraph& _HCG,CConstraintGraph& _VCG)
{
	//FIXME:test only
	int _from_nodeID;
	int _to_nodeID;
	int _edge_count = 0;
	list<CCGraphEdge>::iterator ite;
	for (unsigned int i = 0 ; i < _VCG.m_nodes.size() ; i++){
		for (ite = _VCG.m_nodes[i].m_outedge.begin() ;
		ite!= _VCG.m_nodes[i].m_outedge.end() ; ite++ ){
			//cout << "CEHCKING: " <<ite->m_from_moduleID << "->" << ite->m_to_moduleID <<  "\t SLACK:" << ite->m_slack <<endl;
			if (ite->m_slack <= 0 ){
				cout << "ZERO SLACK: " <<ite->m_from_moduleID << "->" << ite->m_to_moduleID <<  "\t SLACK:" << ite->m_slack << endl; 

				_from_nodeID = _HCG.m_ModuleID2NodeID[ite->m_from_moduleID];
				_to_nodeID = _HCG.m_ModuleID2NodeID[ite->m_to_moduleID];

// 				cout << _VCG.m_nodes[_VCG.m_ModuleID2NodeID[ite->m_from_moduleID]].m_value << "->" 
// 				<< _VCG.m_nodes[_VCG.m_ModuleID2NodeID[ite->m_to_moduleID]].m_value <<endl;
// 				assert(_VCG.m_nodes[_VCG.m_ModuleID2NodeID[ite->m_from_moduleID]].m_value <
// 					_VCG.m_nodes[_VCG.m_ModuleID2NodeID[ite->m_to_moduleID]].m_value);
				
				_edge_count++;
		
			}

		}
	}



	cout << "edge_count:" << _edge_count <<endl; 
}

void CCGraphEdge::ShowEdge( )
{
	cout <<"EDGE:" << m_from_moduleID << "->" << m_to_moduleID << "\tWeight:" <<m_weight << "\tSlack:"<< m_slack <<endl;
	
}

void CMacroLegal::mFixEdge( CConstraintGraph & _HCG, CConstraintGraph & _VCG )
{
	int *_xadj= NULL;
	int *_adjncy= NULL;
	int *_adjwgt = NULL;
	int *_part = NULL;
	int _edge_count;
	int _vertex_count;
	map<int,int> _mvertex2moduleID;
	map<int,int> _moduleID2mvertex;
	set<int> _moduleID;
	int _adj_flag = 0;
	int _vertex_flag=0;
	set<int>::iterator ite;
	list<CCGraphEdge>::iterator ite2;
	int _to_moduleID;
	int _from_moduleID;
	int _nodeID;
	int _edgecut=INT_MAX;
	int zero = 0;
	int one = 1;
	int npart = 2;
	int option[5]= {0,0,0,0,0};
	cout <<"Critical Edges:" << _HCG.m_CriticalEdge.size()  <<endl;
	if(_HCG.m_CriticalEdge.size() != 0){
		if (_HCG.OverLoad  > 1 ){
			npart = 3;
		}else{
			npart = 2;
		}
		cout << "# of partition : " <<  npart <<endl;
		_edge_count = _HCG.m_CriticalEdge.size();
// 		_xadj = (int *)malloc(2*_edge_count * sizeof(*_xadj));
// 		_adjncy = (int *)malloc(2*_edge_count * sizeof(*_adjncy));
// 		_adjwgt = (int *)malloc(2*_edge_count * sizeof(*_adjwgt));
// 		_part = (int *)malloc(2*_edge_count * sizeof(*_part));
// 		_xadj = new int[2*_edge_count+1];
// 		_adjncy = new int[4*_edge_count+2];
// 		_adjwgt = new int[4*_edge_count+2];
// 		_part = new int[2*_edge_count+1];

		for ( int i = 0 ; i < _edge_count ; i++ ){	//Metis ID map create
			_to_moduleID = _HCG.m_CriticalEdge[i].m_to_moduleID;

			if(_moduleID.find(_to_moduleID) == _moduleID.end()){
				_moduleID.insert(_to_moduleID);
				_mvertex2moduleID[_vertex_flag] = _to_moduleID;
				_moduleID2mvertex[_to_moduleID] = _vertex_flag;
				_vertex_flag++;
			}
			_to_moduleID = _HCG.m_CriticalEdge[i].m_from_moduleID;
			if(_moduleID.find(_to_moduleID) == _moduleID.end()){
				_moduleID.insert(_to_moduleID);
				_mvertex2moduleID[_vertex_flag] = _to_moduleID;
				_moduleID2mvertex[_to_moduleID] = _vertex_flag;
				_vertex_flag++;
			}
		}
		_vertex_count = _vertex_flag;
		_vertex_flag = 0;
		
		for (int i = 0; i < _vertex_count ; i++){ //input prerparation
			_xadj[_vertex_flag++] = _adj_flag;
			_nodeID = _HCG.m_ModuleID2NodeID[_mvertex2moduleID[i]];
//			cout << "in-edge:" <<  _mvertex2moduleID[i] <<endl;
			for(ite2 = _HCG.m_nodes[_nodeID].m_inedge.begin() ;
				ite2 != _HCG.m_nodes[_nodeID].m_inedge.end() ; ++ite2){
				
				if(ite2->m_isCritical ){
					
//					ite2->ShowEdge();
					_adjncy[_adj_flag] = _moduleID2mvertex[ite2->m_from_moduleID];
					_adjwgt[_adj_flag++] = _VCG.CalcTransCost(ite2->m_from_moduleID,ite2->m_to_moduleID);
				}
				
			}
//			cout << "out-edge:" << _mvertex2moduleID[i] <<endl;
			for(ite2 = _HCG.m_nodes[_nodeID].m_outedge.begin() ;
				ite2 != _HCG.m_nodes[_nodeID].m_outedge.end() ; ++ite2){
				if(ite2->m_isCritical ){
//					ite2->ShowEdge();
					_adjncy[_adj_flag] = _moduleID2mvertex[ite2->m_to_moduleID];
					_adjwgt[_adj_flag++] = _VCG.CalcTransCost(ite2->m_from_moduleID,ite2->m_to_moduleID);
				}
			}			

		}
		_xadj[_vertex_flag] = _adj_flag;

		
/*
 		cout <<"xadj:";
 		for (int i = 0 ; i <= _vertex_flag;i++)
			cout << " " << _xadj[i];
 		cout <<"\nxadj:";
 		for (int i = 0 ; i < _vertex_flag;i++)
 			cout << " " << _mvertex2moduleID[i];
 		cout <<"\nadjncy:";
 		for (int i = 0 ; i < _adj_flag;i++)
 			cout << " " << _mvertex2moduleID[_adjncy[i]];
 		cout << endl;
 		cout <<"adjncy:";
 		for (int i = 0 ; i < _adj_flag;i++){
 			cout << i <<"-- " << _mvertex2moduleID[_adjncy[i]];
 			cout << ":" << _adjwgt[i];
 			cout << endl;
 
 		}
 		
 */		

		

			METIS_PartGraphRecursive(
				&_vertex_count,
				_xadj,
				_adjncy,
				NULL,
				_adjwgt,
				&one,
				&zero,
				&npart,
				option,
				&_edgecut,
				_part);
		
		

		printf("[%d] ",_edgecut);
		for(int i = 0 ; i < _vertex_count ; i++ ){
			//printf("%d:%d \n",i,_part[i]);
 		}
 		int _from_nodeID ;
                int _to_nodeID ;
		for ( int i = 0 ; i < _edge_count ; i++ ){	//Metis ID map create
			_to_moduleID = _HCG.m_CriticalEdge[i].m_to_moduleID;	
			_from_moduleID = _HCG.m_CriticalEdge[i].m_from_moduleID;
			_from_nodeID = _VCG.m_ModuleID2NodeID[_from_moduleID];	
			_to_nodeID = _VCG.m_ModuleID2NodeID[_to_moduleID];
			if (_part[_moduleID2mvertex[_to_moduleID]]!= _part[_moduleID2mvertex[_from_moduleID]]){
				
/*				cout << "Cut-edge:" ;*/
/* 				_HCG.m_CriticalEdge[i].ShowEdge();*/
				
				_HCG.RemoveEdge(_from_moduleID,_to_moduleID);
/* 				cout << "Adding edge...." ;*/
				 if (_from_nodeID <= _to_nodeID){
					_VCG.AddEdge( _VCG.m_nodes[_from_nodeID], _VCG.m_nodes[_to_nodeID],0);
					cout <<  _VCG.m_nodes[_from_nodeID].m_moduleID << "->" <<  _VCG.m_nodes[_to_nodeID].m_moduleID  << endl;
			
					}
				else	{
					_VCG.AddEdge( _VCG.m_nodes[_to_nodeID], _VCG.m_nodes[_from_nodeID],0);
					cout <<  _VCG.m_nodes[_to_nodeID].m_moduleID << "->" <<  _VCG.m_nodes[_from_nodeID].m_moduleID  << endl;
			
					}
				
				
			}
		}
		

		delete [] _xadj ;
		delete [] _adjncy;
		delete [] _adjwgt;
		delete [] _part;


	}
	
	
	
}

int CConstraintGraph::CalcTransCost( int moduleID1, int moduleID2 )
{
	int _nodeID1,_nodeID2,_nodeID_i,_nodeID_j;
	double _edge_weight;
	double _value;
	_nodeID1 = m_ModuleID2NodeID[moduleID1];
	_nodeID2 = m_ModuleID2NodeID[moduleID2];
	m_MaxTransCost = INT_MAX/m_nodes.size();
	if (_nodeID1 > _nodeID2){
		_nodeID_i = _nodeID2;
		_nodeID_j = _nodeID1;
	}else{
		_nodeID_i = _nodeID1;
		_nodeID_j = _nodeID2;
	}
	_edge_weight = (m_nodes[_nodeID_j].m_length +m_nodes[_nodeID_i].m_length )/2 ;
	
	if (moduleID1 < 0 ||moduleID2 < 0  ){

		return m_MaxTransCost;
	}
	else if (m_nodes[_nodeID_j].m_Right - m_nodes[_nodeID_i].m_Left -_edge_weight < 0 ){
	 	return (m_MaxTransCost /2 + (rand() % m_nodes.size()));
	 }else{
	 	_value = max(m_nodes[_nodeID_i].m_cx - m_nodes[_nodeID_j].m_Right + _edge_weight,0);
	 	_value += max(m_nodes[_nodeID_i].m_Left - m_nodes[_nodeID_j].m_cx + _edge_weight,0);
	 	_value += _edge_weight;
	 	return static_cast<int>(_value );
	 }
}

void CConstraintGraph::RemoveEdge( int moduleID_s, int moduleID_t )
{
	int _nodeID_s= m_ModuleID2NodeID[moduleID_s];
	int _nodeID_t = m_ModuleID2NodeID[moduleID_t];
	list<CCGraphEdge>::iterator ite;
	
	for (ite = m_nodes[_nodeID_s].m_outedge.begin() ;
		ite!= m_nodes[_nodeID_s].m_outedge.end() ; ite++ ){
		if (ite->m_to_moduleID == moduleID_t){
			 m_nodes[_nodeID_s].m_outedge.erase(ite--);
			if (m_nodes[_nodeID_s].m_outedge.size() == 0){
				AddEdge(m_nodes[_nodeID_s],m_nodes[m_ModuleID2NodeID[(int)SINK]]
				,0 );
				
			}
		}
	}
	for (ite = m_nodes[_nodeID_t].m_inedge.begin() ;
		ite!= m_nodes[_nodeID_t].m_inedge.end() ; ite++ ){
		if (ite->m_from_moduleID == moduleID_s){
			 m_nodes[_nodeID_t].m_inedge.erase(ite--);
			 if (m_nodes[_nodeID_t].m_inedge.size() == 0){
				AddEdge(m_nodes[m_ModuleID2NodeID[(int)SOURCE]],m_nodes[_nodeID_t]
				,0 );
				
			}
		}
	}
}

void CMacroLegal::ShowRelation( int moduleID1, int moduleID2 )
{
// 	int _HCG_nodeID1 = HCG.m_ModuleID2NodeID[moduleID1];
// 	int _HCG_nodeID2 = HCG.m_ModuleID2NodeID[moduleID2];


}

void CMacroLegal::ShowRelation( int moduleID )
{
	int _VCG_nodeID = VCG.m_ModuleID2NodeID[moduleID];
	int _HCG_nodeID = HCG.m_ModuleID2NodeID[moduleID];
	list<CCGraphEdge>::iterator ite;
	
	for (ite = HCG.m_nodes[_HCG_nodeID].m_outedge.begin() ;
		ite!= HCG.m_nodes[_HCG_nodeID].m_outedge.end() ; ite++){
		cout << "H:" << ite->m_from_moduleID << "->" << ite->m_to_moduleID <<endl;
	}
	for (ite = VCG.m_nodes[_VCG_nodeID].m_outedge.begin() ;
		ite!= VCG.m_nodes[_VCG_nodeID].m_outedge.end() ; ite++){
		cout << "V:" << ite->m_from_moduleID << "->" << ite->m_to_moduleID <<endl;
	}

}

void CMacroLegal::BoundaryGravity( bool LineGravity)
{
	set<int> HGravity;
	
	for (unsigned int i = 0 ; i < HCG.m_nodes.size();i++){
	
		if((min((HCG.m_nodes[i].m_cy - HCG.m_lower_sidebound), 
		     (HCG.m_upper_sidebound - HCG.m_nodes[i].m_cy)) >
		min((HCG.m_nodes[i].m_cx - HCG.m_lower_bound), 
		     (HCG.m_upper_bound - HCG.m_nodes[i].m_cx)) ) ||
		     LineGravity == false
		     ){
			if (HCG.m_nodes[i].m_cx > HCG.GetCenter())
				HCG.m_nodes[i].m_cx = HCG.GetUpperBoound();
			else
				HCG.m_nodes[i].m_cx = HCG.GetLowerBoound();
 			HGravity.insert(HCG.m_nodes[i].m_moduleID);
// 			cout << "Adding H-gravity " << HCG.m_nodes[i].m_moduleID << endl;
 		}
	}
	for (unsigned int i = 0 ; i < VCG.m_nodes.size();i++){
		if(HGravity.find(VCG.m_nodes[i].m_moduleID) == HGravity.end() ||
			LineGravity == false
		 ){
// 			cout << "Adding V-gravity " << VCG.m_nodes[i].m_moduleID << endl;
			if (VCG.m_nodes[i].m_cx > VCG.GetCenter())
				VCG.m_nodes[i].m_cx = VCG.GetUpperBoound();
			else
				VCG.m_nodes[i].m_cx = VCG.GetLowerBoound();
		}
	}

}

double CConstraintGraph::GetCenter( )
{
	return (m_upper_bound + m_lower_bound) /2.0;
}

void CMacroLegal::BoundaryGravityRecover( )
{
	for (unsigned int i = 0 ; i < HCG.m_nodes.size();i++){

			HCG.m_nodes[i].m_cx = HCG.m_nodes[i].m_value + 0.5* HCG.m_nodes[i].m_length;
	}
	for (unsigned int i = 0 ; i < VCG.m_nodes.size();i++){
		VCG.m_nodes[i].m_cx = VCG.m_nodes[i].m_value + 0.5* VCG.m_nodes[i].m_length;
		
	}
}

int CMacroLegal::TestDeadSpaceRemove(CConstraintGraph& _HCG ,CConstraintGraph& _VCG)
{

	list<CCGraphEdge>::iterator ite;
	double _host_low;
	double _host_high;
	double _client_low;
	double _client_high;
	int count = 0;
	
	for (unsigned int i = 0 ; i < _HCG.m_nodes.size();i++){
		for (ite = _HCG.m_nodes[i].m_outedge.begin();ite != _HCG.m_nodes[i].m_outedge.end();++ite ){
		//	cout << "Checking Edge:"; ite->ShowEdge();
			CCGraphNode& _From_Node = _HCG.m_nodes[_HCG.m_ModuleID2NodeID[ite->m_from_moduleID]];
			CCGraphNode& _To_Node = _HCG.m_nodes[_HCG.m_ModuleID2NodeID[ite->m_to_moduleID]];
			_host_low =  _From_Node.m_solution_y;
			_host_high = _From_Node.m_solution_y + _From_Node.m_high - _From_Node.m_low;
			_client_low = _To_Node.m_solution_y;
			_client_high = _To_Node.m_solution_y + _To_Node.m_high - _To_Node.m_low;
// 			cout << "\tChecking:\t";
// 			cout << _From_Node.m_moduleID << ":" <<  (_From_Node.m_solution_x + _From_Node.m_length);
// 			cout << "(" <<_From_Node.m_solution_x << "+" << _From_Node.m_length << ") ->" ;
// 			
//  			cout << _To_Node.m_moduleID << ":" <<  _To_Node.m_solution_x << endl;
			if ( (_client_low >= _host_high ||_client_high <= _host_low) && ( _To_Node.m_solution_x - (_From_Node.m_solution_x + _From_Node.m_length) < 1E-5 ) ){
				cout << "Removing Edge:" ;
				ite->ShowEdge();

				_HCG.m_nodes[i].m_outedge.erase(ite--);
				count++;
			}
		}

	}
 	return count;
}

void CConstraintGraph::Init( )
{
		m_ModuleID2NodeID.clear();
		//m_ModuleID.clear();
		m_nodes.clear();
		m_CriticalEdge.clear();
}

void CConstraintGraph::ReInit( )
{
	for(unsigned int i = 0 ; i < m_nodes.size();i++){
		m_nodes[i].m_edge_moduleID.clear();
		m_nodes[i].m_outedge.clear();
		m_nodes[i].m_inedge.clear();
	}
	vector<CCGraphNode>::iterator ite;
	ite = m_nodes.end();
	ite--;
	m_nodes.erase(ite--);
	m_nodes.erase(ite--);

	m_CriticalEdge.clear();
	
}

void CConstraintGraph::RemoveDummyEdge( )
{
	for(unsigned int i = 0 ; i < m_nodes.size(); i++){
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_outedge.begin(); ite !=m_nodes[i].m_outedge.end();++ite ){
			if (ite->m_priority > m_pri_thres ){
				m_nodes[i].m_outedge.erase(ite--);
			}
		}
		for (list<CCGraphEdge>::iterator ite = m_nodes[i].m_inedge.begin(); ite !=m_nodes[i].m_inedge.end();++ite ){
			if (ite->m_priority > m_pri_thres ){
				m_nodes[i].m_inedge.erase(ite--);
			}
		}
	}
}




int CConstraintGraph::GetANSI()
{
	double ratio = -(m_most_neg_slack / (m_upper_bound- m_lower_bound));
	if (ratio >= 0.2)
		return 41;
	else if (ratio >= 0.15)
		return 45;
	else if (ratio >= 0.1)
		return 44;
	else if (ratio >= 0.05)
		return 46;
	else if (ratio > 0)
		return 42;
	else
		return 47;
}

void CMacroLegal::AutoRowHeight(){
	vector<int> RHStep;
	vector<int> RHfStep;
	int max_height = static_cast<int>((m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom)/m_pDB->m_sites[0].m_height );
	for (int i = 0 ; i <= max_height ; i++){
		RHStep.push_back(0);
		RHfStep.push_back(0);
	}
	
	if ((int)m_MacroRowHeight != 1)
		return;
	int _rowheight;
	for(unsigned int i = 0; i < m_pDB->m_modules.size() ; i++ ){
		//if((!m_pDB->m_modules[i].m_isOutCore) ){
		if((!m_pDB->BlockOutCore(i)) ){
			_rowheight = static_cast<int>(ceil(m_pDB->m_modules[i].GetHeight() /m_pDB->m_sites[0].m_height)); 
			 if (m_pDB->m_modules[i].m_isFixed)
				RHfStep[_rowheight]++;
			 else
				RHStep[_rowheight]++;
		}	

	}
	int _sum = 0;
	int _fsum = 0;
	for (unsigned int i =  RHStep.size()-1; i >= 0;i--){
		if (_sum < m_MaxNumMacro ){
			_sum  += RHStep[i];
			_fsum  += RHfStep[i];
			
			//cout << i << ":" << RHStep[i] << "  sum:" << _sum << endl;
			

		}else{
			m_MacroRowHeight = i+2;
			//cout << m_MacroRowHeight << endl;
			break;
			
		}


	}

	
	
}

void CMacroLegal::ReloadModulePosition( )
{

	set<int>::iterator ite;

	for(ite = m_ModuleID_set.begin(); ite != m_ModuleID_set.end(); ++ite){
		CCGraphNode& H_node = HCG.m_nodes[HCG.m_ModuleID2NodeID[*ite]];
		H_node.m_value = m_pDB->m_modules[*ite].m_x;
		H_node.m_low = m_pDB->m_modules[*ite].m_y;
		H_node.m_high =H_node.m_low + m_pDB->m_modules[*ite].m_height;

//  		H_node.m_cx = H_node.m_value + 0.5 * H_node.m_length;
//  		H_node.m_cy = (H_node.m_high + H_node.m_low) /2;

		CCGraphNode& V_node = VCG.m_nodes[VCG.m_ModuleID2NodeID[*ite]];
		V_node.m_value = m_pDB->m_modules[*ite].m_y;
		V_node.m_low = m_pDB->m_modules[*ite].m_x;
		V_node.m_high =V_node.m_low + m_pDB->m_modules[*ite].m_width;

//  		V_node.m_cx = V_node.m_value + 0.5 * V_node.m_length;
//  		V_node.m_cy = (V_node.m_high + V_node.m_low) /2;


	}
	HCG.SortNodes();
	VCG.SortNodes();

}
void CCGraphNode::ShowNode( )
{
	printf("Node %f - %f C:(%f,%f) LH(%f,%f) \n",m_value,m_length,m_cx,m_cy,m_low,m_high);
}

void CConstraintGraph::nCGConstruct( )
{
	bool _anymovable;
	double _x_displacement;
	double _y_displacement;
	for (unsigned int i = 0 ; i < m_nodes.size() ; i++ ){
		//printf("Processing : %d\n",m_nodes[i].m_moduleID);
		for (unsigned int j = i+1; j < m_nodes.size() ; j++ ){

				_anymovable = ( (!(m_nodes[i].m_isFixed)) || (!(m_nodes[j].m_isFixed)));
				_x_displacement = m_nodes[i].m_value + m_nodes[i].m_length - m_nodes[j].m_value;
				_y_displacement = Y_Displacement(m_nodes[i],m_nodes[j]);
		//		if(m_nodes[i].m_moduleID ==  451418&& m_nodes[j].m_moduleID == 450994){
		//			printf("X%f\tY:%f\n",_x_displacement,_y_displacement);
		//			m_nodes[i].ShowNode();
		//			m_nodes[j].ShowNode();
		//		}
				if (_anymovable){
					
					if(NodeOverlap(m_nodes[i],m_nodes[j])){	// two macro overlap!
						if (_x_displacement < _y_displacement)
							AddEdge(m_nodes[i],m_nodes[j],0.0);
					}else{			//no overlap
						if (_x_displacement < _y_displacement)
							AddEdge(m_nodes[i],m_nodes[j],(-_x_displacement)/(m_nodes[i].m_length + m_nodes[j].m_length));
					}
				}
		}
	}

}

double CConstraintGraph::Y_Displacement( CCGraphNode & n1, CCGraphNode & n2 )
{
	double _dsp;
	_dsp = (n1.m_high- n1.m_low + n2.m_high - n2.m_low) - (max(n1.m_high,n2.m_high) - min(n1.m_low,n2.m_low));
	return _dsp;
	
}



void CMacroLegal::OffsetRelocate( )
{
	if (isOffset) return;
	for(unsigned int i = 0 ; i < m_pDB->m_modules.size() ; i++){
		m_pDB->m_modules[i].m_x -= m_offset_X;
		m_pDB->m_modules[i].m_cx -= m_offset_X;
		m_pDB->m_modules[i].m_y -= m_offset_Y;
		m_pDB->m_modules[i].m_cy -= m_offset_Y;
		

	}
	m_pDB->m_coreRgn.left -=  m_offset_X;
	m_pDB->m_coreRgn.right -=  m_offset_X;
	m_pDB->m_coreRgn.bottom -=  m_offset_Y;
	m_pDB->m_coreRgn.top -=  m_offset_Y;
	isOffset = true;
}

void CMacroLegal::OffsetRestore( )
{

	for(unsigned int i = 0 ; i < m_pDB->m_modules.size() ; i++){
		m_pDB->m_modules[i].m_x += m_offset_X;
		m_pDB->m_modules[i].m_cx += m_offset_X;
		m_pDB->m_modules[i].m_y += m_offset_Y;
		m_pDB->m_modules[i].m_cy += m_offset_Y;
		

	}
	m_pDB->m_coreRgn.left +=  m_offset_X;
	m_pDB->m_coreRgn.right +=  m_offset_X;
	m_pDB->m_coreRgn.bottom +=  m_offset_Y;
	m_pDB->m_coreRgn.top +=  m_offset_Y;

}



