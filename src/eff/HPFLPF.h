#pragma once

#include <math.h>
#include <stdint.h>

#include "CMyFilter.h"

class HPFLPF
{
    // フィルタークラス(https://vstcpp.wpblog.jp/?page_id=728 より)
    // エフェクターのパラメーター
    private:

        class ChInfo
        {
            public:
                CMyFilter highpassL = CMyFilter();
                CMyFilter highpassR = CMyFilter();
                
                CMyFilter lowpassL = CMyFilter();
                CMyFilter lowpassR = CMyFilter();

                bool hsw = false;
                float hFreq = 1000.0f;
                float hQ = (float)(1.0f / sqrt(2.0f));

                bool lsw = false;
                float lFreq = 300.0f;
                float lQ = (float)(1.0f / sqrt(2.0f));
        };
        
        int clock;
        int maxCh;
        ChInfo* chInfo = NULL;
        float* fbuf = new float[2] { 0.0f, 0.0f };
        int currentCh = 0;

    public:
        HPFLPF(int clock, int maxCh)
        {
            this->clock = clock;
            this->maxCh = maxCh;
            
            Init();
        }

        void Init()
        {
            chInfo = new ChInfo[maxCh];
            for (int i = 0; i < maxCh; i++)
            {
                chInfo[i] = ChInfo();

                // 内部変数
                // 高音域のみ通す(低音域をカットする)フィルタ設定(左右分)
                // カットする周波数の目安は20Hz～300Hz程度
                // 増幅量が大きくなれば、カットオフ周波数も大きくするとよい
                chInfo[i].hFreq = 1000.0f;
                chInfo[i].hQ = (float)(1.0f / sqrt(2.0f));
                chInfo[i].lFreq = 300.0f;
                chInfo[i].lQ = (float)(1.0f / sqrt(2.0f));

                chInfo[i].hsw = false;
                chInfo[i].highpassL = CMyFilter();
                chInfo[i].highpassR = CMyFilter();

                chInfo[i].lsw = false;
                chInfo[i].lowpassL = CMyFilter();
                chInfo[i].lowpassR = CMyFilter();

                if (freqTable == NULL) chInfo[i].highpassL.makeTable();
                updateParam(i);
            }
        }

        void updateParam(int ch)
        {
            chInfo[ch].highpassL.HighPass(chInfo[ch].hFreq, chInfo[ch].hQ, clock);
            chInfo[ch].highpassR.HighPass(chInfo[ch].hFreq, chInfo[ch].hQ, clock);
            chInfo[ch].lowpassL.LowPass(chInfo[ch].lFreq, chInfo[ch].lQ, clock);
            chInfo[ch].lowpassR.LowPass(chInfo[ch].lFreq, chInfo[ch].lQ, clock);
        }

        void updateParamHPF(int ch)
        {
            chInfo[ch].highpassL.HighPass(chInfo[ch].hFreq, chInfo[ch].hQ, clock);
            chInfo[ch].highpassR.HighPass(chInfo[ch].hFreq, chInfo[ch].hQ, clock);
        }

        void updateParamLPF(int ch)
        {
            chInfo[ch].lowpassL.LowPass(chInfo[ch].lFreq, chInfo[ch].lQ, clock);
            chInfo[ch].lowpassR.LowPass(chInfo[ch].lFreq, chInfo[ch].lQ, clock);
        }

        void Mix(int ch, int& inL, int& inR, int wavelength = 1)
        {
            if (ch < 0) return;
            if (ch >= maxCh) return;
            if (chInfo == NULL) return;
            if (!chInfo[ch].hsw && !chInfo[ch].lsw) return;

            fbuf[0] = inL / chInfo[ch].highpassL.convInt;
            fbuf[1] = inR / chInfo[ch].highpassL.convInt;

            // 入力信号にフィルタを適用する
            if (chInfo[ch].hsw)
            {
                fbuf[0] = chInfo[ch].highpassL.Process(fbuf[0]);
                fbuf[1] = chInfo[ch].highpassR.Process(fbuf[1]);
            }

            if (chInfo[ch].lsw)
            {
                fbuf[0] = chInfo[ch].lowpassL.Process(fbuf[0]);
                fbuf[1] = chInfo[ch].lowpassR.Process(fbuf[1]);
            }

            inL = (int)(fbuf[0] * chInfo[ch].highpassL.convInt);
            inR = (int)(fbuf[1] * chInfo[ch].highpassL.convInt);
        }

        void SetReg(uint32_t adr, uint8_t data)
        {
            switch (adr)
            {
                case 0:
                    currentCh = fmax(fmin(data & 0x3f, 38), 0);
                    if ((data & 0x80) != 0) Init();
                    updateParam(currentCh);
                    break;
                case 1:
                    chInfo[currentCh].lsw = (data != 0);
                    updateParamLPF(currentCh);
                    break;
                case 2:
                    chInfo[currentCh].lFreq = freqTable[data];
                    updateParamLPF(currentCh);
                    break;
                case 3:
                    chInfo[currentCh].lQ = QTable[data];
                    updateParamLPF(currentCh);
                    break;

                case 4:
                    chInfo[currentCh].hsw = (data != 0);
                    updateParamHPF(currentCh);
                    break;
                case 5:
                    chInfo[currentCh].hFreq = freqTable[data];
                    updateParamHPF(currentCh);
                    break;
                case 6:
                    chInfo[currentCh].hQ = QTable[data];
                    updateParamHPF(currentCh);
                    break;
            }
        }
};