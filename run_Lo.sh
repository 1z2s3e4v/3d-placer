#!/usr/bin/bash

if [ -f "./bin/3d-placer" ]; then
    rm ./bin/3d-placer
    echo "./bin/3d-placer removed."
fi     
make 
./bin/3d-placer ./testcase/case4.txt ./output/case4_output.txt


