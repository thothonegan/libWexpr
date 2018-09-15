#!/bin/bash

HGUILD=$HOME/Hackerguild/Source/HGuild@master/Prefix/hguild

ls text-success/* | while read i; do
	base=$(basename "$i")
	binaryName=${base/.wexpr/.bwexpr}
	${HGUILD} source:run libWexpr WexprTool -c binary -i "$i" -o binary-success/${binaryName}
	
done

