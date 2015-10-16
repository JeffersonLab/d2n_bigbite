//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsAnalytical
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Feb 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////
//BEGIN_HTML <!-- 
// materials below is optimized for HTML reading at
// http://www.jlab.org/~jinhuang/BigBiteDoc/THaOpticsAnalytical.html
/* -->
<h2>
What's this?</h2>
<p>
THaOpticsAnalytical is a derived class of THaOptics, which uses a analytical approach
calculating track back to target for BigBite spectrometer. One can mount it to a
THaOptics-supporting spectrometer by following line and fill up database according
to last section.</p>
<pre>Bool_t mr=MountOptics(new THaOpticsAnalytical("opticstest","BigBite Optics",this));
assert(mr);
</pre>
<h2>
Feature of the model</h2>
<p>
Since the solid angle of bigbite is relatively large, it will not be possible to
use a single matrix to describe backward optics calculation for this spectrometer.
This is because for backward matrix, it will be deeply involved with parameters
such as distance between magnetic and target. This dependancy is not simple and
a matrix for one experiment will not be able to be used on the other. Besides, the
angle of spectrometer will also affect the matrix element. The overall error of
matrix methord is equivalent to the spacial resolusion of detector is worth by the
following amount :</p>
<pre>$$\Delta x_{error}=\tan( \theta_{Target} )(\Delta L+\cos(\Theta_0)z_{react})$$</pre>
<p>
To solve these problem, a general, fitting free model is developped.
</p>
<h3>
Assumption</h3>
<p>
This class use a analytical model calculating the whole track back to target given
infomation of the track on the detector side. It based on the following assumption:</p>
<ul>
<li>The edge of magnetic field is sharp</li>
<li>In the magnetic region, field is homogeneous and along y axis</li>
<li>the front end of the magnetic field is along a surface perpendicular to central
optics line</li>
<li>The back end of magnetic field is along a surface, which could be described by a
rotation of the front surface for a certain angle (described by fMagTheta) along
y axis. The hight of the rotating axis is described by fMagS</li>
</ul>
<h3>
Advantages over Matrix expansion</h3>
<ul>
<li>No fitting needed. All input parameter is directly related to experiment setup.</li>
<li>do not require condition, such as the particle is close to central optical axis.</li>
</ul>
<p>
thus, this optics class will be suitable for online replay, in which case, usually
optics data is not available.</p>
<h3>
Beyond this model</h3>
<p>
accuracy of this model will not meet the requirement of further offline analysis.
Therefore, the following steps is suggested for improvement of precision:</p>
<ul>
<li>Use otptical run data to build a correctiong table. And make an additional corrections
base one this table in data analysis.</li>
<li>Use otptical run data to fit a series of matrix element, and use THaOpticsHRS reading
in the matrix and performing the optics calculation.</li>
</ul>
<h2>
Algorithm Description</h2>
<h3>
calculate bending radius</h3>
<p>
Base on the assumptions, a particle going through the whole system could be devided
into 3 stages: drift in the target region, helix movement in magnetic feild and
free drift in detector region. Following is a figure of a typical track, projected
to midplane:</p>
<img src="./THaOpticsAnalytical.png" border="1" alt="typical track">
<p>
This also shows the temporary coordinate system used in internal function THaOpticsAnalytical::CalxA
and THaOpticsAnalytical::CalR. These two fucntion use infomation of track parameter
on the detector side and aprox. beam position to calculate position of point A(enter
point), B(exit point) and bending radius R. For the y position, which is perpendicular
to midplane, this model first assume a simple reflectiong model (get FirstVertex)
and then correct it with calculated particle path (and get SecendVertex). Aproximate
beam position is also corrected according to y position.</p>
<h3>
scaling bending radius to momentum</h3>
<p class="MsoPlainText" style="margin: 0cm 0cm 0pt">
<span lang="EN-US">P=q*B*R=K*I*R, in which,<?xml namespace="" ns="urn:schemas-microsoft-com:office:office"
prefix="o" ?><o:p></o:p></span></p>
<ul>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>P is calculated momentum<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>q is charge of the incoming particle<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>B is the magnetic field<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>R is bending radius<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>K is an constant related to the magnetic<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>I is the current in coil<o:p></o:p></span></li>
</ul>
<p class="MsoPlainText" style="margin: 0cm 0cm 0pt">
<span lang="EN-US">
<o:p></o:p>
</span>
</p>
<p class="MsoPlainText" style="margin: 0cm 0cm 0pt">
<span lang="EN-US"><span style="mso-spacerun: yes">&nbsp;</span>there are two method
of scaling bending radius to momentum:<o:p></o:p></span></p>
<ol>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>given current I and constant K<o:p></o:p></span></li>
<li class="MsoPlainText" style="margin: 0cm 0cm 0pt"><span lang="EN-US"><span style="mso-spacerun: yes">
&nbsp;</span>given radius of central momentum<o:p></o:p></span></li>
</ol>
<p class="MsoPlainText" style="margin: 0cm 0cm 0pt">
<span lang="EN-US"><span style="mso-spacerun: yes">&nbsp;</span>this class will try
both of above method. </span><span lang="EN-US"><span style="mso-spacerun: yes">&nbsp;</span>Priority
of method 1 is higher.<o:p></o:p></span></p>
<p class="MsoPlainText" style="margin: 0cm 0cm 0pt">
<span lang="EN-US"><span style="mso-spacerun: yes">&nbsp;a </span>K&lt;=0 will a push
this class to use method 2</span></p>
<h2>
Database</h2>
<p>
This class will acquire following information from database (in additional to general
<tt>THaOptics</tt>). All length units are meter and angle are degrees.</p>
<h4>
From <tt>db_run.dat</tt></h4>
<ul>
<li><tt>&lt;Spectrometer Name&gt;.theta</tt>:Angle of Spectrometer.</li>
<li><tt>&lt;Spectrometer Name&gt;.pcentral</tt>:Central momentum. The final momentum
will be scaled respect to this value</li>
<li><tt>&lt;Spectrometer Name&gt;.colldist</tt>:distance of sieve plane away from target
center. As expressed as "L" in last section.</li>
</ul>
<h4>
From <tt>db_&lt;Spectrometer Name&gt;.&lt;Optics Module Name&gt;.dat</tt></h4>
<ul>
<li><tt>MagneticFieldFrontShift</tt>: distance between sieve plane and front boundary
of magnetic field. Called "fMagDL" in code and picture above.</li>
<li><tt>MagneticFieldTopCrossing</tt>:Called "fMagS" in code and picture above. </li>
<li><tt>MagneticFieldRearSlopeAngle</tt>:Called "fMagTheta" in code and picture above.</li>
<li><tt>CurveRadiusOfCentralMomentum</tt>:For the central momentum, it will curve this
given radius in the magnetic field. If it's an negative value, the code will automatically
generate one according to definition.</li>
<li><tt>K</tt>:onstant K here, as discussed above. units: GeV/A/m/c </li>
</ul>

<!--*/
// -->END_HTML
//





