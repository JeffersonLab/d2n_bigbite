#!/usr/bin/perl -w
# Updated: Wed Mar  9 15:05:33 EST 2011  (BDS)

if (scalar(@ARGV) != 1){
    print "Error:  missing run number.\n\n";
    print "Usage:  $0 <runno>\n\n";
    exit 1;
}

my $runno = shift(@ARGV);
my $FTname = "filetime_d2n";
my @PATHLIST = (
    ".",
    $ENV{"DB_DIR"}
);

## Locate file containing runno, timestamps
for my $tmp ( @PATHLIST ) {
    $tmp .= "/$FTname";
    if( -e $tmp ) {
        $FTname = $tmp;
        goto OPENFILE;
    }
}
die " -!-> Can't find '$FTname' in " . join(", ", @PATHLIST) . "\n";

OPENFILE:
open(FT,"<$FTname") or die " -!-> Can't open '$FTname': $!\n";

## Find requested run in file $FTname
my ($ft_run,$ft_date,$ft_time);
while(<FT>) {
    chomp;
    ($ft_run,$ft_date,$ft_time) = split;
    goto FOUNDRUN if( $ft_run == $runno );
}
die " -!-> Can't find run $runno in '$FTname'\n";

FOUNDRUN:
close(FT);
print "  Found run $ft_run    from date $ft_date at $ft_time\n";

## Find nearest existing date-stamp DB directory with date <= run date
## @DB must be sorted in ascending numerical order, be paranoid here
my @DB=`ls $ENV{'DB_DIR'} | egrep '^2009'`;
@DB = sort {$b <=> $a} @DB;  
for($i=0; $i<$#DB; $i++){
    if($ft_date>=$DB[$i]) {
        my $final = $DB[$i];
        chomp($final);
        print "  Link run $runno DB from date $final \n";

        my $DB_PATH="$ENV{'DB_DIR'}/$final";
        adjcer($runno, $DB_PATH);
        adjprl($runno, $DB_PATH);
        adjbbcer($runno, $DB_PATH);
        adjBBps_sh($runno, $DB_PATH);
        adjbb_mwdc($runno, $DB_PATH);
        adjLHRSTrig($runno,$DB_PATH); 
        print "--> Done.\n";
        exit 0;
    }
}
die " -!-> Could not find suitable DB directory in '$ENV{DB_DIR}'\n"; 


sub dolink($$) {
    my $old = shift;
    my $new = shift;

    if( -e $new ) {
        if( -l $new ) {
            unlink $new;
        } else {
            die "ERROR:  '$new' is not a symlink, will not delete it.  Exiting\n"
        }
    }

    symlink $old, $new or die "$0: Failed link\n";
}


#==================================================================================
# Begin LHRS Section
#
# The subroutines listed below create a new 'db_L.det_pkg.dat' file for the
# LHRS gas Cerenkov and the LHRS Pion Rejector.  These new *.dat files 
# contain the gain matching coefficients appropriate for the polarity
# setting and/or calibration of the LHRS for the given run. These files show up in 
# your current working (replay) directory.  
#
# 'db_L.prlL.dat.N' (N=1,...,14) correspond to the 14 different kinematic settings 
# for the LHRS.  There are calibration coefficients for each setting; hence, 
# 14 different files for each layer of the Pion Rejector (L=1,2). 
#    - 05.09.11: There are now 22 kinematic settings.  The new kinematic settings
#      utilize the calibrations for the ORIGINAL 14 settings.  These are 
#      specified where applicable. 
#
#=================================================================================

#==================================================================================
# LHRS Cherenkov
#==================================================================================

