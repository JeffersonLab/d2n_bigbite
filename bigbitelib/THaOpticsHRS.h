//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsHRS
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Feb 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOpticsHRS
#define ROOT_THaOpticsHRS

#include "THaOptics.h"

class THaTrack;
class THaSpectrometer;
class TVector3;
class TString;
class THaBeam;

//------------------------------------------------------//
//
//    Debug Definitions
//    place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//    DEBUG_LEVEL;    
//    =0    or not define: no debug, full speed 
//    >=1    enable debug extra warning (suggested setting)
//    >=2    above + enable debug assert
//    >=3    above + enable debug extra info
//    >=4    above + massive info (in a for or while)
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//


class THaOpticsHRS : public THaOptics 
{
public:
    THaOpticsHRS(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOpticsHRS();

    //apply optics : do the job
    virtual Int_t ApplyOptics(
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
        );

protected:
    virtual Int_t   ReadDatabase( const TDatime& date );

protected:
    // declarations for target vertex reconstruction
    enum EFPMatrixElemTags { T000 = 0, Y000, P000 };
    enum { kPORDER = 7 };

    // private class for storing matrix element data
    class THaMatrixElement;
    friend class THaMatrixElement;
    class THaMatrixElement {
    public:
        THaMatrixElement() : iszero(true), pw(3), order(0), v(0), poly(kPORDER) {}
        bool match( const THaMatrixElement& rhs ) const;

        bool iszero;             // whether the element is zero
        std::vector<int> pw;     // exponents of matrix element
        //   e.g. D100 = { 1, 0, 0 }
        int  order;
        double v;                // its computed value
        std::vector<double> poly;// the associated polynomial
    };

    // initial matrix elements
    std::vector<THaMatrixElement> fTMatrixElems;
    std::vector<THaMatrixElement> fDMatrixElems;
    std::vector<THaMatrixElement> fPMatrixElems;
    std::vector<THaMatrixElement> fPTAMatrixElems;  // involves abs(theta_fp)
    std::vector<THaMatrixElement> fYMatrixElems;
    std::vector<THaMatrixElement> fYTAMatrixElems;  // involves abs(theta_fp)
    std::vector<THaMatrixElement> fFPMatrixElems;   // matrix elements used in
    // focal plane transformations
    // { T, Y, P }

    std::vector<THaMatrixElement> fLMatrixElems;   // Path-length corrections (meters)

    void CalcMatrix(const double x, std::vector<THaMatrixElement> &matrix);
    Double_t DoPoly(const int n, const std::vector<double> &a, const double x);
    Double_t PolyInv(const double x1, const double x2, const double xacc, 
        const double y, const int norder, 
        const std::vector<double> &a);
    Double_t CalcTargetVar(const std::vector<THaMatrixElement> &matrix, 
        const double powers[][5]);
    Double_t CalcTarget2FPLen(const std::vector<THaMatrixElement>& matrix,
        const Double_t powers[][5]);

private:
    ClassDef(THaOpticsHRS,1);//optics class with HRS matrix expansion algorithm
};




#endif

