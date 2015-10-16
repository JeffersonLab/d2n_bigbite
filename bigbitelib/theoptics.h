// Modified by D. Parno, 22 November 2010, to add p_firstorder variable.

#include <iostream>
#include "TObject.h"

using namespace std;

class theoptics : public TObject { 
 public:
  theoptics(double sievex, double sievey, double sievexp,  double sieveyp, double bendx,
			double bendy, double charge, double vzflag, double bend_theta, double theta_elastic,
			double sieve_flag, double sieve_flag2, double eloss, double p_firstord) :  
		fsievex(sievex), fsievey(sievey), fsievexp(sievexp), fsieveyp(sieveyp), fbendx(bendx),
		fbendy(bendy), fcharge(charge), fvzflag(vzflag), fbend_theta(bend_theta),
		ftheta_elastic(theta_elastic), fsieve_flag(sieve_flag), fsieve_flag2(sieve_flag2),
		feloss(eloss), fp_firstord(p_firstord){}
    virtual ~theoptics() {}
    
    /* virtual void Print(){ */
/*       cout << fsievex << endl; */
/*     } */

    double Geteloss() {return feloss;}
    double Getsievex(){return fsievex;}
    double Getsievey(){return fsievey;}
    double Getsievexp(){return fsievexp;}
    double Getsieveyp(){return fsieveyp;}
    double Getbendx(){return fbendx;}
    double Getbendy(){return fbendy;}
    double Getcharge(){return fcharge;}
    double Getvzflag(){return fvzflag;}
    double Getbend_theta(){return fbend_theta;}
    double Gettheta_elastic(){return ftheta_elastic;}
    double Getsieve_flag(){return fsieve_flag;}
    double Getsieve_flag2(){return fsieve_flag2;}
		double Getp_firstord(){return fp_firstord;}
    
 protected:
    double fsievex;
    double fsievey; 
    double fsievexp;
    double fsieveyp;
    double fbendx;
    double fbendy;
    double fcharge;
    double fvzflag;
    double fbend_theta;
    double ftheta_elastic;
    double fsieve_flag;
    double fsieve_flag2;
    double feloss;
		double fp_firstord;
    
 private:
    ClassDef(theoptics,1);  
};
