#!/bin/bash

git grep -l "typedef" | grep -v "fixes.sh" | while read f ; do perl -pi -e 's#^.*?typedef\s*[^ ].*?\bINT\b##' $f ; done

git grep -l "\<INT\>" | grep -v "fixes.sh" | while read f; do perl -pi -e 's#(([^_A-Z]|^))INT([^_A-Z])#\1int\3#g' $f ; done



