#!/bin/bash

###############################################
###### IRIS-Processor #########################
###############################################

# This script processes an iris-database the following way
# Input Image 	=> Segmentation result (<filename>.png)
#		=> Iris code	       (<filename>.png)

TARGET=results/iitd
SRC=$TARGET/itex

TARGET_IC=$TARGET/ic
TOOLDIR=bin
TOOLS='lg qsw ko cr cb dct'

IC_EXT=png


 # Clean up
rm -rf $TARGET_IC


# Segmentations that are available
SEGMENTS=`ls -l --time-style="long-iso" $SRC | egrep '^d' | awk '{print $8}'`

echo $SRC
echo $SEGMENTS

for SEGMENT in $SEGMENTS
do
       

  for TOOL in $TOOLS
  do
    mkdir -p $TARGET_IC/$SEGMENT/$TOOL

    FILELIST=`ls $SRC/$SEGMENT`

    # echo $FILELIST


    for INPUTFILE in $FILELIST
    do

      # Retrieve the Filename without the suffix... FIXME: Hardcoded suffixes!!

      extension="${INPUTFILE##*.}"
      y=${INPUTFILE%.$extension}
      FNAME=`echo ${y##*/}`

      echo $TOOL ': Processing ' $SEGMENT/$FNAME
      INPUT=$SRC/$INPUTFILE

      $TOOLDIR/$TOOL -i $SRC/$SEGMENT/$INPUTFILE -o $TARGET_IC/$SEGMENT/$TOOL/$FNAME.$IC_EXT -q

    done
   done
 done