#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TList.h"
#include "TString.h"

using namespace std;
//_____________________________________________________________________________

//put this header file below all other headers
#include "THaOpticsAnalytical.h"
//_____________________________________________________________________________

THaOpticsAnalytical::THaOpticsAnalytical(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName)
:THaOptics(name,desc,pspec,strBeamDetectorName)
// constructor function of THaOpticsAnalytical
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;
}
//_____________________________________________________________________________

THaOpticsAnalytical::~THaOpticsAnalytical() {} 
//_____________________________________________________________________________

Int_t THaOpticsAnalytical::ReadDatabase(const TDatime& date)
{
    THaOptics::ReadDatabase(date);
    static const char* const here = "ReadDatabase";
    //const int LEN = 200;
    //char buff[LEN];
    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    DEBUG_INFO(Here(here),"\tDB File open OK" );

    Double_t MagTheta, constK;

    const TagDef tags[]={
        {"MagneticFieldFrontShift", &fMagDL ,1},
        {"MagneticFieldTopCrossing", &fMagS ,2},
        {"MagneticFieldRearSlopeAngle", &MagTheta,3 },
        {"CurveRadiusOfCentralMomentum", &fRCentral,4},
        {"K", &constK,5},
        { 0 }
    };
    Int_t err =LoadDB( file, date,tags, "" );
    if( err ){
        if (err==4) fRCentral=-1;
        else if (err==5) constK=-1;
        else{
            Error(Here(here), "Required tag %s missing in the "
                "run databse. \nBigBite initialization failed.",
                tags[err-1].name );
            fclose(file);
            return kInitError;
        }
    }
#if DEBUG_LEVEL>=3//start show info
    TString sDebugOutput;
    sDebugOutput=GetName();
    sDebugOutput+=" Database read in successfully with:";
    sDebugOutput+="\n \t MagneticFieldFrontShift\t= ";    sDebugOutput+=fMagDL;
    sDebugOutput+="\n \t MagneticFieldTopCrossing\t= ";    sDebugOutput+=fMagS;
    sDebugOutput+="\n \t MagneticFieldRearSlopeAngle\t= ";    sDebugOutput+=MagTheta;
    sDebugOutput+="\n \t CurveRadiusOfCentralMomentum\t= ";    sDebugOutput+=fRCentral;
    sDebugOutput+="\n \t K\t= ";    sDebugOutput+=constK;
    sDebugOutput+="\n";
    Info(Here(here),sDebugOutput.Data());
#endif    


    fclose(file);

    fMagTheta=MagTheta*TMath::Pi()/180;

    if (constK>0&&GetCurrent()>0)
    {
        Info(Here(here),
            "The momentum is calculated to P= %f(constant K) * %f(Current) * R",
            constK,GetCurrent()
            );
        fRadius2Momentum=constK*GetCurrent();
    }
    else if (GetPcentral()>0)
    {
        Info(Here(here),
            "The momentum is calculated by comparing with central momentum",
            constK,GetCurrent()
            );
        SetRCentral();
        fRadius2Momentum=GetPcentral()/fRCentral;
    }
    else
    {
        Warning(Here(here), 
            "There is no way that I can converting bending radius to momentum. "
            "Optics calculation will be deactived. To active optics, at least one"
            "of the following number in db_run.data should be positive: "
            " <spec name>.current,<spec name>.pcentral");
        fRadius2Momentum=-1;
    }

    return kOK;
}
//_____________________________________________________________________________

