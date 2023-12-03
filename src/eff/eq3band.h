#pragma once

#include <math.h>
#include <stdint.h>

#include "CMyFilter.h"

//
// 3バンドイコライザー(https://vstcpp.wpblog.jp/?p=1417 より)
//

extern float* freqTable;
extern float* gainTable;
extern float* QTable;

class eq3band
{
    private:
    float fl, fr;
        int samplerate = 44100;

        // エフェクターのパラメーター
        bool lowSw = false;
        float lowfreq = 400.0f; // 低音域の周波数。50Hz～1kHz程度
        float lowgain = 2.0f;   // 低音域のゲイン(増幅値)。-15～15dB程度
        float lowQ = (float)(1.0f / sqrt(2.0f));

        bool midSw = false;
        float midfreq = 1000.0f; // 中音域の周波数。500Hz～4kHz程度
        float midgain = -4.0f;   // 中音域のゲイン(増幅値)。-15～15dB程度
        float midQ = (float)(1.0f / sqrt(2.0f));

        bool highSw = false;
        float highfreq = 4000.0f; // 高音域の周波数。1kHz～12kHz程度
        float highgain = 4.0f;    // 高音域のゲイン(増幅値)。-15～15dB程度
        float highQ = (float)(1.0f / sqrt(2.0f));

        //パラメータのdefault値は
        //low
        // freq:126
        // gain:141
        // Q:67
        //mid
        // freq:162
        // gain:102
        // Q:67
        //high
        // freq:192
        // gain:154
        // Q:67


        // 内部変数
        CMyFilter lowL = CMyFilter(), lowR = CMyFilter();
        CMyFilter midL = CMyFilter(), midR = CMyFilter();
        CMyFilter highL = CMyFilter(), highR = CMyFilter(); // フィルタークラス(https://vstcpp.wpblog.jp/?page_id=728 より)


    eq3band(int samplerate = 44100)
    {
        this->samplerate = samplerate;
        if (freqTable == NULL) lowL.makeTable();
        updateParam();
    }

    void Mix(int* buffer, int nsamples)
    {
        for (int i = 0; i < nsamples; i++)
        {
            fl = buffer[i * 2 + 0] / lowL.convInt;
            fr = buffer[i * 2 + 1] / lowL.convInt;


            // inL[]、inR[]、outL[]、outR[]はそれぞれ入力信号と出力信号のバッファ(左右)
            // wavelenghtはバッファのサイズ、サンプリング周波数は44100Hzとする
            // 入力信号にエフェクトをかける
            // 入力信号にフィルタをかける
            if (lowSw)
            {
                fl = lowL.Process(fl);
                fr = lowR.Process(fr);
            }
            if (midSw)
            {
                fl = midL.Process(fl);
                fr = midR.Process(fr);
            }
            if (highSw)
            {
                fl = highL.Process(fl);
                fr = highR.Process(fr);
            }


            buffer[i * 2 + 0] = (int)(fl * lowL.convInt);
            buffer[i * 2 + 1] = (int)(fr * lowL.convInt);
        }
    }

    void SetReg(uint32_t adr, uint8_t data)
    {
        switch (adr & 0xf)
        {
            case 0:
                lowSw = data != 0;
                break;
            case 1:
                lowfreq = freqTable[data];
                break;
            case 2:
                lowgain = gainTable[data];
                break;
            case 3:
                lowQ = QTable[data];
                break;

            case 4:
                midSw = data != 0;
                break;
            case 5:
                midfreq = freqTable[data];
                break;
            case 6:
                midgain = gainTable[data];
                break;
            case 7:
                midQ = QTable[data];
                break;

            case 8:
                highSw = data != 0;
                break;
            case 9:
                highfreq = freqTable[data];
                break;
            case 10:
                highgain = gainTable[data];
                break;
            case 11:
                highQ = QTable[data];
                break;
        }

        updateParam();
    }

    void updateParam()
    {
        // 低音域を持ち上げる(ローシェルフ)フィルタ設定(左右分)
        lowL.LowShelf(lowfreq, lowQ, lowgain, samplerate);
        lowR.LowShelf(lowfreq, lowQ, lowgain, samplerate);
        // 中音域を持ち上げる(ピーキング)フィルタ設定(左右分)
        midL.Peaking(midfreq, midQ, midgain, samplerate);
        midL.Peaking(midfreq, midQ, midgain, samplerate);
        // 高音域を持ち上げる(ローシェルフ)フィルタ設定(左右分)
        highL.HighShelf(highfreq, highQ, highgain, samplerate);
        highR.HighShelf(highfreq, highQ, highgain, samplerate);
    }
};
