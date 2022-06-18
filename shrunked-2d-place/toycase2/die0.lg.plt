
set title " ./run_tmp/toycase2/die0.lg.plt, block= 5, net= 4, HPWL= 183 " font "Times, 22"

set size ratio 1
set nokey

plot[:][:] '-' w l lt 3, '-' w l lt 4, '-' w l lt 1, '-' w l lt 7, '-' w l lt 5, '-' w l lt 2


# core region
0.000, 0.000
80.000, 0.000
80.000, 40.000
0.000, 40.000
0.000, 0.000


# die area
0.000, 0.000
0.000, 0.000
0.000, 0.000
0.000, 0.000
0.000, 0.000


# blocks
0, 0

25.000, 8.000
41.000, 8.000
41.000, 16.000
25.000, 16.000
25.000, 8.000

37.800, 12.000
37.800, 16.000
36.200, 15.200
37.800, 16.000
39.400, 15.200

1.000, 32.000
17.000, 32.000
17.000, 40.000
1.000, 40.000
1.000, 32.000

13.800, 36.000
13.800, 40.000
12.200, 39.200
13.800, 40.000
15.400, 39.200

0.000, 0.000
14.000, 0.000
14.000, 8.000
0.000, 8.000
0.000, 0.000

11.200, 4.000
11.200, 8.000
9.800, 7.200
11.200, 8.000
12.600, 7.200


EOF


# fixed blocks
0, 0

17.000, 18.000
17.000, 18.000
17.000, 18.000
17.000, 18.000
17.000, 18.000

17.000, 18.000
17.000, 18.000
17.000, 18.000
17.000, 18.000
17.000, 18.000

17.000, 8.000
17.000, 8.000
17.000, 8.000
17.000, 8.000
17.000, 8.000

17.000, 8.000
17.000, 8.000
17.000, 8.000
17.000, 8.000
17.000, 8.000


EOF


# cell move
0, 0


EOF


# nets
0, 0

37.000, 19.000
26.000, 24.000

37.000, 19.000
24.000, 10.000

22.000, 40.000
16.000, 9.000

22.000, 40.000
21.000, 22.000

22.000, 40.000
21.000, 16.000

40.000, 14.000
16.000, 38.000

46.000, 16.000
13.000, 43.000

46.000, 16.000
11.000, 7.000

46.000, 16.000
30.000, 13.000


EOF


# dummy modules
0, 0


EOF


# pins
	0, 0


EOF

pause -1 'Press any key'