Int_t THaOpticsAnalytical::
ApplyOptics(
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
    //reloaded gateway function do the calculation.
    if (fRadius2Momentum<0)  
    {//deactive optics
        TargetX=0;
        TargetY=0;
        TargetTheta=0;
        TargetPhi=0;
        PathLen=0;

        return 0;
    }


    Double_t MagFrontZ=GetCollDist()+fMagDL;
    //z coordinate in TCS of the front surface of mag field

    TOpticsDir BackTrackDirection(TOpticsDir::kTRCS,this,trackTheta,trackPhi,1);
    TOpticsPos BackTrackOrigin(TOpticsDir::kTRCS,this,trackX,trackY,0);
    TOpticsDir BeamPos(TOpticsDir::kHCS,this,beamPos);
    TOpticsPos BeamDir(TOpticsDir::kHCS,this,beamDir);

    BackTrackDirection.TransCoordSys(TOpticsDir::kTCS);
    BackTrackOrigin.TransCoordSys(TOpticsDir::kTCS);
    BeamDir.TransCoordSys(TOpticsDir::kTCS);
    BeamPos.TransCoordSys(TOpticsDir::kTCS);

    DEBUG_LINE_MASSINFO(Info("ApplyOptics","BackTrackDirection.Print();");BackTrackDirection.Print(););
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","BackTrackOrigin.Print();");BackTrackOrigin.Print(););

    //-------------------------------------------------
    // all following calculation if in TCS 
    // model defineded variables are xx zz TanF xA/B yA/B
    //-------------------------------------------------
    DEBUG_MASSINFO("ApplyOptics",
        "fMagS=%f, MagFrontZ=%f, fMagTheta=%f",fMagS,MagFrontZ, fMagTheta);

    //position where back track get out of magnetic field
    TVector3 ExitPoint=CrossingSurfLine3D(
        TVector3(-fMagS,0,MagFrontZ),
        TVector3(-TMath::Tan(fMagTheta),0,1),
        BackTrackOrigin.GetVector(),
        BackTrackDirection.GetVector()
        );
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","ExitPoint.Print();");ExitPoint.Print(););

    Double_t TanF=-BackTrackDirection.X()/BackTrackDirection.Z();

    //Get the first guess vertex position
    TVector3 CentralPlaneCrossing=CrossingSurfLine3D(
        TVector3(-fMagS,0,MagFrontZ),
        TVector3(-TMath::Tan(fMagTheta/2),0,1),
        BackTrackOrigin.GetVector(),
        BackTrackDirection.GetVector()
        );
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","CentralPlaneCrossing.Print();");\
        CentralPlaneCrossing.Print(););

    TVector3 FirstVertex=CrossingSurfLine3D(
        CentralPlaneCrossing,
        TVector3(0,1,-trackPhi),
        BeamPos.GetVector(),
        BeamDir.GetVector()
        );
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","FirstVertex.Print();");\
        FirstVertex.Print(););

    //analytical calculation on the midplane
    Double_t xx=FirstVertex.X()-0;
    Double_t zz=FirstVertex.Z()-MagFrontZ;
    Double_t xB=ExitPoint.X()-0;
    Double_t zB=ExitPoint.Z()-MagFrontZ;

    Double_t xA=CalxA(xB,zB,TanF,xx,zz);

    Double_t R=CalR(xA,xB,zB,TanF,xx,zz);

    Double_t ArcLen2D=R*2*TMath::ATan( TMath::Sqrt((xB-xA)*(xB-xA)+zB*zB)/2/R );

    DEBUG_INFO(Here("ApplyOptics"),
        "R=%f, xA=%f, xB=%f, zB=%f, TanF=%f, xx=%f, zz=%f, ArcLen2D=%f",
        R,xA,xB,zB,TanF,xx,zz,ArcLen2D);

    //entrance point to the magnetic field
    TVector3 EnterPoint=
        TVector3(
        xA,
        ExitPoint.Y()-ArcLen2D*trackPhi,
        MagFrontZ
        );

    //second guess : y corrected vertex
    TVector3 SecondVertex=CrossingSurfLine3D(
        EnterPoint,
        TVector3(0,1,-trackPhi),
        BeamPos.GetVector(),
        BeamDir.GetVector()
        );
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","SecondVertex.Print();");\
        SecondVertex.Print(););

    //path length projection at midplane 
    Double_t PathLen2D=0;

    TVector3 tmpVec;
    tmpVec=EnterPoint-SecondVertex;
    PathLen2D+=tmpVec.Mag();

    PathLen2D+=ArcLen2D;

    tmpVec=BackTrackOrigin.GetVector()-ExitPoint;
    PathLen2D+=tmpVec.Mag();

    //-------------------------------------------------
    // Correction, no arbitary coord. sys.
    //-------------------------------------------------

    TVector3 TargDir=EnterPoint-SecondVertex;
    PathLen=PathLen2D*TMath::Sqrt(1+trackPhi*trackPhi);

		Double_t 
			tmpTh=TargDir.x()/TargDir.z(),
			tmpPh=TargDir.y()/TargDir.z(),
			scalarP=R*fRadius2Momentum;

		ZeroOrderCorrection(tmpTh,tmpPh,scalarP,PathLen);

		TargDir.SetXYZ(tmpTh,tmpPh,1);

    //-------------------------------------------------
    // Out put section, no arbitary coord. sys.
    //-------------------------------------------------

    //TVector3 &P,        //output:   vector momentum     HCS
    TOpticsDir vP(TOpticsDir::kTCS,this,TargDir.Unit()*scalarP);
    vP.TransCoordSys(TOpticsDir::kHCS);
    P=vP.GetVector();    
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","vP.Print();");vP.Print(););

    //TVector3 &Vertex,   //output:   vertex cordinate    HCS
    TOpticsPos vVertex(TOpticsDir::kTCS,this,SecondVertex);
    vVertex.TransCoordSys(TOpticsDir::kHCS);
    Vertex=vVertex.GetVector();    
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","vVertex.Print();");vVertex.Print(););

    //Double_t &TargetX,  //output:   Target X            TCS
    //Double_t &TargetY, //output:   Target Y            TCS
    TVector3 VertexProjTargZ0=CrossingSurfLine3D(
        TVector3(0,0,0),
        TVector3(0,0,1),
        SecondVertex,
        TargDir
        );
    TargetX=VertexProjTargZ0.X();
    TargetY=VertexProjTargZ0.Y();
    DEBUG_LINE_MASSINFO(Info("ApplyOptics","VertexProjTargZ0.Print();");VertexProjTargZ0.Print(););

    //Double_t &TargetTheta,//output: Target Theta        TCS
    TargetTheta=TargDir.X()/TargDir.Z();

    //Double_t &TargetPhi,//output:   Target Phi          TCS
    TargetPhi=TargDir.Y()/TargDir.Z();

    //Double_t &PathLen   //output:   Length of path      any coordinate
    //PathLen=PathLen2D*TMath::Sqrt(1+trackPhi*trackPhi);
    DEBUG_MASSINFO(Here("ApplyOptics"),"TargetTheta=%f, TargetPhi=%f, PathLen=%f",
        TargetTheta, TargetPhi, PathLen);

    return kOK;
};

