//////////////////////////////////////////////////////////////////////////
//                                                                          
// rootlogon.C                                                            
//                                                                          
// Load Lib, paths and possible scripts to the analyzer upon start
//                                                                          
//////////////////////////////////////////////////////////////////////////
//	
// Author : Jin Huang <mailto:jinhuang@jlab.org>    Mar 2008
//
//////////////////////////////////////////////////////////////////////////
{
    printf("\nrootlogon.C: Loading BigBite Library...");
    gSystem->Load("libBigBite.so");
    //gSystem->Load("/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/libg2c.so");

    //Load more libs here, if necessary. 
    //Make sure it's in path of $LD_LIBRARY_PATH

    printf("\nrootlogon.C: Loading BB Normalization Analysis Library...");
    gSystem->Load("libBBNormAna.so");
    //gSystem->Load("libBBNormAna.so");
    
    //add extra directory of header file here. 

    //gSystem->Load("$ROOTSYS/cint/cint/stl/exception");

    //Load online_C.so to use onlineGUI:
//    gROOT->LoadMacro("./onlineGUI/online_C.so");

    char* macros[] =
    {   //list of scripts to load, end with "\0"
      "def.h",
//      "HallA_style.cxx",
//      "AnaShowRun.C",
//       "./onlineGUI/online_C.so",
      "replay_det_L.C",
      "replay_det_BB.C",
	  "replay_farm_BB.C",
	  "replay_farm_L.C",
//      "get_bpm_pedestals.C",
      "replay_phys.C",
      "replay_phys_L.C",
      "replay_phys_BB.C",
      "replay_phys_coin.C",
      "replay_scalar.C",
      "replay_asym.C",
              "\0"
    };

    if (*macros[0]!=0)
        cout << "\nrootlogon.C: Loading macro's:" << endl;
    for(UInt_t i=0; *macros[i]!=0; i++) {
        cout << "\t " << macros[i] << endl;
        gROOT->LoadMacro(macros[i]);
    }

    printf("\nrootlogon.C: Done!\n\n"); 

/*    printf("----------------------------------------------------\n"
	   "To Do Shift Worker Replay & Show Data, just call:\n\n"
	   "  do_all(<your run number>)\n\n"
	   "Please do replay on adaql4 to adaql7.\n"
	   "Please start replay as soon as starting a run.\n"
      	   "----------------------------------------------------\n"
	   "https://hallaweb.jlab.org/wiki/index.php/Transversity/Online_Replay\n"
      	   "----------------------------------------------------\n"
	   "\n");
*/
}

