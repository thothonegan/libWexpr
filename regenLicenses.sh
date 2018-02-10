#!/bin/bash

HGUILD=$HOME/Hackerguild/Source/HGuild@master/Prefix/hguild

# NOTE : If you change this, also alter LICENSE.txt at the toplevel.
`$HGUILD source:installPath Wolf`/Wolf/Tools/WolfLicenseTool -c replace -y '2017-2018' -o 'Kenneth Perry (thothonegan)' -d `pwd`/

