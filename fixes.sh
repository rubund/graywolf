#!/bin/bash

#git grep -l "\<int\>" | while read f; do perl -pi -e 's#\bINT\b#int#' $f ; done


git grep -l "typedef" | grep -v "fixes.sh" | while read f ; do perl -pi -e 's#^.*?typedef.*?int.*?int.*$##' $f ; done

