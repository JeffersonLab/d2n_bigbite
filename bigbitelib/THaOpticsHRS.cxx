//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsHRS
//
//  Derived class of THaOptics, which uses matrix expansion algorithm  
//  The core code is copied from JLab Hall A VDC vertex reconstruction
//    
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Feb 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TMath.h"
#include "TList.h"

#include "TROOT.h"
#include "THaString.h"
#include <map>
#include <cstdio>
#include <cstdlib>
using namespace std;


#include "ha_compiledata.h" //include for analyzer version check


//put this header file below all other headers
#include "THaOpticsHRS.h"


THaOpticsHRS::THaOpticsHRS(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName)
:THaOptics(name,desc,pspec,strBeamDetectorName)
// constructor function of THaOpticsHRS
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;


    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;
}
//_____________________________________________________________________________

THaOpticsHRS::~THaOpticsHRS() {}
//_____________________________________________________________________________

Int_t THaOpticsHRS::ReadDatabase(const TDatime& date)
{
    THaOptics::ReadDatabase(date);
    static const char* const here = "ReadDatabase";
    const int LEN = 200;
    char buff[LEN];
    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    DEBUG_INFO(Here(here),"\tDB File open OK" );

    // Look for the section [matrix] in the file
    TString tag("[matrix]");
    TString line, tag2(tag);
    tag.ToLower();

    bool found = false;
    while (!found && fgets (buff, LEN, file) != NULL) {
        char* buf = ::Compress(buff);  //strip blanks
        line = buf;
        delete [] buf;
        if( line.EndsWith("\n") ) line.Chop();
        line.ToLower();
        if ( tag == line ) 
            found = true;
    }
    if( !found ) {
        Error(Here(here), "Database entry %s not found!", tag2.Data() );
        fclose(file);
        return kInitError;
    }


    fTMatrixElems.clear();
    fDMatrixElems.clear();
    fPMatrixElems.clear();
    fPTAMatrixElems.clear();
    fYMatrixElems.clear();
    fYTAMatrixElems.clear();
    fLMatrixElems.clear();

    fFPMatrixElems.clear();
    fFPMatrixElems.resize(3);

#if ANALYZER_VERSION_CODE<=ANALYZER_VERSION(1,5,0)
    typedef vector<THaString>::size_type vsiz_t;
#else    
    typedef vector<string>::size_type vsiz_t;
#endif

    map<string,vsiz_t> power;
    power["t"] = 3;  // transport to focal-plane tensors
    power["y"] = 3;
    power["p"] = 3;
    power["D"] = 3;  // focal-plane to target tensors
    power["T"] = 3;
    power["Y"] = 3;
    power["YTA"] = 4;
    power["P"] = 3;
    power["PTA"] = 4;
    power["L"] = 4;  // pathlength from z=0 (target) to focal plane (meters)
    power["XF"] = 5; // forward: target to focal-plane (I think)
    power["TF"] = 5;
    power["PF"] = 5;
    power["YF"] = 5;

    map<string,vector<THaMatrixElement>*> matrix_map;
    matrix_map["t"] = &fFPMatrixElems;
    matrix_map["y"] = &fFPMatrixElems;
    matrix_map["p"] = &fFPMatrixElems;
    matrix_map["D"] = &fDMatrixElems;
    matrix_map["T"] = &fTMatrixElems;
    matrix_map["Y"] = &fYMatrixElems;
    matrix_map["YTA"] = &fYTAMatrixElems;
    matrix_map["P"] = &fPMatrixElems;
    matrix_map["PTA"] = &fPTAMatrixElems;
    matrix_map["L"] = &fLMatrixElems;

    map <string,int> fp_map;
    fp_map["t"] = 0;
    fp_map["y"] = 1;
    fp_map["p"] = 2;

    // Read in as many of the matrix elements as there are.
    // Read in line-by-line, so as to be able to handle tensors of
    // different orders.
    while( fgets(buff, LEN, file) ) {

#if ANALYZER_VERSION_CODE<=ANALYZER_VERSION(1,5,0)
        THaString line(buff);
#else
        string line(buff);
#endif

        // Erase trailing newline
        if( line.size() > 0 && line[line.size()-1] == '\n' ) {
            buff[line.size()-1] = 0;
            line.erase(line.size()-1,1);
        }
        // Split the line into whitespace-separated fields    
#if ANALYZER_VERSION_CODE<=ANALYZER_VERSION(1,5,0)
        vector<THaString> line_spl = line.Split();
#else
        vector<string> line_spl = THaString::Split(line);
#endif
        // Stop if the line does not start with a string referring to
        // a known type of matrix element. In particular, this will
        // stop on a subsequent timestamp or configuration tag starting with "["
        if(line_spl.empty())
            continue; //ignore empty lines
        const char* w = line_spl[0].c_str();
        vsiz_t npow = power[w];
        if( npow == 0 ) 
            break;

        // Looks like a good line, go parse it.
        THaMatrixElement ME;
        ME.pw.resize(npow);
        ME.iszero = true;  ME.order = 0;
        vsiz_t pos;
        for (pos=1; pos<=npow && pos<line_spl.size(); pos++) {
            ME.pw[pos-1] = atoi(line_spl[pos].c_str());
        }
        vsiz_t p_cnt;
        for ( p_cnt=0; pos<line_spl.size() && p_cnt<kPORDER && pos<=npow+kPORDER;
            pos++,p_cnt++ ) {
                ME.poly[p_cnt] = atof(line_spl[pos].c_str());
                if (ME.poly[p_cnt] != 0.0) {
                    ME.iszero = false;
                    ME.order = p_cnt+1;
                }
        }
        if (p_cnt < 1) {
            Error(Here(here), "Could not read in Matrix Element %s%d%d%d!",
                w, ME.pw[0], ME.pw[1], ME.pw[2]);
            Error(Here(here), "Line looks like: %s",line.c_str());
            fclose(file);
            return kInitError;
        }
        // Don't bother with all-zero matrix elements
        if( ME.iszero )
            continue;

        // Add this matrix element to the appropriate array
        vector<THaMatrixElement> *mat = matrix_map[w];
        if (mat) {
            // Special checks for focal plane matrix elements
            if( mat == &fFPMatrixElems ) {
                if( ME.pw[0] == 0 && ME.pw[1] == 0 && ME.pw[2] == 0 ) {
                    THaMatrixElement& m = (*mat)[fp_map[w]];
                    if( m.order > 0 ) {
                        Warning(Here(here), "Duplicate definition of focal plane "
                            "matrix element: %s. Using first definition.", buff);
                    } else
                        m = ME;
                } else
                    Warning(Here(here), "Bad coefficients of focal plane matrix "
                    "element %s", buff);
            } 
            else {
                // All other matrix elements are just appended to the respective array
                // but ensure that they are defined only once!
                bool match = false;
                for( vector<THaMatrixElement>::iterator it = mat->begin();
                    it != mat->end() && !(match = it->match(ME)); it++ );
                    if( match ) {
                        Warning(Here(here), "Duplicate definition of "
                            "matrix element: %s. Using first definition.", buff);
                    } else
                        mat->push_back(ME);
            }
        }
        else if ( fDebug > 0 )
            Warning(Here(here), "Not storing matrix for: %s !", w);

    } //while(fgets)    

    CalcMatrix(1.,fLMatrixElems); // tensor without explicit polynomial in x_fp

    fclose(file);
    return kOK;
}
//_____________________________________________________________________________

