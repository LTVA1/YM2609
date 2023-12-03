#pragma once

#include <cmath>
#include <stdint.h>

#include "CRingBuffur.h"

class ChInfo
{
    public:
        bool sw = false;
        // エフェクターのパラメーター
        float mix = 0.3f; // コーラスのかかり具合。0.0～1.0の間
        float rate = 3.0f; // コーラスの揺らぎの間隔。0Hz～16Hz程度
        float depth = 10.0f; // コーラスの揺らぎの深さ。5.0～200.0サンプル程度
        float feedback = 0.3f; // コーラスのフィードバック量。0.0～1.0の間

        // 内部変数
        CRingBuffur* ringbufL;
        CRingBuffur* ringbufR; // リングバッファ(https://vstcpp.wpblog.jp/?p=1505 より)

        // ディレイタイムをサンプル数に変換して設定
        // depth分だけ読み込むサンプル位置が動くので、動いた際にintervalが0以下にならないようにする
        // とりあえず1000サンプル程度とする
        // (intervalはリングバッファ https://vstcpp.wpblog.jp/?p=1505 参照)
        int delaysample;
        float theta;
        //public float speed;

        ChInfo(int clock = 44100)
        {
            delaysample = 10;
            theta = 0; // ディレイ読み込み位置を揺らすためのsin関数の角度 θ。初期値は0

            sw = false;
            ringbufL = new CRingBuffur(clock, 0.02f);
            ringbufR = new CRingBuffur(clock, 0.02f);
            ringbufL->SetInterval(delaysample);
            ringbufR->SetInterval(delaysample);
        }

        ~ChInfo()
        {
            delete ringbufL;
            delete ringbufR;
        }
};

class chorus
{
    //コーラス・フランジャーの実装例(https://vstcpp.wpblog.jp/?p=1797 より)
    public:
        float clock;
        int maxCh;
        ChInfo** chInfo = NULL;
        int currentCh = 0;

        // 線形補間関数
        // v1とv2を割合tで線形補間する。tは0.0～1.0の範囲とする
        // tが0.0の時v1の値となり、tが1.0の時v2の値となる
        float lerp(float v1, float v2, float t)
        {
            return (1.0f - t) * v1 + t * v2;
        }

        chorus(int clock, int maxCh)
        {
            this->clock = (float)clock;
            this->maxCh = maxCh;
            Init();
        }

        void Init()
        {
            chInfo = new ChInfo*[maxCh];

            for (int i = 0; i < maxCh; i++)
            {
                chInfo[i] = new ChInfo((int)clock);
            }
        }

        ~chorus()
        {
            for (int i = 0; i < maxCh; i++)
            {
                delete chInfo[i];
            }

            delete chInfo;
        }

        void Mix(int ch, int& inL, int& inR)
        {
            if (ch < 0) return;
            if (ch >= maxCh) return;
            if (chInfo == NULL) return;
            if (chInfo[ch] == NULL) return;
            if (!chInfo[ch]->sw) return;

            ChInfo ci = *chInfo[ch];
            float finL = inL / 21474.83647f;
            float finR = inR / 21474.83647f;
            float speed = (2.0f * 3.14159265f * ci.rate) / clock; // 揺らぎのスピード。角速度ωと同じ。

            // inL[]、inR[]、outL[]、outR[]はそれぞれ入力信号と出力信号のバッファ(左右)
            // wavelenghtはバッファのサイズ、サンプリング周波数は44100Hzとする

            // 入力信号にコーラスかける
            // 角度θに角速度を加える
            ci.theta += speed;

            // 読み込み位置を揺らす量を計算
            // sin()関数の結果にdepthを掛ける
            float a = (float)(sin(ci.theta) * ci.depth);

            // 読み込み位置を揺らした際の前後の整数値を取得(あとで線形補間するため)
            int p1 = (int)a;
            int p2 = (int)(a + 1);

            // 前後の整数値から読み込み位置の値を線形補間で割り出す
            float lerpL1 = lerp(ci.ringbufL->Read(p1), ci.ringbufL->Read(p2), a - (float)p1);
            float lerpR1 = lerp(ci.ringbufR->Read(p1), ci.ringbufR->Read(p2), a - (float)p1);

            // 入力信号にディレイ信号を混ぜる
            float tmpL = (1.0f - ci.mix) * finL + ci.mix * lerpL1;
            float tmpR = (1.0f - ci.mix) * finR + ci.mix * lerpR1;

            // ディレイ信号として入力信号とフィードバック信号をリングバッファに書き込み
            ci.ringbufL->Write((1.0f - ci.feedback) * finL + ci.feedback * tmpL);
            ci.ringbufR->Write((1.0f - ci.feedback) * finR + ci.feedback * tmpR);


            // リングバッファの状態を更新する
            ci.ringbufL->Update();
            ci.ringbufR->Update();

            // 出力信号に書き込む
            finL = tmpL;
            finR = tmpR;

            inL = (int)(finL * 21474.83647f);
            inR = (int)(finR * 21474.83647f);
        }

        void SetReg(uint32_t adr, uint8_t data)
        {
            if (adr == 0)
            {
                currentCh = fmax(fmin((data & 0x3f), 38), 0);
                if ((data & 0x80) != 0) Init();
            }
            else if (adr == 1)
            {
                chInfo[currentCh]->sw = ((data & 0x80) != 0);
                chInfo[currentCh]->mix = (data & 0x7f) / 127.0f;
            }
            else if (adr == 2)
            {
                chInfo[currentCh]->rate = 16.0f * (data & 0x7f) / 127.0f;
            }
            else if (adr == 3)
            {
                chInfo[currentCh]->depth = 195.0f * (data & 0x7f) / 127.0f + 5.0f;
            }
            else if (adr == 4)
            {
                chInfo[currentCh]->feedback = (data & 0x7f) / 127.0f;
            }
        }
};
