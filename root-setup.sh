## NOTE:  Update this file, then delete/comment the 'echo' lines
echo "THIS IS ONLY AN EXAMPLE.  IT WILL NOT WORK FOR YOU."
echo "Edit the file and change 'd2n_root' (at least) to the directory with your analyzer"
echo 

#===============================================================
# for the test analyzer
#===============================================================

#You want "ls $d2n_root" to look something like this:
# brads@d2n 2023% ls -l $d2n_root 
# total 2952
# drwxr-xr-x 14 brads a-phy    4096 Aug 20 14:41 analyzer
# -rw-r--r--  1 brads a-phy 2994462 Dec  5 18:56 analyzer-d2n.tar.bz2
# drwxr-xr-x  9 brads a-phy    4096 Dec  6 16:11 d2n
# drwxr-xr-x  4 brads a-phy    4096 Dec  5 18:30 d2n-replay
# -rwxr-xr-x  1 brads a-phy     605 Feb  3  2010 pull_cvs
# -rw-r--r--  1 brads a-phy    1222 Feb  3  2010 root-setup.sh
#export d2n_root="$HOME/d2n_analysis_copy"
export d2n_root="/usr/local/d2n_analysis"

# Configure ROOT
case `/bin/hostname -f` in
  *.jlab.org )
    echo "Use JLab's ROOT installation..."
    use root
    ;;

  *)
    echo "Use local ROOT installation."
    echo "!!!  Is this really what you want?  !!!"

    export ROOTSYS="/usr"
    export SHLIB_PATH="$ROOTSYS/lib"
    export LD_LIBRARY_PATH="$ROOTSYS/lib:$LD_LIBRARY_PATH"
    export PATH="$ROOTSYS/bin:$PATH"
    ;;
esac

export ANALYZER="${d2n_root}/analyzer"
export ANALYZER_BIN="$ANALYZER"
export ANALYZER_LIB="$ANALYZER"
export PATH="$ANALYZER_BIN:$PATH"
export LD_LIBRARY_PATH="${d2n_root}/analyzer:$ROOTSYS/lib"
#export LD_LIBRARY_PATH="$ANALYZER_LIB:$LD_LIBRARY_PATH"
#we're using the updated DB here! make sure we have $d2n_root here:
export DB_DIR="${d2n_root}/d2n/DB"
export ANALYZER_DB="$DB_DIR"


## Old stuff
#CVER=3.2.3
#ROOTVER=3.10-02

#use root/$ROOTVER
#use gcc/$CVER

#export LD_LIBRARY_PATH="/site/coda/2.5/Linux/lib:$LD_LIBRARY_PATH"
#export PATH="$PATH:/site/coda/2.5/Linux/bin"