//_____________________________________________________________________________

inline TComplex operator +(Int_t d, const TComplex & c)
{return ((Double_t)d)+c;}
inline TComplex operator -(Int_t d, const TComplex & c)
{return ((Double_t)d)-c;}

//_____________________________________________________________________________

Double_t   THaOpticsAnalytical::
CalxA(
      Double_t xB,
      Double_t zB,
      Double_t TanF,
      Double_t xx,
      Double_t zz
      )
{
    //calculating xA. See following page for definition:
    //http://www.jlab.org/~jinhuang/BigBiteDoc/THaOpticsAnalytical.png

    TComplex xA;

#define Sqrt(x) CSqrt(x)
    xA=
        (2*xB + xx + 2*TanF*zB - TanF*zz)/3. + 
        ((1 - Complex(0,1)*Sqrt(3))*
        (-Power(-2*xB - xx - 2*TanF*zB + TanF*zz,2) + 
        3*(Power(xB,2) + 2*xB*xx + 2*TanF*xB*zB + 2*TanF*xx*zB - 
        Power(zB,2) - 2*TanF*xB*zz + 2*zB*zz)))/
        (3.*Power(2,0.6666666666666666)*
        Power(-2*Power(xB,3) + 6*Power(xB,2)*xx - 6*xB*Power(xx,2) + 
        2*Power(xx,3) - 6*TanF*Power(xB,2)*zB + 12*TanF*xB*xx*zB - 
        6*TanF*Power(xx,2)*zB + 18*xB*Power(zB,2) + 
        12*Power(TanF,2)*xB*Power(zB,2) - 18*xx*Power(zB,2) - 
        12*Power(TanF,2)*xx*Power(zB,2) + 18*TanF*Power(zB,3) + 
        16*Power(TanF,3)*Power(zB,3) - 6*TanF*Power(xB,2)*zz + 
        12*TanF*xB*xx*zz - 6*TanF*Power(xx,2)*zz + 18*xB*zB*zz + 
        6*Power(TanF,2)*xB*zB*zz - 18*xx*zB*zz - 
        6*Power(TanF,2)*xx*zB*zz - 18*TanF*Power(zB,2)*zz - 
        24*Power(TanF,3)*Power(zB,2)*zz - 6*Power(TanF,2)*xB*Power(zz,2) + 
        6*Power(TanF,2)*xx*Power(zz,2) + 18*TanF*zB*Power(zz,2) + 
        12*Power(TanF,3)*zB*Power(zz,2) - 2*Power(TanF,3)*Power(zz,3) + 
        Sqrt(Power(-2*Power(xB,3) + 6*Power(xB,2)*xx - 6*xB*Power(xx,2) + 
        2*Power(xx,3) - 6*TanF*Power(xB,2)*zB + 12*TanF*xB*xx*zB - 
        6*TanF*Power(xx,2)*zB + 18*xB*Power(zB,2) + 
        12*Power(TanF,2)*xB*Power(zB,2) - 18*xx*Power(zB,2) - 
        12*Power(TanF,2)*xx*Power(zB,2) + 18*TanF*Power(zB,3) + 
        16*Power(TanF,3)*Power(zB,3) - 6*TanF*Power(xB,2)*zz + 
        12*TanF*xB*xx*zz - 6*TanF*Power(xx,2)*zz + 18*xB*zB*zz + 
        6*Power(TanF,2)*xB*zB*zz - 18*xx*zB*zz - 
        6*Power(TanF,2)*xx*zB*zz - 18*TanF*Power(zB,2)*zz - 
        24*Power(TanF,3)*Power(zB,2)*zz - 
        6*Power(TanF,2)*xB*Power(zz,2) + 
        6*Power(TanF,2)*xx*Power(zz,2) + 18*TanF*zB*Power(zz,2) + 
        12*Power(TanF,3)*zB*Power(zz,2) - 2*Power(TanF,3)*Power(zz,3),
        2) + 4*Power(-Power(-2*xB - xx - 2*TanF*zB + TanF*zz,2) + 
        3*(Power(xB,2) + 2*xB*xx + 2*TanF*xB*zB + 2*TanF*xx*zB - 
        Power(zB,2) - 2*TanF*xB*zz + 2*zB*zz),3)),
        0.3333333333333333)) - ((1 + Complex(0,1)*Sqrt(3))*
        Power(-2*Power(xB,3) + 6*Power(xB,2)*xx - 6*xB*Power(xx,2) + 
        2*Power(xx,3) - 6*TanF*Power(xB,2)*zB + 12*TanF*xB*xx*zB - 
        6*TanF*Power(xx,2)*zB + 18*xB*Power(zB,2) + 
        12*Power(TanF,2)*xB*Power(zB,2) - 18*xx*Power(zB,2) - 
        12*Power(TanF,2)*xx*Power(zB,2) + 18*TanF*Power(zB,3) + 
        16*Power(TanF,3)*Power(zB,3) - 6*TanF*Power(xB,2)*zz + 
        12*TanF*xB*xx*zz - 6*TanF*Power(xx,2)*zz + 18*xB*zB*zz + 
        6*Power(TanF,2)*xB*zB*zz - 18*xx*zB*zz - 
        6*Power(TanF,2)*xx*zB*zz - 18*TanF*Power(zB,2)*zz - 
        24*Power(TanF,3)*Power(zB,2)*zz - 6*Power(TanF,2)*xB*Power(zz,2) + 
        6*Power(TanF,2)*xx*Power(zz,2) + 18*TanF*zB*Power(zz,2) + 
        12*Power(TanF,3)*zB*Power(zz,2) - 2*Power(TanF,3)*Power(zz,3) + 
        Sqrt(Power(-2*Power(xB,3) + 6*Power(xB,2)*xx - 6*xB*Power(xx,2) + 
        2*Power(xx,3) - 6*TanF*Power(xB,2)*zB + 12*TanF*xB*xx*zB - 
        6*TanF*Power(xx,2)*zB + 18*xB*Power(zB,2) + 
        12*Power(TanF,2)*xB*Power(zB,2) - 18*xx*Power(zB,2) - 
        12*Power(TanF,2)*xx*Power(zB,2) + 18*TanF*Power(zB,3) + 
        16*Power(TanF,3)*Power(zB,3) - 6*TanF*Power(xB,2)*zz + 
        12*TanF*xB*xx*zz - 6*TanF*Power(xx,2)*zz + 18*xB*zB*zz + 
        6*Power(TanF,2)*xB*zB*zz - 18*xx*zB*zz - 
        6*Power(TanF,2)*xx*zB*zz - 18*TanF*Power(zB,2)*zz - 
        24*Power(TanF,3)*Power(zB,2)*zz - 
        6*Power(TanF,2)*xB*Power(zz,2) + 
        6*Power(TanF,2)*xx*Power(zz,2) + 18*TanF*zB*Power(zz,2) + 
        12*Power(TanF,3)*zB*Power(zz,2) - 2*Power(TanF,3)*Power(zz,3),
        2) + 4*Power(-Power(-2*xB - xx - 2*TanF*zB + TanF*zz,2) + 
        3*(Power(xB,2) + 2*xB*xx + 2*TanF*xB*zB + 2*TanF*xx*zB - 
        Power(zB,2) - 2*TanF*xB*zz + 2*zB*zz),3)),
        0.3333333333333333))/(6.*Power(2,0.3333333333333333))
        ;
#undef Sqrt

    DEBUG_INFO(Here("THaOpticsAnalytical"),"xA=(%f,%f)",xA.Re(),xA.Im());
    static Int_t countAll=0,countFail=0;
    countAll++;
    if (TMath::Abs(xA.Im())>1e-10)
    {
        countFail++;
        if ((countFail*1.0)/(Double_t)countAll>.01 and countAll>1000)
	    DEBUG_INFO(Here("xA"),"%f percent (%d/%d) event do not have valid solusion! returen central ray position."
			  ,(countFail*100.0)/(Double_t)countAll,countFail,countAll
			  );
        return kBig;
    }
    else
        return xA.Re();
}