sub adjcer{
    my $runno  = shift;
    my $dbpath = shift;

    if ($runno>=1203 && $runno<=1230){
        print "  Coincidence running [NEGATIVE POLARITY] (3He Target) \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    }
    if ($runno>=1231 && $runno<=1257){
        print "  Coincidence running [POSITIVE POLARITY]. \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    } 
    if ($runno>=1258 && $runno<=1273){
        print "  Coincidence running (H2 Target) [POSITIVE POLARITY]. \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    }
    if ($runno>=1274 && $runno<=1294){
        print "  Coincidence running [POSITIVE POLARITY]. \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    }
    if ($runno>=20493 && $runno<=20528){
        print "  Single-arm running [POSITIVE POLARITY]. \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    }
    if ($runno>=20620 && $runno<=20651){
        print "  Single-arm running [POSITIVE POLARITY]. \n";
        print "  Adjusting the LHRS Cerenkov gain-matching coefficients. \n";
        dolink("$dbpath/db_L.cer.dat.1", "db_L.cer.dat");
        return;
    }

    dolink("$dbpath/db_L.cer.dat", "db_L.cer.dat");
} 

#==================================================================================
# LHRS Pion Rejector [Negative Polarity -- no specification here]
#==================================================================================

sub adjprl{
    my $runno  = shift;
    my $dbpath = shift;
    # p = 0.60 GeV, E = 4.73 GeV
    if (($runno>=20675) && ($runno<=20680)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
     }
    if (($runno==20682)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20717) && ($runno<=20720)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
     }
    # p = 0.60 GeV, E = 5.89 GeV
    if (($runno>=20115) && ($runno<=20120)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20144) && ($runno<=20146)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
    }
    if (($runno>=20154) && ($runno<=20174)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
    }
    # p = 0.70 GeV, E = 5.89 GeV; we're using the calibration at p = 0.60
    if (($runno>=20183) && ($runno<=20194)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.70 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    # p = 0.80 GeV, E = 4.73 GeV
    if (($runno>=20412) && ($runno<=20422)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20671) && ($runno<=20673)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
     }
    # p = 0.90, E = 5.89 GeV
    if (($runno==20332)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20340) && ($runno<=20354)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    if (($runno==20356)||($runno==20357)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    # p = 1.12 GeV, E = 4.73 GeV; we're using the calibration at p = 1.13 GeV
    if (($runno==20604) || ($runno==20605)){ 
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
     if (($runno>=20607) && ($runno<=20612)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    if ((($runno>=20657) && ($runno<=20659)) || (($runno>=20661) && ($runno<=20669))){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    # p = 1.13 GeV, E = 5.89 GeV
    if (($runno>=20314) && ($runno<=20328)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.13 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    if (($runno==20330)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.13 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
     # p = 1.19, GeV, E = 4.73 GeV; we're using the calibration at p = 1.20 GeV
    if (($runno>=20701) && ($runno<=20716)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.19 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
     # p = 1.20, GeV, E = 5.89 GeV
    if (($runno>=20122) && ($runno<=20123)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20476) && ($runno<=20481)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    if (($runno==20546)||($runno==20548)||($runno==20549)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20551) && ($runno<=20553)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20559) && ($runno<=20565)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    # p = 1.26 GeV, E = 4.73 GeV; we're using the calibration at p = 1.27 GeV
    if (($runno>=20588) && ($runno<=20603)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.26 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.27 GeV, E = 5.89 GeV
    if (($runno>=20287) && ($runno<=20291)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20297) && ($runno<=20302)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20304) && ($runno<=20312)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 4.73 GeV; we're using the calibration for p = 1.42 GeV 
    if ( (($runno>=20687) && ($runno<=20700)) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 5.89 GeV; we're using the calibration for p = 1.42 GeV 
    if ( (($runno>=20451) && ($runno<=20473)) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 5.89 GeV; we're using the calibration for p = 1.42 GeV 
    if (($runno==20530) || ($runno==20536) || (($runno>=20540) && ($runno<=20545))){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.42, E = 4.73 GeV
    if ((($runno>=20566) && ($runno<=20568)) || (($runno>=20570) && ($runno<=20583))){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    if (($runno==20585)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.42, E = 5.89 GeV
    if (($runno>=20263) && ($runno<=20276)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.9", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.9", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20279) && ($runno<=20285)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.9", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.9", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 4.73 GeV
    if (($runno>=20392) && ($runno<=20400)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.10", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.10", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20402) && ($runno<=20408)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.10", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.10", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 5.89 GeV
    if (($runno>=20432) && ($runno<=20447)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.11", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.11", "db_L.prl2.dat");
        return;
     }
    # p = 1.60, E = 4.73 GeV
    if (($runno>=20373) && ($runno<=20389)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 4.73 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.12", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.12", "db_L.prl2.dat");
        return;
     }
    # p = 1.60, E = 5.89 GeV
    if (($runno==20121)||($runno==20124)||($runno==20125)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20217) && ($runno<=20226)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    if (($runno==20237)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20239) && ($runno<=20246)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    # p = 1.70, E = 5.89 GeV
    if (($runno==20195)||($runno==20197)||($runno==20198)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.70 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.14", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.14", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20205) && ($runno<=20208)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.70 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.14", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.14", "db_L.prl2.dat");
        return;
     }
    if (($runno==20215)||($runno==20216)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.70 GeV, E = 5.89 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.14", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.14", "db_L.prl2.dat");
        return;
     }
    # p = 1.00/1.20 GeV, E = 1.23 GeV (elastics)
    if (($runno>=1203) && ($runno<=1230)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.00/1.20 GeV, E = 1.23 GeV [Elastic runs]\n";
        dolink("$dbpath/db_L.prl1.dat.15", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.15", "db_L.prl2.dat");
        return;
     }
    # p = 0.60 GeV, E = 1.23 GeV; MISCELLANIOUS
    if (($runno>=20129) && ($runno<=20133)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 1.23 GeV \n";
        dolink("$dbpath/db_L.prl1.dat.16", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.16", "db_L.prl2.dat");
        return;
     }

    #====================================================
    # Nitrogen Runs
    #====================================================

    # p = 0.60 GeV, E = 4.73 GeV
    if (($runno==20674)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
     }
    # p = 0.60 GeV, E = 5.89 GeV
    if (($runno>=20067) && ($runno<=20069)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    if (($runno==20079) || (($runno>=20081)&&($runno<=20087))){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
    }
    # p = 0.60 GeV, E = 5.89 GeV
    if (($runno>=20259) && ($runno<=20261)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
    }
    # p = 0.70 GeV, E = 5.89 GeV; we're using the calibration at p = 0.60
    if (($runno>=20256) && ($runno<=20258)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.70 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    # p = 0.80 GeV, E = 4.73 GeV
    if (($runno==20410) || ($runno==20411) || ($runno==20670)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
     }
    # p = 0.90, E = 5.89 GeV
    if (($runno==20331)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    # p = 1.12 GeV, E = 4.73 GeV; we're using the calibration at p = 1.13 GeV
    if (($runno==20606) || ($runno==20661)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    # p = 1.13 GeV, E = 5.89 GeV
    if (($runno==20313)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.13 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
     # p = 1.19, GeV, E = 4.73 GeV; we're using the calibration at p = 1.20 GeV
    if (($runno==20702)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.19 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
     # p = 1.20, GeV, E = 5.89 GeV
    if (($runno==20475) || ($runno==20475)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    # p = 1.26 GeV, E = 4.73 GeV; we're using the calibration at p = 1.27 GeV
    if (($runno==20587)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.26 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.27 GeV, E = 5.89 GeV
    if (($runno==20286)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 4.73 GeV; we're using the calibration for p = 1.42 GeV 
    if (($runno==20685) || ($runno==20686)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 5.89 GeV; we're using the calibration for p = 1.42 GeV 
    if (($runno==20450)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.42, E = 4.73 GeV
    if (($runno==20569)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.42, E = 5.89 GeV
    if (($runno==20255)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.9", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.9", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 4.73 GeV
    if (($runno==20391) || ($runno==20409)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.10", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.10", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 5.89 GeV
    if (($runno==20431)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.11", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.11", "db_L.prl2.dat");
        return;
     }
    # p = 1.60, E = 4.73 GeV
    if (($runno==20365) || (($runno>=20370) && ($runno<=20372)) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.12", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.12", "db_L.prl2.dat");
        return;
     }
    # p = 1.60, E = 5.89 GeV
    if (($runno==20236)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    # p = 1.70, E = 5.89 GeV
    if (($runno==20429)||($runno==20430)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.70 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.14", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.14", "db_L.prl2.dat");
        return;
     }

     #=======================================================
     # Carbon runs 
     #=======================================================

    # p = 0.60 GeV, E = 5.89 GeV
    if ( ($runno>=20070)&&($runno<=20076) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
    }
    if ( ($runno==20077) || ($runno==20078) ||($runno==20078) || (($runno>=20088)&&($runno<=20091)) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
    }
    if ( ($runno==20139) || ($runno==20148) || ($runno==20149) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
    }
    # p = 0.70 GeV, E = 5.89 GeV; we're using the calibration at p = 0.60
    if (($runno==20175) || ($runno==20181)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.70 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    # p = 0.80 GeV, E = 4.73 GeV
    if ($runno==20428){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
    }
    # p = 0.90, E = 5.89 GeV
    if ( (($runno>=20334)&&($runno<=20337)) || ($runno==20355) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    # p = 1.12 GeV, E = 4.73 GeV; we're using the calibration at p = 1.13 GeV
    if (($runno==20610) || ($runno==20611) || ($runno==20652) || ($runno==20658)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
     # p = 1.20, GeV, E = 5.89 GeV
    if (($runno==20550) || ($runno==20554) || ($runno==20556)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.20 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.6", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.6", "db_L.prl2.dat");
        return;
     }
    # p = 1.26 GeV, E = 4.73 GeV; we're using the calibration at p = 1.27 GeV
    if (($runno==20595)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.26 GeV, E = 4.73 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 4.73 GeV; we're using the calibration for p = 1.42 GeV 
    if ($runno==20684){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 4.73 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.34, E = 5.89 GeV; we're using the calibration for p = 1.42 GeV 
    if ((($runno>=20456)&&($runno<=20458)) || ($runno==20531) || ($runno==20532) || ($runno==20538) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.42, E = 5.89 GeV
    if (($runno==20247) || ($runno==20278) || ($runno==20566)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.42 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.9", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.9", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 4.73 GeV
    if (($runno==20391) || ($runno==20409)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 4.73 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.10", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.10", "db_L.prl2.dat");
        return;
     }
    # p = 1.51, E = 5.89 GeV
    if (($runno==20431)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.51 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.11", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.11", "db_L.prl2.dat");
        return;
     }
    # p = 1.60, E = 5.89 GeV
    if ( (($runno>=20229)&&($runno<=20233)) || ($runno==20235) || ($runno==20238) ){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.60 GeV, E = 5.89 GeV [NITROGEN] \n";
        dolink("$dbpath/db_L.prl1.dat.13", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.13", "db_L.prl2.dat");
        return;
     }
    # p = 1.70, E = 5.89 GeV
    if (($runno==20210)||($runno==20211)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.70 GeV, E = 5.89 GeV [CARBON] \n";
        dolink("$dbpath/db_L.prl1.dat.14", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.14", "db_L.prl2.dat");
        return;
     }  

#==================================================================================
# Positive Polarity Runs [runs will be labeled as POSITIVE POLARITY]
#  - Nitrogen runs are INCLUDED in these run ranges! 
#==================================================================================

    # p = 0.60 GeV, E = 4.73 GeV [positive polarity]
    if (($runno>=20512) && ($runno<=20522)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.1", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.1", "db_L.prl2.dat");
        return;
     }
    # p = 0.60 GeV, E = 5.89 GeV [positive polarity]
    if (($runno>=20632) && ($runno<=20638)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5,89 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    if (($runno==20650) || ($runno==20651)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.60 GeV, E = 5,89 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.2", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.2", "db_L.prl2.dat");
        return;
     }
    # p = 0.80 GeV, E = 5.89 GeV [positive polarity]
    if (($runno>=20624) && ($runno<=20631)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20646) && ($runno<=20649)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.80 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.3", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.3", "db_L.prl2.dat");
        return;
     }
    # p = 0.90 GeV, E = 5.89 GeV [positive polarity]
    if (($runno>=20506) && ($runno<=20511)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    if (($runno>=20527) && ($runno<=20528)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 0.90 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.4", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.4", "db_L.prl2.dat");
        return;
     }
    # p = 1.12 GeV, E = 4.73 GeV [positive polarity] 
    if (($runno>=20621) && ($runno<=20623)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     } 
    if (($runno>=20643) && ($runno<=20645)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    # p = 1.13 GeV, E = 4.73 GeV [positive polarity]
    if (($runno>=20498) && ($runno<=20505)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.13 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    if (($runno==20525) || ($runno==20526)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.13 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    # p = 1.27 GeV, E = 4.73 GeV [positive polarity]
    if (($runno>=20493) && ($runno<=20497)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    if (($runno==20523) || ($runno==20524)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 4.73 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    # p = 1.34 GeV, E = 5.89 GeV [positive polarity] 
    if (($runno>=20639) && ($runno<=20642)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.34 GeV, E = 5.89 GeV [POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.8", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.8", "db_L.prl2.dat");
        return;
     }
    # p = 1.00/1.20 GeV, E = 1.23 GeV (elastics)
    if (($runno>=1231) && ($runno<=1294)){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.00/1.20 GeV, E = 1.23 GeV [Elastic runs, POSITIVE POLARITY]\n";
        dolink("$dbpath/db_L.prl1.dat.15", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.15", "db_L.prl2.dat");
        return;
     }
 
     #=======================================================
     # Carbon runs 
     #=======================================================

    # p = 1.12 GeV, E = 4.73 GeV; we're using the calibration at p = 1.13 GeV
    if ($runno==20620){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.12 GeV, E = 4.73 GeV [CARBON, POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.5", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.5", "db_L.prl2.dat");
        return;
     }
    # p = 1.27 GeV, E = 5.89 GeV
    if ($runno==20493){
        print "  Adjusting the LHRS Pion Rejector gain-matching coefficients: \n";
        print "  p = 1.27 GeV, E = 5.89 GeV [CARBON, POSITIVE POLARITY] \n";
        dolink("$dbpath/db_L.prl1.dat.7", "db_L.prl1.dat");
        dolink("$dbpath/db_L.prl2.dat.7", "db_L.prl2.dat");
        return;
     }
    dolink("$dbpath/db_L.prl1.dat", "db_L.prl1.dat");
    dolink("$dbpath/db_L.prl2.dat", "db_L.prl2.dat");


}

#==================================================================================
# Fix LHRS Trigger Issue (for runs < 20140) 
#==================================================================================

sub adjLHRSTrig{

    my $runno = shift;
    my $dbpath = shift;

    my $TrigPath = $ENV{"DB_DIR"};  # db_DL.dat.1 file is in the DB_DIR root

    if(($runno>20000)&&($runno<20140)){
        print "WARNING: Adjusting trigger DB!\n"; 
        dolink("$TrigPath/db_DL.dat.1","db_DL.dat");
        return;
    }

    dolink("$dbpath/db_DL.dat","db_DL.dat");
}

#==================================================================================
# End of LHRS Section
#==================================================================================

#==================================================================================
# Begin BigBite Section
#
#
# 1263-1294 polarity to positive
# 1339-1361 scintilator HV change
# 1397-1403 BB ADC crate change
# 1516-1531 BB pmt 1-8/11-18 split between 1881 and FADC (half adc amp)
# 1812-1828 old BB cer data, 1829->next change new BB cer data
# 2087-2101 polarity positive (cosmics)
# 2199-2214 positive polarity
# 
#==================================================================================

sub adjbb_mwdc{
    my $runno = shift;
    my $dbpath = shift;


    if($runno>=1208 && $runno<=1245){
       print "  Adusting BB mwdc t0s prior to retiming T6 and L1A and BEFORE Threshold Change\n";
       dolink("$dbpath/db_BB.mwdc.dat.0", "db_BB.mwdc.dat");
       return;
    }

    if($runno>=1246 && $runno<=1274){
       print "  Adusting BB mwdc t0s prior to retiming T6 and L1A and AFTER Threshold Change\n";
       dolink("$dbpath/db_BB.mwdc.dat.1", "db_BB.mwdc.dat");
       return;
    }
#270 cal
    if($runno>=1311 && $runno<=1361){
       print "  Adusting BB mwdc t0s prior to retiming T6 and L1A\n";
       dolink("$dbpath/db_BB.mwdc.dat.2", "db_BB.mwdc.dat");
       return;
    }
#270 cal
    if($runno==1402){
       print "  Adusting BB mwdc t0s prior to retiming T6 and L1A\n";
       dolink("$dbpath/db_BB.mwdc.dat.2", "db_BB.mwdc.dat");
       return;
    }
    if($runno>=1362 && $runno<=1397){
       print "  Adusting BB mwdc t0s prior to retiming T6 and L1A\n";
       dolink("$dbpath/db_BB.mwdc.dat.0", "db_BB.mwdc.dat");
       return;
    }
    if($runno>=1398 && $runno<=1401){
       print "  Adusting BB mwdc t0s after retiming T6 and L1A \n";
       dolink("$dbpath/db_BB.mwdc.dat.1", "db_BB.mwdc.dat");
       return;
    }
    if($runno>=1449 && $runno<=1529){
       print "  Adusting BB mwdc t0s after to retiming T6 and L1A\n";
       print " And after shower mod fix \n";
       dolink("$dbpath/db_BB.mwdc.dat.3", "db_BB.mwdc.dat");
       return;
    }

    dolink("$dbpath/db_BB.mwdc.dat", "db_BB.mwdc.dat");
}


sub adjbbcer{
    my $runno  = shift;
    my $dbpath = shift;

    if ($runno>=1010 && $runno<=1052){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1053 && $runno<=1055){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1056 && $runno<=1063){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1064 && $runno<=1086){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1087 && $runno<=1090){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  

    if ($runno>=1091 && $runno<=1093){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1094 && $runno<=1128){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1129 && $runno<=1143){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1144 && $runno<=1152){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno==1153){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1154 && $runno<=1158){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno==1159){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1160 && $runno<=1180){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1181 && $runno<=1188){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno==1189){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }  
    if ($runno==1190){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1191 && $runno<=1197){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1198){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }      
    if ($runno>=1199 && $runno<=1200){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }   
    if ($runno==1201){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }   
    if ($runno>=1202 && $runno<=1207){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }   
    if ($runno>=1208 && $runno<=1230){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }    
    if ($runno>=1231 && $runno<=1243){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }  
    if ($runno>=1244 && $runno<=1252){
        print "  Adjusting the BB Cerenkov FlashADC  \n";
        dolink("$dbpath/db_BB.cer.dat.3", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1253 && $runno<=1273){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    } 
    if ($runno==1274){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }     
    if ($runno>=1275 && $runno<=1294){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1295 && $runno<=1308){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1309 && $runno<=1310){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1311 && $runno<=1314){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1315 && $runno<=1318){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1319 && $runno<=1357){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 for ROC5 code mod \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1358 && $runno<=1361){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 for ROC5 code mod\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1362 && $runno<=1364){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 for ROC5 code mod\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1365 && $runno<=1368){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 for ROC5 code mod\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1369 && $runno<=1388){
        print "  Adjusting the BB Cerenkov Beam on Peds ADC v792 HV#1 for ROC5 code mod\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    } 
    if ($runno>=1389 && $runno<=1397){
        print "  Adjusting the BB Cerenkov Beam off Peds ADC v792 HV#1 for ROC5 code mod\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    } 
    #start 1881
    if ($runno>=1398 && $runno<=1401){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1402 && $runno<=1403){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1404 && $runno<=1409){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1410){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1411 && $runno<=1447){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1448){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1449 && $runno<=1471){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1472 && $runno<=1474){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#2, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    #HV#3 PMT 12 HV changed
    if ($runno>=1474 && $runno<=1476){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0.2", "db_BB.cer.dat");
        return;
    }

    if ($runno>=1477 && $runno<=1490){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1491){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off/low current Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1492 && $runno<=1509){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1510 && $runno<=1515){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds. \n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    #PMT 1-8, 11-18 split 1881/FADC
    if ($runno>=1516 && $runno<=1519){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC. \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1520 && $runno<=1523){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1524 && $runno<=1526){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n ***WARNING PMT 1-2 Missing***. \n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1527 && $runno<=1531){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n ***WARNING PMT 1-2 Missing***. \n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1532 && $runno<=1558){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1559 && $runno<=1563){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1564 && $runno<=1606){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1607 && $runno<=1617){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1618 && $runno<=1629){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1630 && $runno<=1633){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==1634){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam  Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1635 && $runno<=1644){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1645 && $runno<=1646){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1647 && $runno<=1671){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1672){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, low Beam Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1673 && $runno<=1695){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1696){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1697 && $runno<=1698){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1699 && $runno<=1700){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3,Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1701 && $runno<=1708){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1709 && $runno<=1719){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1720 && $runno<=1741){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1742 && $runno<=1743){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1744 && $runno<=1746){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1747 && $runno<=1768){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1769){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1770 && $runno<=1777){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1778){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1779){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1780 && $runno<=1811){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1812 && $runno<=1813){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1814 && $runno<=1815){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1816 && $runno<=1823){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#3, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1824 && $runno<=1842){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1843 && $runno<=1846){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.3", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1847 && $runno<=1851){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1852 && $runno<=1876){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1877){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1878 && $runno<=1880){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1882){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1883 && $runno<=1885){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1886 && $runno<=1893){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1894){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno==1895){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1896 && $runno<=1899){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.3", "db_BB.cer.dat");
        return;
    }
    if ($runno==1900){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1901){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1902 && $runno<=1903){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1904 && $runno<=1905){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1906 && $runno<=1908){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1909){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1910 && $runno<=1911){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1912 && $runno<=1914){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1915 && $runno<=1917){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1918 && $runno<=1921){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1922 && $runno<=1925){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1926 && $runno<=1927){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1928){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==1929){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1930 && $runno<=1931){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1932 && $runno<=1945){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1946 && $runno<=1947){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==1948){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1949 && $runno<=1955){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1956 && $runno<=1957){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1958 && $runno<=1959){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1960 && $runno<=1961){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1962){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1963 && $runno<=1977){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==1978){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1979 && $runno<=1998){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=1999 && $runno<=2000){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2001 && $runno<=2002){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2003 && $runno<=2004){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==2005){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2006 && $runno<=2020){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==2021){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2022 && $runno<=2023){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2024 && $runno<=2045){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 15uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2046 && $runno<=2047){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2048 && $runno<=2052){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 15uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }

    if ($runno>=2053 && $runno<=2054){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno==2055 || $runno==2059){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2056 && $runno<=2061){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }

    if ($runno>=2062 && $runno<=2065){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno==2066){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2067 && $runno<=2081){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno==2082){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno==2083){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2084 && $runno<=2086){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#4, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }

    # HV#5
    if ($runno>=2087 && $runno<=2100){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==2101){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2102 && $runno<=2105){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==2106){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2107 && $runno<=2108){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2109 && $runno<=2110){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno==2111){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2112 && $runno<=2114){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2115 && $runno<=2116){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2117 && $runno<=2120){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno==2121){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2122 && $runno<=2123){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2124 && $runno<=2128){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2129 && $runno<=2131){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2132 && $runno<=2135){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 14 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.2", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2136 && $runno<=2143){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==2144){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2145 && $runno<=2151){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno==2152){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2153 && $runno<=2155){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2156 && $runno<=2162){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2163 && $runno<=2167){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2168 && $runno<=2173){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno==2174 || $runno==2186){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Low Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1.1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2175 && $runno<=2185){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2187 && $runno<=2196){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2197 && $runno<=2198){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2199 && $runno<=2202){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2203 && $runno<=2212){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, 15 uA Beam on Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b1", "db_BB.cer.dat");
        return;
    }
    if ($runno>=2213 && $runno<=2230){
        print "  Adjusting the BB Cerenkov ADC map to 1881 ADC HV#5, Beam off Peds, Split ADC 1881/FADC\n";
        dolink("$dbpath/db_BB.cer.dat.b0", "db_BB.cer.dat");
        return;
    }


    dolink("$dbpath/db_BB.cer.dat", "db_BB.cer.dat");
 }

sub adjBBps_sh{
    my $runno  = shift;
    my $dbpath = shift;

    if ($runno>=1311  &&  $runno<=1447){
        print " 5.89 GeV beam energy. \n";
        print "  Adjusting the BB ps/sh Energy coefficients before shower mod fix. \n";
        dolink("$dbpath/db_BB.ts.ps.dat.01", "db_BB.ts.ps.dat");
        dolink("$dbpath/db_BB.ts.sh.dat.01", "db_BB.ts.sh.dat");
        return;
    }
    if ($runno>=1448  &&  $runno<=1475){
        print " 5.89 GeV beam energy. \n";
        print "  Adjusting BB sh/ps HV after shower mod. fix. \n";
        dolink("$dbpath/db_BB.ts.ps.dat.01.5", "db_BB.ts.ps.dat");
        dolink("$dbpath/db_BB.ts.sh.dat.01.5", "db_BB.ts.sh.dat");
        return;
    }
    if ($runno>=1476  &&  $runno<=1742){
        print " 5.89 GeV beam energy (Target S=0). \n";
        print "  Adjusting the BB ps/sh Energy coefficients. \n";
        dolink("$dbpath/db_BB.ts.ps.dat.02", "db_BB.ts.ps.dat");
        dolink("$dbpath/db_BB.ts.sh.dat.02", "db_BB.ts.sh.dat");
        return;
    }
    if ($runno>=1829  &&  $runno<=2020){
        print " 5.89 GeV beam energy. \n";
        print "  Adjusting the BB ps/sh Energy coefficients. \n";
        dolink("$dbpath/db_BB.ts.ps.dat.03", "db_BB.ts.ps.dat");
        dolink("$dbpath/db_BB.ts.sh.dat.03", "db_BB.ts.sh.dat");
        return;
    }
    if ($runno>=2024  &&  $runno<=2250){
        print " 4.74 GeV beam energy. \n";
        print "  Adjusting the BB ps/sh Energy coefficients. \n";
        dolink("$dbpath/db_BB.ts.ps.dat.00", "db_BB.ts.ps.dat");
        dolink("$dbpath/db_BB.ts.sh.dat.00", "db_BB.ts.sh.dat");
        return;
    }  
    print "Check StartType.pl in adjBBps_sh()\n";
    print "Can't tag run with calibration \n";
    dolink("$dbpath/db_BB.ts.ps.dat.00", "db_BB.ts.ps.dat");
    dolink("$dbpath/db_BB.ts.sh.dat.00", "db_BB.ts.sh.dat");
}

## MODELINES ###
# vim: ts=4 sts=4 shiftwidth=4 expandtab
