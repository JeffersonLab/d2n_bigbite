                  -----==========================-----
                       d2n Analysis Replay README
                  -----==========================-----

######################################
## Instructions for installation (if necessary)
######################################
First, make sure that you have ROOT installed, and your environment is set
properly.

For the d2n analyzer -- pull the last version from the CVS (follow these
directions):

https://hallaweb.jlab.org/wiki/index.php/Building_the_Analyzer_and_BigBite_libs_from_CVS_%28d2n%29

######################################
## Environment handling
######################################
You need to set your environment properly.  I strongly recommend using the
'root-setup.sh' script provided.  NOTE that you need to edit the script first
so that it points to where you put your analyzer.  Once it's working, you can
copy the root-setup.sh file to your $HOME
  cp root-setup.sh ${HOME}/.bash_root-setup
then add a line to your $HOME/.bashrc like this
  source ${HOME}/.bash_root-setup
so it will be automatically sourced for interactive shells.


You will also want to update the replay/localROOT symlink to point to YOUR data
directory.  CVS can't handle symlinks.  There is a program in replay that will
create some to reasonable directories for d2n.jlab.org.  
For example:
  cd replay
  ./make_symlinks                    # edit/ignore as appropriate
  ln -fs /data1/$USER localROOT      # if needed
The other symlinks (ROOTfiles, etc) run through the localROOT symlink.
Hopefully you only have to change the one link.


######################################
## Raw data file handling
######################################
The ' replay/raw/' symlink points to the global raw_data directory on
d2n.jlab.org.  The replay scripts look in the directories listed in PATHS[] in
def.h.  As a rule, if you need/want to duplicate a file from /cache, then
please put it in /data1/raw/ (AFTER checking that it isn't already there) so it
is available for others.  Note that we have a pretty large amount of disk space
in /data1 so you don't have to be /too/ conservative.  If you've had to run
jcache on something more than one, you may as well put the file in
/data1/raw_data/.  If we run short on space, those raw data files will the
first to go.


######################################
## StartType.pl and fine-grained DB file handling
######################################
You will find in the ~/replay directory a file called 'filetime_d2n' and
'StartType.pl'.  These files will adjust the calibration coefficients for both
the BB and LHRS detectors according to run number.  Before a replay, type
'./StartType.pl <run number>'.  This will set the proper calibration
coefficients for the specified run.  


######################################
## Replay script notes
######################################
There are detector and Physics replays for both the BB and LHRS
(replay_det_XX.C, replay_phys_XX.C).  There is also a coincidence physics
replay for the coincidence running of d2n.  The physics replays work in
principle, but Matt and I will need to do more work on them to make sure they
give us the information we want concerning relevant physics for data quality
check, etc.  The same may be said of the scalar replay scripts.  (Matt and I
will most likely incorporate the scalar data into the respective detector
replays.)

######################################
## Running multiple jobs
######################################
The 'StartType.pl' manages finer-grained DB file handling by copying files to
the working replay directory.  This isn't great since it means you can only run
one replay at a time in a given replay directory.  See below for a better
solution (that needs implementing).

The 'quick fix' is to make clones of the replay directory.  The general procedure
is to do this from the d2n/ directory:
  ## This clones the replay_master directory 4 times.
  % ./clone_replay
  % ./clone_replay
  % ./clone_replay
  % ./clone_replay
  ## Run screen in a 4-fold tile
  % source root-setup.sh  # if needed!
  % screen -c screenrc-4
You can switch between the 4 sub-tiles in screen using F12.  Each is a separate
bash shell. Just 'cd' into each of the replay_<num> dirs, run StartType.pl, and
fire up an analyzer (using 'nice'!).

Note that this copies everything in replay_master.  Don't put huge files in
there.  (That shouldn't be a problem since generated output should be going to
/data1/* anyway.)

The 'screen' program is extremely nice.  Try this run screen in an xterm
(which, by default, just forks into a shell), run 'vim .bashrc', now close the
xterm without quitting your editor.  Open another xterm and run 'scls' to
reconnect to your editor session without breaking a beat.  ('scls' is a wrapper
of mine that I put in /usr/local/bin/).  Note you can do this from any machine
(ie. home, work), anytime.  Running analyzer jobs in screen also means that you
don't have to worry about them getting killed if your network connection drops.


REPLAY NOTES: 
  - d2n.jlab.org is an 8-core machine.  If there's an idle core, use it.
  - PLEASE 'nice' long-running analysis jobs by running the analyzer like this:
     % nice analyzer


FIXME (needs a volunteer!)
  A better approach would be to rewrite StartType.pl to 
    - create a directory called 'runDB/DB_<runno>' (if it doesn't exist),
    - create symlinks to all of the appropriate DB files in that directory,
    - then invoke the analyzer with a local DB_DIR setting by calling
        'exec DB_DIR="$PWD/runDB_<runno>" analyzer'
  You would then start a replay analyzer by running
    ./StartType.pl 2222
  This would give you the usual analyzer prompt, but the DB_DIR would be
  set to what you need for run 2222.  It would simple to modify the exec
  line to pass arguments to the analyzer prompt too, so you could do
    ./StartType.pl 2222 replay_det_BB(2222)
  (among other options).