//_____________________________________________________________________________
Double_t   THaOpticsAnalytical::
CalR(
     Double_t xA,
     Double_t xB,
     Double_t zB,
     Double_t TanF,
     Double_t xx,
     Double_t zz
     )
{
    //calculating R. See following page for definition:
    //http://www.jlab.org/~jinhuang/BigBiteDoc/THaOpticsAnalytical.png


#define Sqrt(x) TMath::Sqrt(x)
    return 
        Sqrt(Power(xA,4) + Power(TanF,2)*Power(xA,4) - 2*Power(xA,3)*xB - 
        2*Power(TanF,2)*Power(xA,3)*xB + Power(xA,2)*Power(xB,2) + 
        Power(TanF,2)*Power(xA,2)*Power(xB,2) - 2*Power(xA,3)*xx - 
        2*Power(TanF,2)*Power(xA,3)*xx + 4*Power(xA,2)*xB*xx + 
        4*Power(TanF,2)*Power(xA,2)*xB*xx - 2*xA*Power(xB,2)*xx - 
        2*Power(TanF,2)*xA*Power(xB,2)*xx + Power(xA,2)*Power(xx,2) + 
        Power(TanF,2)*Power(xA,2)*Power(xx,2) - 2*xA*xB*Power(xx,2) - 
        2*Power(TanF,2)*xA*xB*Power(xx,2) + Power(xB,2)*Power(xx,2) + 
        Power(TanF,2)*Power(xB,2)*Power(xx,2) + 2*Power(xA,2)*zB*zz + 
        2*Power(TanF,2)*Power(xA,2)*zB*zz - 2*xA*xB*zB*zz - 
        2*Power(TanF,2)*xA*xB*zB*zz - 2*xA*xx*zB*zz - 
        2*Power(TanF,2)*xA*xx*zB*zz + 2*xB*xx*zB*zz + 
        2*Power(TanF,2)*xB*xx*zB*zz + Power(zB,2)*Power(zz,2) + 
        Power(TanF,2)*Power(zB,2)*Power(zz,2))/
        Sqrt(Power(xA,2) - 2*xA*xx + Power(xx,2) - 2*TanF*xA*zz + 
        2*TanF*xx*zz + Power(TanF,2)*Power(zz,2))
        ;
#undef Sqrt
}
//_____________________________________________________________________________

