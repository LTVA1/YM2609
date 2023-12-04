#include "eff/chorus.h"
#include "eff/CMyFilter.h"
#include "eff/reverb.h"
#include "fmgen.h"
#include <iostream>

int main()
{
    CMyFilter filt;

    filt.AllPass(55.6, 1.3, 44000.7);

    reverb rv = reverb(1000, 12);

    rv.StoreDataC(4, 5);

    fmgen fm = fmgen();

    std::cout << "euiyrte" << std::endl;

    //while(1) {}
    int i = 0;
    std::cin >> i;

    return 0;
}