Int_t THaOpticsHRS::ApplyOptics
(
 Double_t trackX,    //input:    Track X             TRCS
 Double_t trackY,    //input:    Track Y             TRCS
 Double_t trackTheta,//input:    Track Theta         TRCS
 Double_t trackPhi,  //input:    Track Phi           TRCS
 TVector3 beamPos,   //input:    Beam Position       HCS
 TVector3 beamDir,   //input:    Beam Direction      HCS
 TVector3 &P,        //output:   vector momentum     HCS
 TVector3 &Vertex,   //output:   vertex cordinate    HCS
 Double_t &TargetX,  //output:   Target X            TCS
 Double_t &TargetY, //output:   Target Y            TCS
 Double_t &TargetTheta,//output: Target Theta        TCS
 Double_t &TargetPhi,//output:   Target Phi          TCS
 Double_t &PathLen   //output:   Length of path      any coordinate
 )
{
    const Int_t kNUM_PRECOMP_POW = 10;

    Double_t x_fp, y_fp, th_fp, ph_fp;
    Double_t powers[kNUM_PRECOMP_POW][5];  // {(x), th, y, ph, abs(th) }
    Double_t x, y, theta, phi, dp, p, pathl;

    x_fp = trackX;
    y_fp = trackY;
    th_fp =trackTheta;
    ph_fp =trackPhi;

    // calculate the powers we need
    for(int i=0; i<kNUM_PRECOMP_POW; i++) {
        powers[i][0] = pow(x_fp, i);
        powers[i][1] = pow(th_fp, i);
        powers[i][2] = pow(y_fp, i);
        powers[i][3] = pow(ph_fp, i);
        powers[i][4] = pow(TMath::Abs(th_fp),i);
    }

    // calculate the matrices we need
    CalcMatrix(x_fp, fDMatrixElems);
    CalcMatrix(x_fp, fTMatrixElems);
    CalcMatrix(x_fp, fYMatrixElems);
    CalcMatrix(x_fp, fYTAMatrixElems);
    CalcMatrix(x_fp, fPMatrixElems);
    CalcMatrix(x_fp, fPTAMatrixElems);

    // calculate the coordinates at the target
    theta = CalcTargetVar(fTMatrixElems, powers);
    phi = CalcTargetVar(fPMatrixElems, powers)+CalcTargetVar(fPTAMatrixElems,powers);
    y = CalcTargetVar(fYMatrixElems, powers)+CalcTargetVar(fYTAMatrixElems,powers);

    // calculate momentum
    dp = CalcTargetVar(fDMatrixElems, powers);
    p  = GetPcentral() * (1.0+dp);

    // pathlength matrix is for the Transport coord plane
    pathl = CalcTarget2FPLen(fLMatrixElems, powers);

    //FIXME: estimate x ??
    x = 0.0;

    // Save the target quantities with the tracks
    TargetX=x;
    TargetY=y;
    TargetTheta=theta;
    TargetPhi=phi;

    TOpticsDir vp(TOpticsDir::kTCS,this,theta,phi,1);
    vp.Unit();
    vp=vp*p;
    vp.TransCoordSys(TOpticsDir::kHCS);
    P=vp.GetVector();

    PathLen=pathl;

    return kOK;
};



