## NOTE:  Update this file, then delete/comment the 'echo' lines
echo "THIS IS ONLY AN EXAMPLE.  IT WILL NOT WORK FOR YOU."
echo "Edit the file and change 'd2n_root' (at least) to the directory with your analyzer"
echo 

#===============================================================

#You want "ls $d2n_root" to look something like this:
# brads@d2n 2023% ls -l $d2n_root 
# total 2952
# drwxr-xr-x 14 brads a-phy    4096 Aug 20 14:41 d2n_analyzer
# drwxr-xr-x  9 brads a-phy    4096 Dec  6 16:11 d2n
# drwxr-xr-x  4 brads a-phy    4096 Dec  5 18:30 d2n-replay
# -rwxr-xr-x  1 brads a-phy     605 Feb  3  2010 pull_cvs
# -rw-r--r--  1 brads a-phy    1222 Feb  3  2010 root-setup.sh

#export d2n_root="$HOME/d2n_analysis_copy"
setenv d2n_root "/work/halla/e08014/disk1/brads"

# Configure ROOT
switch (`/bin/hostname -f`)
  case *.jlab.org :
    echo "Use JLab's ROOT installation..."
    source /site/12gev_phys/production.csh 1.2
    breaksw

  default:
    echo "Use local ROOT installation."
    echo "!!!  Is this really what you want?  !!!"

    setenv ROOTSYS "/usr"
    setenv SHLIB_PATH "$ROOTSYS/lib"
    setenv LD_LIBRARY_PATH "$ROOTSYS/lib:$LD_LIBRARY_PATH"
    setenv PATH "$ROOTSYS/bin:$PATH"
    breaksw
endsw

setenv ANALYZER "${d2n_root}/d2n_analyzer"
setenv ANALYZER_BIN "$ANALYZER"
setenv ANALYZER_LIB "$ANALYZER"
setenv PATH "${ANALYZER_BIN}:${PATH}"
setenv LD_LIBRARY_PATH "$ANALYZER_LIB:$LD_LIBRARY_PATH"
#we're using the updated DB here! make sure we have $d2n_root here:
setenv DB_DIR "${d2n_root}/d2n/DB"
setenv ANALYZER_DB "$DB_DIR"
