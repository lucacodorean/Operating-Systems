#! /bin/bash

FILES_COUNT=`find . | wc -l`
DIRECTORIES_COUNT=`find . -type d | wc -l`
echo "Exista $FILES_COUNT fisiere, dintre care $DIRECTORIES_COUNT sunt foldere."