#!/bin/bash

git grep -l "\<int\>" | while read f; do perl -pi -e 's#\bINT\b#int#' $f ; done
