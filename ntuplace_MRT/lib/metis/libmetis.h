extern "C" {
	void METIS_PartGraphRecursive (int *n,
		int *xadj,
		int *adjncy,
		int *vwgt,
		int *adjwgt,
		int *wgtflag,
		int *numflag,
		int *nparts,
		int *options,
		int *edgecut,
		int *part);
}
