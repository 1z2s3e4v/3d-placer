#!/bin/bash
if [ $# -ne 0 ]; then 
	#make
	if [ $1 == 'a' ]; then
		for ((i=1; i<=4; i++)); do
			case_id=$i
			input="testcase/case${case_id}.txt"
			output="output/case${case_id}_output.txt"
			rm $output
			./bin/3d-placer $input $output
			evaluator/evaluator $input $output |& tee evaluator/case${case_id}_eva.log
		done
	else
		while [ $# != 0 ]
		do
			case_id=$1
			input="testcase/case${case_id}.txt"
			output="output/case${case_id}_output.txt"
			rm $output
			gdb -ex=r -args ./bin/3d-placer $input $output
			# ./bin/3d-placer $input $output
			# evaluator/evaluator $input $output |& tee evaluator/case${case_id}_eva.log
			shift
		done
	fi
else 
	echo "Usage1: \"./run.sh <case_id>\" (example: \"./run.sh 1\" or \"./run.sh 1 2 3\")";
	echo "Usage2: \"./run.sh a\" (run all case)";
	exit
fi
