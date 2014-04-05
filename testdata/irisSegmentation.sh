#!/bin/bash

###############################################
###### IRIS-Processor #########################
###############################################

# This script processes an iris-database the following way
# Input Image 	=> Segmentation result (<filename>.png)
#		=> Iris code	       (<filename>.png)

SRC=iitd/data
TARGET=results/iitd
TARGET_SEG=$TARGET/seg
TARGET_ITEX=$TARGET/itex
TOOLDIR=bin
TOOLS='wahet caht'

SEG_EXT=png
ITEX_EXT=png
rm -rf $TARGET_SEG
rm -rf $TARGET_ITEX

echo $TOOLS


for TOOL in $TOOLS
do
# Clean up


  mkdir -p $TARGET_SEG/$TOOL
  mkdir -p $TARGET_ITEX/$TOOL

  FILELIST=`ls $SRC`

  # echo $FILELIST


  for INPUTFILE in $FILELIST
  do

    # Retrieve the Filename without the suffix... FIXME: Hardcoded suffixes!!

    extension="${INPUTFILE##*.}"
    y=${INPUTFILE%.$extension}
    FNAME=`echo ${y##*/}`

    echo $TOOL ': Processing ' $FNAME
    INPUT=$SRC/$INPUTFILE

    $TOOLDIR/$TOOL -i $SRC/$INPUTFILE -bm $TARGET_SEG/$TOOL/$FNAME.$SEG_EXT -o $TARGET_ITEX/$TOOL/$FNAME.$ITEX_EXT -s 512 64 -e -q

  done
 done
