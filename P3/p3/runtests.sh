#!/bin/sh
chmod +x -R tests
# cat tests/test1.sh | ./mysh > outputs/test1.out
# diff data/test1.out outputs/test1.out
# cat tests/test2.sh | ./mysh > outputs/test2.out
# diff data/test2.out outputs/test2.out
# cat tests/test3.sh | ./mysh > outputs/test3.out
# diff data/test3.out outputs/test3.out

ROOT_DIR=$(realpath .)
for i in $( seq 10 ); do
	echo runningtests $i
	OUTPUT="$ROOT_DIR/outputs/test$i.out"
	SCRIPT="$ROOT_DIR/tests/test$i.sh"
	cd data
	"$ROOT_DIR/mysh" $SCRIPT > $OUTPUT 2>&1
	cd $ROOT_DIR
	DIFF=$( diff data/test$i.out $OUTPUT )
	if [ "$DIFF" != "" ]; then
		cat outputs/test$i.out
		echo "tests $i failed:"
		echo "$DIFF"
	else 
		echo sucess 
	fi
	rm -rf $ROOT_DIR/data/testdir/*
done
