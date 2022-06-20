#include "heap.h"
//#include "netlist.h"
#include <vector>
#include <iostream>
#include <climits>

using namespace std;

//extern vector<Node> netlist;

inline unsigned int parent(unsigned int i)
{
	return (i - 1) / 2;
}

inline unsigned int left(unsigned int i)
{
	return i * 2 + 1;
}

inline unsigned int right(unsigned int i)
{
	return i * 2 + 2;
}

inline void swapItem(HeapItem& a, HeapItem& b)
{
	HeapItem tmp = a;
	a = b;
	b = tmp;
}

void maxHeapify(Heap& A, unsigned int i)
{
	while(1) {
		unsigned int l = left(i);	// left child
		unsigned int r = right(i);	// right child
		unsigned int maxItem = i;	// max item of left and right
		
		if(l < A.size()) {
			if(A[l].s > A[i].s)
				maxItem = l;
		}
		if(r < A.size()) {
			if(A[r].s > A[maxItem].s)
				maxItem = r;
		}
		if(maxItem != i) {
			// exchange indices in heap
		/*	Node& nodeA = netlist[A[i].nodeId];
			Node& nodeMax = netlist[A[maxItem].nodeId];
			nodeA.heapIndex = maxItem;
			nodeMax.heapIndex = i;
		*/	// exchange items in heap
			swapItem(A[i], A[maxItem]);
			i = maxItem;
		}
		else
			break;
	}
}

void buildMaxHeap(Heap& A)
{
	if(A.size() == 1)	return;	// nothing to do
	
	for(int i=parent(A.size()-1); i>=0; i--)
		maxHeapify(A, i);
}

HeapItem heapExtractMax(Heap& A)
{
	HeapItem maxItem = A[0];
	
	// eliminate the index in heap
//	Node& node = netlist[A[0].nodeId];
//	node.heapIndex = INT_MAX;
	// delete the item in heap
	A[0] = A[A.size()-1];
	A.pop_back();
	maxHeapify(A, 0);
	
	return maxItem;
}

void heapChangeS(Heap& A, unsigned int i, double newS)
{
	if(newS < A[i].s) {
		A[i].s = newS;
		maxHeapify(A, i);
	}
	else if(newS > A[i].s) {
		A[i].s = newS;
		while(i > 0) {
			unsigned int p = parent(i);	// parent
			if(A[i].s > A[p].s) {
				// exchange indices in heap
			/*	Node& nodeA = netlist[A[i].nodeId];
				Node& nodeP = netlist[A[p].nodeId];
				nodeA.heapIndex = p;
				nodeP.heapIndex = i;
			*/	// exchange items in heap
				swapItem(A[i], A[p]);
				i = p;
			}
			else
				break;
		}
	}
}
/*
void dumpHeap(Heap& A)
{
	while(A.size() > 0) {
		HeapItem maxItem = heapExtractMax(A);
		cout << maxItem.nodeId << ' ' << maxItem.s << endl;
	}
	cout << endl;
}

int main()
{
	HeapItem item;
	Heap A;
		
	item.nodeId = 1;	item.s = 4;
	A.push_back(item);
	item.nodeId = 2;	item.s = 1;
	A.push_back(item);
	item.nodeId = 3;	item.s = 3;
	A.push_back(item);
	item.nodeId = 4;	item.s = 2;
	A.push_back(item);
	item.nodeId = 5;	item.s = 16;
	A.push_back(item);
	item.nodeId = 6;	item.s = 9;
	A.push_back(item);
	item.nodeId = 7;	item.s = 10;
	A.push_back(item);
	item.nodeId = 8;	item.s = 14;
	A.push_back(item);
	item.nodeId = 9;	item.s = 8;
	A.push_back(item);
	item.nodeId = 10;	item.s = 7;
	A.push_back(item);

	buildMaxHeap(A);

	heapChangeS(A, 3, 15);
	for(unsigned int i=0; i<A.size(); i++)
		cout << A[i].s << ' ' ;
	cout << endl;
	
	while(A.size() > 0) {
		HeapItem maxItem = heapExtractMax(A);
		cout << maxItem.s << ' ';
	}
	cout << endl;

	return 0;
}
*/