//_____________________________________________________________________________
void THaOpticsHRS::CalcMatrix( const Double_t x, vector<THaMatrixElement>& matrix )
{
    // calculates the values of the matrix elements for a given location
    // by evaluating a polynomial in x of order it->order with 
    // coefficients given by it->poly

    for( vector<THaMatrixElement>::iterator it=matrix.begin();
        it!=matrix.end(); it++ ) {
            it->v = 0.0;

            if(it->order > 0) {
                for(int i=it->order-1; i>=1; i--)
                    it->v = x * (it->v + it->poly[i]);
                it->v += it->poly[0];
            }
    }
}

//_____________________________________________________________________________
Double_t THaOpticsHRS::CalcTargetVar(const vector<THaMatrixElement>& matrix,
                                     const Double_t powers[][5])
{
    // calculates the value of a variable at the target
    // the x-dependence is already in the matrix, so only 1-3 (or np) used
    Double_t retval=0.0;
    Double_t v=0;
    for( vector<THaMatrixElement>::const_iterator it=matrix.begin();
        it!=matrix.end(); it++ ) 
        if(it->v != 0.0) {
            v = it->v;
            unsigned int np = it->pw.size(); // generalize for extra matrix elems.
            for (unsigned int i=0; i<np; i++)
                v *= powers[it->pw[i]][i+1];
            retval += v;
            //      retval += it->v * powers[it->pw[0]][1] 
            //	              * powers[it->pw[1]][2]
            //	              * powers[it->pw[2]][3];
        }

        return retval;
}

//_____________________________________________________________________________
Double_t THaOpticsHRS::CalcTarget2FPLen(const vector<THaMatrixElement>& matrix,
                                        const Double_t powers[][5])
{
    // calculates distance from the nominal target position (z=0)
    // to the transport plane

    Double_t retval=0.0;
    for( vector<THaMatrixElement>::const_iterator it=matrix.begin();
        it!=matrix.end(); it++ ) 
        if(it->v != 0.0)
            retval += it->v * powers[it->pw[0]][0]
        * powers[it->pw[1]][1]
        * powers[it->pw[2]][2]
        * powers[it->pw[3]][3];

        return retval;
}


//_____________________________________________________________________________

ClassImp(THaOpticsHRS);


