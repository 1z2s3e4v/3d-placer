
set title " ./run_tmp/case1/shrunked-2d-place.gp.plt, block= 8, net= 6, HPWL= 56 " font "Times, 22"

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

9.865, 9.807
16.865, 9.807
16.865, 16.807
9.865, 16.807
9.865, 9.807

15.465, 13.307
15.465, 16.807
14.765, 16.107
15.465, 16.807
16.165, 16.107

2.739, 6.828
18.739, 6.828
18.739, 13.828
2.739, 13.828
2.739, 6.828

15.539, 10.328
15.539, 13.828
13.939, 13.128
15.539, 13.828
17.139, 13.128

2.927, 13.117
18.927, 13.117
18.927, 20.117
2.927, 20.117
2.927, 13.117

15.727, 16.617
15.727, 20.117
14.127, 19.417
15.727, 20.117
17.327, 19.417

7.649, 8.316
20.649, 8.316
20.649, 15.316
7.649, 15.316
7.649, 8.316

18.049, 11.816
18.049, 15.316
16.749, 14.616
18.049, 15.316
19.349, 14.616

10.729, 11.191
23.729, 11.191
23.729, 18.191
10.729, 18.191
10.729, 11.191

21.129, 14.691
21.129, 18.191
19.829, 17.491
21.129, 18.191
22.429, 17.491

9.624, 13.707
25.624, 13.707
25.624, 20.707
9.624, 20.707
9.624, 13.707

22.424, 17.207
22.424, 20.707
20.824, 20.007
22.424, 20.707
24.024, 20.007

4.053, 16.401
20.053, 16.401
20.053, 23.401
4.053, 23.401
4.053, 16.401

16.853, 19.901
16.853, 23.401
15.253, 22.701
16.853, 23.401
18.453, 22.701

19.318, 15.372
26.318, 15.372
26.318, 22.372
19.318, 22.372
19.318, 15.372

24.918, 18.872
24.918, 22.372
24.218, 21.672
24.918, 22.372
25.618, 21.672


EOF


# fixed blocks
0, 0


EOF


# cell move
0, 0


EOF


# nets
0, 0

17.365, 18.307
13.739, 13.328

14.739, 14.328
14.927, 20.617

14.739, 14.328
15.053, 22.901

23.739, 14.328
26.818, 23.872

23.927, 20.617
20.624, 20.207

23.927, 20.617
16.053, 23.901

20.149, 14.816
25.229, 18.691

20.149, 14.816
21.624, 21.207

22.149, 15.816
23.229, 17.691


EOF


# dummy modules
0, 0


EOF


# pins
	0, 0


EOF

pause -1 'Press any key'