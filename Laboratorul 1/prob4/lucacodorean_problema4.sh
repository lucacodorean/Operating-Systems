#! /bin/bash

FILES=`find . -name "*.c"`;
for CURRENT_FILE in $FILES
do
    CURRENT_FILE_LINES_COUNT=`wc -l $CURRENT_FILE | cut -d ' ' -f 1`;
    if test $CURRENT_FILE_LINES_COUNT -gt 20
    then
        echo "Fisier .c cu mai mult de 20 de linii: $CURRENT_FILE.";
    fi
done