Double_t THaOpticsAnalytical::SetRCentral()
{
    //calcuate the radius of curve of a central ray 
    // central ray is the ray go through z axis of TRCS and target coord. sys. origin point
    //How it works. Set a dummy run of ApplyOptics for central ray at an
    //arbitary R_Central. Then real R_Central will be proportional to the 
    //reconstructed p magnitude
    //
    //if fRCentral is read in from database and valid
    //then no need for calculation again. It will keep unchanged.

    if (fRCentral>0) return fRCentral;

    Info(Here("SetRCentral"),
        "The CurveRadiusOfCentralMomentum do not have a valid value. So it will be calculated according to definition.");

    fRCentral=1;

    TVector3  P;        //output:   vector momentum     HCS
    TVector3  Vertex;   //output:   vertex cordinate    HCS
    Double_t  TargetX;  //output:   Target X            TCS
    Double_t  TargetY; //output:   Target Y            TCS
    Double_t  TargetTheta;//output: Target Theta        TCS
    Double_t  TargetPhi;//output:   Target Phi          TCS
    Double_t  PathLen ;  //output:   Length of path      any coordinate

    TOpticsDir BeamPos(TOpticsDir::kTCS,this,0,0,0);
    TOpticsPos BeamDir(TOpticsDir::kTCS,this,0,1,0);

    BeamDir.TransCoordSys(TOpticsDir::kHCS);
    BeamPos.TransCoordSys(TOpticsDir::kHCS);

    ApplyOptics(0,0,0,0,
        BeamPos.GetVector(),
        BeamDir.GetVector(),
        P,        //output:   vector momentum     HCS
        Vertex,   //output:   vertex cordinate    HCS
        TargetX,  //output:   Target X            TCS
        TargetY, //output:   Target Y            TCS
        TargetTheta,//output: Target Theta        TCS
        TargetPhi,//output:   Target Phi          TCS
        PathLen   //output:   Length of path      any coordinate
        );

    fRCentral=P.Mag()/GetPcentral();


    DEBUG_INFO(Here("SetRCentral"),"Central Momentum %fGeV/c is cooresponding to curving radius %fm",GetPcentral(),fRCentral);

    return fRCentral;
}
//_____________________________________________________________________________
ClassImp(THaOpticsAnalytical);

