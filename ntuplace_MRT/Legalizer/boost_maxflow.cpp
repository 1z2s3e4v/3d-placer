#include "macrolegal.h"
#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>


void CMacroLegal::bFixEdge( CConstraintGraph & _HCG, CConstraintGraph & _VCG,bool escape )
{
 	using namespace boost;
	map<int,int> _mvertex2moduleID;
	map<int,int> _moduleID2mvertex;
	int _edge_count =_HCG.m_CriticalEdge.size() ;
	int _to_moduleID;
	int _from_moduleID;
	int _src = 0;
	int  _sink = 0;
	if(_edge_count == 0)
		return;
	int _vertex_flag=0;
	vector<int> _out_degree;
	vector<int> _in_degree;
	
	_mvertex2moduleID.clear();
	_moduleID2mvertex.clear();
	
	for ( int i = 0 ; i < _edge_count ; i++ ){	// BGL map create
			_to_moduleID = _HCG.m_CriticalEdge[i].m_to_moduleID;
			if(_moduleID2mvertex.find(_to_moduleID) == _moduleID2mvertex.end()){
				_out_degree.push_back(0);
				_in_degree.push_back(1);
				_mvertex2moduleID[_vertex_flag] = _to_moduleID;
				_moduleID2mvertex[_to_moduleID] = _vertex_flag;
				_vertex_flag++;
			}else{
				_in_degree[_moduleID2mvertex[_to_moduleID]] +=1;
			}
			

			_to_moduleID = _HCG.m_CriticalEdge[i].m_from_moduleID;		//WARN: _to_moduleID CHANGED 
			if(_moduleID2mvertex.find(_to_moduleID) == _moduleID2mvertex.end()){
				_out_degree.push_back(1);
				_in_degree.push_back(0);

				_mvertex2moduleID[_vertex_flag] = _to_moduleID;
				_moduleID2mvertex[_to_moduleID] = _vertex_flag;
				_vertex_flag++;
			}else{ 
				_out_degree[_moduleID2mvertex[_to_moduleID]] +=1;
			}
	}

	//seek for source and sink
	assert(_out_degree.size() == _in_degree.size());
	bool _bSRC = false;
	bool _bSINK = false;
	for(unsigned int i = 0; i <_out_degree.size() ; i++ ){

		if (_in_degree[i] ==  0 ){
			if (!_bSRC)
			_src = i;
// 			cout << "SRC:" << _mvertex2moduleID[_src] << endl;
			//assert(!_bSRC);
			_bSRC = true;
		}
		if (_out_degree[i] ==  0 ){
			if (!_bSINK)
			_sink  = i;
// 			cout << "SINK:" << _mvertex2moduleID[_sink] << endl;
			//assert(!_bSINK);
			_bSINK = true;
		}

		
	}
	assert (_bSRC & _bSINK);
	
// 	for (int i  = 0; i < 7 ; i++)
// 		cout << "v->m:" << i << ":" <<_mvertex2moduleID[i]  << endl;
// 	for (int i  = -2; i < 5; i++)	
// 		cout << "m->v:" << i << ":" <<_moduleID2mvertex[i]  << endl;


	
	
	typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
	typedef adjacency_list<listS, vecS, directedS,
	property<vertex_name_t, std::string>,
	property<edge_capacity_t, long,
	property<edge_residual_capacity_t, long,
	property<edge_reverse_t, Traits::edge_descriptor> > > > Graph;
	
	Graph g;
	
	property_map<Graph, edge_capacity_t>::type capacity = get(edge_capacity, g);
	property_map<Graph, edge_reverse_t>::type rev = get(edge_reverse, g);
	property_map<Graph, edge_residual_capacity_t>::type residual_capacity = get(edge_residual_capacity, g);
	
	Traits::vertex_descriptor s, t;
	//construct graph
	std::vector<Traits::vertex_descriptor> verts;
	
	for (long vi = 0; vi < _vertex_flag; ++vi)
         	 verts.push_back(add_vertex(g));
	s = verts[_src];
	t = verts[_sink];
// 	cout << "Source:" << _src << "~" << _mvertex2moduleID[_src] << endl;
// 	cout << "Sink:" << _sink << "~" << _mvertex2moduleID[_sink] << endl;
	int head, tail,cap;
	
	for ( int i = 0 ; i < _edge_count ; i++ ){	// ADD edge to BOOST GRAPH

// 		_HCG.m_CriticalEdge[i].ShowEdge();
		head = _moduleID2mvertex[_HCG.m_CriticalEdge[i].m_to_moduleID];
		tail = _moduleID2mvertex[_HCG.m_CriticalEdge[i].m_from_moduleID];
		cap = _VCG.CalcTransCost(_HCG.m_CriticalEdge[i].m_from_moduleID,_HCG.m_CriticalEdge[i].m_to_moduleID);

//  		cout << tail << "<->" << head << ":" << cap << endl;
		
		Traits::edge_descriptor e1, e2; 
		bool in1, in2;
		tie(e1, in1) = add_edge(verts[tail], verts[head], g);
		tie(e2, in2) = add_edge(verts[head], verts[tail], g);
		if (!in1 || !in2) {
		std::cerr << "unable to add edge (" << head << "," << tail << ")" 
			<< std::endl;
			exit(-1);
		}
		capacity[e1] = cap;
		capacity[e2] = 0;
		rev[e1] = e2;
		rev[e2] = e1;
	
	}
	
	long flow;
	flow = push_relabel_max_flow(g, s, t);
	
// 	std::cout << "c  The total flow:" << std::endl;
// 	std::cout << "s " << flow << std::endl << std::endl;
	
// 	std::cout << "c flow values:" << std::endl;
	graph_traits<Graph>::vertex_iterator u_iter, u_end;
	graph_traits<Graph>::out_edge_iterator ei, e_end;
	
	int _from_nodeID, _to_nodeID;
//	int _max_cost = _VCG.GetMaxTransCost();
//	int _random_edge =-1;
// 	if (flow == _max_cost){
// 		_random_edge = rand() % _HCG.m_CriticalEdge.size();
// 	}else{
	
	for (tie(u_iter, u_end) = vertices(g); u_iter != u_end; ++u_iter){
		for (tie(ei, e_end) = out_edges(*u_iter, g); ei != e_end; ++ei){
			if (capacity[*ei] > 0){
// 				std::cout << "f " << *u_iter << " " << target(*ei, g) << " "
// 						<<  residual_capacity[*ei] << std::endl;
				if (residual_capacity[*ei] == 0){
					_from_moduleID = _mvertex2moduleID[*u_iter];
					_to_moduleID = _mvertex2moduleID[target(*ei, g)];
					_from_nodeID = _VCG.m_ModuleID2NodeID[_from_moduleID];	
					_to_nodeID = _VCG.m_ModuleID2NodeID[_to_moduleID];
					_HCG.RemoveEdge(_from_moduleID,_to_moduleID);
//  	 				cout << "Adding edge...." ;
					if (_from_nodeID <= _to_nodeID){
						_VCG.AddEdge( _VCG.m_nodes[_from_nodeID], _VCG.m_nodes[_to_nodeID],0);
// 						cout <<  _VCG.m_nodes[_from_nodeID].m_moduleID << "->" << 
// 						_VCG.m_nodes[_to_nodeID].m_moduleID  << endl;
				
						}
					else	{
						_VCG.AddEdge( _VCG.m_nodes[_to_nodeID], _VCG.m_nodes[_from_nodeID],0);
// 						cout <<  _VCG.m_nodes[_to_nodeID].m_moduleID << "->" << 
// 						_VCG.m_nodes[_from_nodeID].m_moduleID  << endl;
				
						}

				}	
			}
		}
	}
// 	}
	

}

