#include "eff/chorus.h"
#include "eff/CMyFilter.h"
#include "eff/reverb.h"
#include "fmgen.h"
#include "Timer.h"
#include "psg.h"
#include "psg2.h"
#include <iostream>

int main()
{
    CMyFilter filt;

    filt.AllPass(55.6, 1.3, 44000.7);

    reverb rv = reverb(1000, 12);

    rv.StoreDataC(4, 5);

    fmgen fm = fmgen();

    std::cout << "euiyrte" << std::endl;

    PSG ps = PSG();

    ps.SetReg(13, 0x70);

    int** buffer = new int*[2];

    buffer[0] = new int[200];
    buffer[1] = new int[200];

    ps.Mix(buffer, 200);

    std::cout << buffer[0][30] << std::endl;

    int i = 0;
    std::cin >> i;

    delete[] buffer[0];
    delete[] buffer[1];
    delete[] buffer;

    return 0;
}