//_____________________________________________________________________________
//
//  Below is for fitting only
//_____________________________________________________________________________

#if DEBUG_LEVEL>1

#define NPoints 10000

struct SwapInfo
{
    THaOpticsAnalytical* pOptics;
    Int_t    MaxIndex;

    Double_t x[NPoints];
    Double_t theta[NPoints];
    Double_t y[NPoints];
    Double_t phi[NPoints];

    Double_t p[NPoints];
    Double_t vz[NPoints];
};

SwapInfo sSwapInfo;

Double_t FunToMin(Double_t MagDL,Double_t MagS, Double_t fRCentral )
{
    sSwapInfo.pOptics->SetMagneticFieldFrontShift(MagDL);
    sSwapInfo.pOptics->SetMagneticFieldTopCrossing(MagS);
    sSwapInfo.pOptics->SetCurveRadiusOfCentralMomentum(fRCentral);

    Double_t ds=0;

    TVector3  P;        //output:   vector momentum     HCS
    TVector3  Vertex;   //output:   vertex cordinate    HCS
    Double_t  TargetX;  //output:   Target X            TCS
    Double_t  TargetY; //output:   Target Y            TCS
    Double_t  TargetTheta;//output: Target Theta        TCS
    Double_t  TargetPhi;//output:   Target Phi          TCS
    Double_t  PathLen ;  //output:   Length of path      any coordinate

    TVector3 BeamPos(0,0,0);
    TVector3 BeamDir(0,0,1);

    for (Int_t i=0;i<=sSwapInfo.MaxIndex;i++)
    {


        sSwapInfo.pOptics->ApplyOptics(
            sSwapInfo. x[i],
            sSwapInfo. y[i],
            sSwapInfo. theta[i],
            sSwapInfo. phi[i],
            BeamPos,
            BeamDir,
            P,        //output:   vector momentum     HCS
            Vertex,   //output:   vertex cordinate    HCS
            TargetX,  //output:   Target X            TCS
            TargetY, //output:   Target Y            TCS
            TargetTheta,//output: Target Theta        TCS
            TargetPhi,//output:   Target Phi          TCS
            PathLen   //output:   Length of path      any coordinate
            );

        ds+=(P.Mag()-sSwapInfo. p[i])*(P.Mag()-sSwapInfo. p[i]);
    }

    return ds;
}

