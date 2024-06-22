#! /bin/bash
clear

FILES_IN_DIR=`find .| tail -n +2`;
COMMAND=`echo $1 | cut -c 1`;

for CURRENT_FILE in $FILES_IN_DIR;
do

    TEMP="${CURRENT_FILE##*/}";
    FILE_NAME="${TEMP%.*}";

    $PERMS=`ls -l "$CURRENT_FILE" | cut -d ' ' -f 1`;
    
    if echo -q "$COMMAND" | grep -q -E "$PERMS";
    then 
        FILE_SIZE=$(du -b "$CURRENT_FILE" | cut -d' ' -f 1);
        mkdir "$FILE_NAME.dir"; chmod +rw "$FILE_NAME.dir";
        cd "$FILE_NAME.dir";

        touch "size.txt"; chmod +rw "size.txt";
        echo "$FILE_SIZE"> "size.txt";

        cd ..;
    else echo "No permission.";
    fi

done