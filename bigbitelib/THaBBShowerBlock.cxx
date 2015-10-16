///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaBBShowerBlock                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "THaBBShowerBlock.h"

const double THaBBShowerBlock::kBig = 1.e15;

THaBBShowerBlock::THaBBShowerBlock(Float_t x, Float_t y, 
                                   Float_t ped, Float_t gain, 
                                   Int_t row, Int_t col) 
{
    fX=x; fY=y; fPed=ped; fGain=gain; fRow=row; fCol=col;
}


THaBBShowerBlock* THaBBShowerBlock::operator=(THaBBShowerBlock* rhs) {
    SetX(rhs->GetX());
    SetY(rhs->GetY());
    SetE(rhs->GetE());
    SetPed(rhs->GetPed());
    SetGain(rhs->GetGain());
    SetRow(rhs->GetRow());
    SetCol(rhs->GetCol());
    return this;
}

void THaBBShowerBlock::ClearEvent() {
    fE = 0.; fStat = 0;
}



ClassImp(THaBBShowerBlock)

