// CGRIP - Copyright 2011 University of Wisconsin-Madison.  All Rights Reserved.
// $HeadURL: http://homepages.cae.wisc.edu/~hamid/cgrip/cgrip.h $
// $Id: CGRIP.h 152 2011-01-19 02:05:45Z shojaei $


/*! \file CGRIP.h
    \brief Header for the CGRIP API.
    
    Details.
*/

#ifndef CGRIP_H
#define CGRIP_H

/*! \fn int CGP_readBenchmark(const char *fileName)
    \brief Read in the benchmark files and initialize the router. Note, you will not be running the router yet. 

    \param inAuxFile The benchmark name to read in.
    \returns 1 if successful, 0 otherswise(e.g. the input file is not valid) 
*/
int CGP_readBenchmark(const char *inAuxFile);

/*! \fn int CGP_readPlacement(const char *inPlFile)
    \brief Read in the placement file and initialize the coordinates. 
		   The first line should always be "UCLA pl 1.0".
		   After the first line, any line starting with a "#" is a comment and is ignored.
		   This is followed by the coordinates of the nodes
	
    \param inPlFile he placement solution file to be read in.
    \returns 1 if successful, 0 otherswise(e.g. the input file is not valid) 
*/
int CGP_readPlacement(const char *inPlFile);

/*! \fn int CGP_setNodeCoordinate(const char *nodeName, const int llx, const int lly)
    \brief Set the coordinates of a moveable object.

    \param nodeName The name of the object to set the coordinates.
    \param llx The lower left x coordinate.
    \param lly The lower left y coordinate.
    \returns 1 if successful, 0 otherswise(e.g. the input name or coordinates are not valid) 
*/
int CGP_setNodeCoordinate(const char *nodeName, const int llx, const int lly);

/*! \fn CGP_runRouting(const int initialTime, const int totalRunTime, const int resolution)
    \brief Run routing.

    \param initialTime. The time limit for the initial routing(FGR) in second.	Default is 300.
    \param totalRunTime. The total time limit for the cgrip routing in second.	Default is 900.
    \param resolution. The resolution for routing(>=1). If you consider resolution equal to 1, the original routing will be done.
    \returns 1 if successful, 0 otherswise(e.g. initialization is not correct.) 
*/
int CGP_runRouting(const int initialTime, const int totalRunTime, const int resolution);

/*! \fn int CGP_getTotalOverflow(void)
    \brief get the total overflow.

    \returns total overflow if successful, -1 otherswise(e.g. routing is not yet done.) 
*/
int CGP_getTotalOverflow(void);

/*! \fn int CGP_getMaxOverflow(void)
    \brief get the total overflow.

    \returns the maximum overflow if successful, -1 otherswise(e.g. routing is not yet done.) 
*/
int CGP_getMaxOverflow(void);

/*! \fn int CGP_getWirelength(void)
    \brief get the total wire length.

    \returns the wire length if successful, -1 otherswise(e.g. routing is not yet done.) 
*/
int CGP_getWirelength(void);

/*! \fn int* CGP_getLayerNCongestion(const int layer, int* n)
    \brief get the tile congestion for a specific layer.
		
		\param layer The layer we want to have congestion for.
		\param n Number of tiles on this layer.
		\returns a vector of "total congestion" for each tile / g-cell in the routing grid. 
		
		for a given layer and tile edge, define: 
		
		max_capacity    = max possible capacity of the tile edge 
		
		actual_capacity = actual capacity of the tile edge 
		
		blockage_demand = max_capacity - actual_capacity 
		
		wire_demand = (number_of_wires)*(wire_width + wire_spacing) 

		edge_overflow = 100*(blockage_demand + wire_demand) / max_capacity 

 		For M1, since it has the same wire width and spacing as M2, we consider the max_capacity to be equal to the max_capacity of M2. 
 		
		Here is how total overflow for a tile is defined: 
		
		For a horizontal layer: 
		
		  Max(left_edge_overflow, right_edge_overflow) 
		  
		For a vertical layer: 
		
		  Max(top_edge_overflow, bottom_edge_overflow) 
*/
int* CGP_getLayerNCongestion(const int layer, int* n);

/*! \fn int CGP_writeOutput(const char *outRouteFile)
    \brief Write the routing output in the ISPD 08 format.

    \param outRouteFile The file name to write in.
    \returns 1 if successful, 0 otherswise(e.g. the output file is not valid) 
*/
int CGP_writeOutput(const char *outRouteFile);

/*! \fn int CGP_disposeCGRIP(void)
    \brief Release the memory occupied by CGRIP. 
    You should call it when routing is done and the information about metrics is not needed anymore.

    \returns 1 if successful, 0 otherswise
*/
int CGP_disposeCGRIP(void);

#endif // CGRIP_H