#include <fstream>
void THaOpticsAnalytical::Fitter(TString sInputFileName)
{
    //fit the for the parameters in the model

    sSwapInfo.pOptics=this;

    char buf[255];
    Double_t x0,y0,xp,yp,p,vz;
    Int_t i;

    std::fstream InFile(sInputFileName.Data(),std::ios_base::in);

    if (!InFile.is_open())
        Fatal("Fitter","InFile Open Failed!");

    for (i=0;true;i++)
    {
        InFile.getline(buf,255);

        //Info("SetTrack","Get Buffer: %s",buf);
        if (sscanf(buf,"%lg,%lg,%lg,%lg,%lg,%lg",&x0,&y0,&xp,&yp,&p,&vz)!=6)
            break;

        DEBUG_MASSINFO("Fitter","track get(%f,%f,%f,%f,%f,%f);",x0,y0,xp,yp,p,vz);

        sSwapInfo. x[i]=x0;
        sSwapInfo. y[i]=y0;
        sSwapInfo. theta[i]=xp;
        sSwapInfo. phi[i]=yp;

        sSwapInfo. p[i]=p;
        sSwapInfo. vz[i]=vz;
    }

    sSwapInfo.MaxIndex=i-1;

    return;
}

#endif//#if DEBUG_LEVEL>1

