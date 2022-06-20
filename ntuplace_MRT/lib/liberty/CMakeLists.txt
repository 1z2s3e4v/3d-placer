ADD_LIBRARY (si2dr_liberty 
attr_lookup.c        liberty_parser.c  PI.c             token.c
group_lookup.c       libhash.c         syntax_checks.c
liberty_front_lex.c  mymalloc.c        syntax_decls.c
)
########################################################
# This library is obtained from liberty_parse-1.9
########################################################

# original compile flag from the package
SET(CMAKE_C_FLAGS "-DYYERROR_VERBOSE -O2")
