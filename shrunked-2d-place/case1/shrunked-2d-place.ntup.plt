
set title " ./run_tmp/case1/shrunked-2d-place.ntup.plt, block= 8, net= 6, HPWL= 94 " font "Times, 22"

set size ratio 1
set nokey

plot[:][:] '-' w l lt 3, '-' w l lt 4, '-' w l lt 1, '-' w l lt 7, '-' w l lt 5, '-' w l lt 2


# core region
0.000, 0.000
30.000, 0.000
30.000, 28.000
0.000, 28.000
0.000, 0.000


# die area
0.000, 0.000
0.000, 0.000
0.000, 0.000
0.000, 0.000
0.000, 0.000


# blocks
0, 0

0.000, 0.000
7.000, 0.000
7.000, 7.000
0.000, 7.000
0.000, 0.000

5.600, 3.500
5.600, 7.000
4.900, 6.300
5.600, 7.000
6.300, 6.300

0.000, 7.000
16.000, 7.000
16.000, 14.000
0.000, 14.000
0.000, 7.000

12.800, 10.500
12.800, 14.000
11.200, 13.300
12.800, 14.000
14.400, 13.300

0.000, 14.000
16.000, 14.000
16.000, 21.000
0.000, 21.000
0.000, 14.000

12.800, 17.500
12.800, 21.000
11.200, 20.300
12.800, 21.000
14.400, 20.300

16.000, 14.000
29.000, 14.000
29.000, 21.000
16.000, 21.000
16.000, 14.000

26.400, 17.500
26.400, 21.000
25.100, 20.300
26.400, 21.000
27.700, 20.300

16.000, 21.000
29.000, 21.000
29.000, 28.000
16.000, 28.000
16.000, 21.000

26.400, 24.500
26.400, 28.000
25.100, 27.300
26.400, 28.000
27.700, 27.300

14.000, 0.000
30.000, 0.000
30.000, 7.000
14.000, 7.000
14.000, 0.000

26.800, 3.500
26.800, 7.000
25.200, 6.300
26.800, 7.000
28.400, 6.300

0.000, 21.000
16.000, 21.000
16.000, 28.000
0.000, 28.000
0.000, 21.000

12.800, 24.500
12.800, 28.000
11.200, 27.300
12.800, 28.000
14.400, 27.300

16.000, 7.000
23.000, 7.000
23.000, 14.000
16.000, 14.000
16.000, 7.000

21.600, 10.500
21.600, 14.000
20.900, 13.300
21.600, 14.000
22.300, 13.300


EOF


# fixed blocks
0, 0


EOF


# cell move
0, 0


EOF


# nets
0, 0

7.500, 8.500
11.000, 13.500

12.000, 14.500
12.000, 21.500

12.000, 14.500
11.000, 27.500

21.000, 14.500
23.500, 15.500

21.000, 21.500
25.000, 6.500

21.000, 21.500
12.000, 28.500

28.500, 20.500
30.500, 28.500

28.500, 20.500
26.000, 7.500

30.500, 21.500
28.500, 27.500


EOF


# dummy modules
0, 0


EOF


# pins
	0, 0


EOF

pause -1 'Press any key'