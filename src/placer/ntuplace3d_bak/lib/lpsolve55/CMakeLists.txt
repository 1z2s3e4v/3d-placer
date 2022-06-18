ADD_LIBRARY (lpsolve55 
colamd.c     lp_Hash.c    lp_mipbb.c     lp_pricePSE.c  lp_SOS.c    myblas.c
commonlib.c  lp_lib.c     lp_MPS.c       lp_report.c    lp_utils.c  yacc_read.c
fortify.c    lp_LUSOL.c   lp_params.c    lp_rlp.c       lp_wlp.c
ini.c        lp_matrix.c  lp_presolve.c  lp_scale.c     lusol.c
lp_crash.c   lp_MDO.c     lp_price.c     lp_simplex.c   mmio.c
)
########################################################
# This library is obtained from lp_solve_5.5.2.0
########################################################

# original compile flag from the package
SET(CMAKE_C_FLAGS "-O3 -DYY_NEVER_INTERACTIVE -DPARSER_LP -DINVERSE_ACTIVE=INVERSE_LUSOL -DRoleIsExternalInvEngine")
