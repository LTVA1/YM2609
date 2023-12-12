#include "eff/chorus.h"
#include "eff/CMyFilter.h"
#include "eff/distortion.h"
#include "eff/eq3band.h"
#include "eff/HPFLPF.h"
#include "eff/Compressor.h"
// #include "fmgen.h"
#include "Timer.h"
#include "psg.h"
#include "psg2.h"
#include "opna2.h"
#include <iostream>

int main()
{
    /*CMyFilter filt;

    filt.AllPass(55.6, 1.3, 44000.7);

    reverb rv = reverb(1000, 12);

    rv.StoreDataC(4, 5);

    fmgen fm = fmgen();*/

    std::cout << "euiyrte" << std::endl;

    //PSG ps = PSG();

    //ps.SetReg(13, 0x70);

    int** buffer = new int*[2];

    buffer[0] = new int[200];
    buffer[1] = new int[200];

    //ps.Mix(buffer, 200);

    reverb* rev = new reverb(44100 * 4, 39);
    distortion* dist = new distortion(8000000, 39);
    chorus* chor = new chorus(8000000, 39);
    eq3band* eq = new eq3band(44100);
    HPFLPF* filt = new HPFLPF(8000000, 39);
    ReversePhase* reph = new ReversePhase();
    Compressor* comp = new Compressor(44100, 39);

    //OPNA2* opna2 = new OPNA2(0, new reverb(44100 * 4, 39), new distortion(8000000, 39), new chorus(8000000, 39), new eq3band(44100), new HPFLPF(8000000, 39), new ReversePhase(), new Compressor(44100, 39));
    OPNA2* opna2 = new OPNA2(0, rev, dist, chor, eq, filt, reph, comp);

    opna2->Init(8000000, 8000000);

    opna2->Reset();

    opna2->Mix(buffer, 200);

    std::cout << buffer[0][30] << std::endl;

    int i = 0;
    std::cin >> i;

    delete rev;
    delete dist;
    delete chor;
    delete eq;
    delete filt;
    delete reph;
    delete comp;

    delete[] buffer[0];
    delete[] buffer[1];
    delete[] buffer;

    delete opna2;

    return 0;
}