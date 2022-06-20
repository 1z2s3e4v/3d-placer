#ifndef __HEAP__
#define __HEAP__

#include <vector>

using std::vector;

struct HeapItem {
	unsigned int nodeId;		// node id
	double s;			// sensitivity
};
typedef vector<HeapItem> Heap;

void buildMaxHeap(Heap&);
HeapItem heapExtractMax(Heap&);
void heapChangeS(Heap&, unsigned int, double);
void dumpHeap(Heap&);

#endif
