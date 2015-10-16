#!/bin/tcsh

## NOTE:  Update this file, then delete/comment the 'echo' lines
echo "THIS IS ONLY AN EXAMPLE.  IT WILL NOT WORK FOR YOU."
echo "Edit the file and change D2NDISK (at least) to something valid"

setenv D2NDISK /data1/rootfiles-brad/d2n

setenv DB_DIR $D2NDISK/analysis/DB
setenv ROOTSYS $D2NDISK/root
setenv ANALYZER $D2NDISK/analyzer

setenv PATH ${ROOTSYS}/bin:${ANALYZER}:${PATH}
setenv LD_LIBRARY_PATH $ANALYZER/:$ROOTSYS/lib/
