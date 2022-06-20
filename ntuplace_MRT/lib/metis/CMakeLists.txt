ADD_LIBRARY (metis 
balance.c     initpart.c       memory.c       mmatch.c    refine.c
bucketsort.c  kmetis.c         mesh.c         mmd.c       separator.c
ccgraph.c     kvmetis.c        meshpart.c     mpmetis.c   sfm.c
coarsen.c     kwayfm.c         mfm2.c         mrefine2.c  srefine.c
compress.c    kwayrefine.c     mfm.c          mrefine.c   stat.c
debug.c       kwayvolfm.c      mincover.c     mutil.c     subdomains.c
estmem.c      kwayvolrefine.c  minitpart2.c   myqsort.c   timing.c
fm.c          match.c          minitpart.c    ometis.c    util.c
fortran.c     mbalance2.c      mkmetis.c      parmetis.c
frename.c     mbalance.c       mkwayfmh.c     pmetis.c
graph.c       mcoarsen.c       mkwayrefine.c  pqueue.c
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
########################################################
# This library is obtained from metis-4.0
# and libmetis.h was added
########################################################

# original compile flag from the package
SET(CMAKE_C_FLAGS "-O2")
