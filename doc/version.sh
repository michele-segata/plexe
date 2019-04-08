#!/bin/bash

# Generates the version number used for the doxygen documentation
# by using git describe to analyze the current HEAD of the repository.
#
# Example: plexe-5a1-16-gd81b4a14a0
# The current HEAD points at the commit d81b4a14a0, which is 16 commits
# newer than the tag plexe-5a1 it is based on.
#
# In case git describe fails, the version is empty.

TAG=$(git describe --tags --always 2> /dev/null)
if [[ ($? -eq 0) && ($TAG =~ ^plexe-) ]]; then
    echo $TAG | sed -n 's/^plexe-\(.*\)$/\1/p'
else
    echo ""
fi
