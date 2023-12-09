#pragma once

#include "CMyFilter.h"
#include <cmath>

#include "../macros.h"

float convInt = 21474.83647f;

class Compressor
{
    //フィルタークラス(https://vstcpp.wpblog.jp/?p=1939 より)

    private:
        class ChInfo
        {
            public:
                bool sw = false;

                // エフェクターのパラメーター
                float threshold = 0.3f;  // 圧縮が始まる音圧。0.1～1.0程度
                float ratio = 2.0f; // 圧縮する割合。2.0～10.0程度
                float volume = 2.0f; // 最終的な音量。1.0～3.0程度

                // 内部変数
                // フィルタークラス(https://vstcpp.wpblog.jp/?page_id=728 より)
                CMyFilter envfilterL, envfilterR;   // 音圧を検知するために使うローパスフィルタ
                CMyFilter gainfilterL, gainfilterR; // 急激な音量変化を避けるためのローパスフィルタ
                float envFreq = 30.0f;
                float envQ = 1.0f;
                float gainFreq = 5.0f;
                float gainQ = 1.0f;
        };

        int samplerate = 44100;
        int currentCh = 0;
        int maxCh;
        ChInfo* chInfo = NULL;
        ChInfo sysInfo;
        float fbuf[2];
        float fr, fl;



    public:
        Compressor(int samplerate, int maxCh)
        {
            this->samplerate = samplerate;
            this->maxCh = maxCh;
            Init();
        }

        ~Compressor()
        {
            delete[] chInfo;
        }

        void Init()
        {
            currentCh = 0;
            chInfo = new ChInfo[maxCh];
            for (int i = 0; i < maxCh; i++)
            {
                chInfo[i] = ChInfo();
                chInfo[i].sw = false;

                chInfo[i].threshold = 0.3f;
                chInfo[i].ratio = 2.0f;
                chInfo[i].volume = 2.0f;

                chInfo[i].envFreq = 30.0f;
                chInfo[i].envQ = 1.0f;
                chInfo[i].gainFreq = 5.0f;
                chInfo[i].gainQ = 1.0f;
                chInfo[i].envfilterL = CMyFilter();
                chInfo[i].envfilterR = CMyFilter();   // 音圧を検知するために使うローパスフィルタ
                chInfo[i].gainfilterL = CMyFilter();
                chInfo[i].gainfilterR = CMyFilter(); // 急激な音量変化を避けるためのローパスフィルタ
                SetLowPass(i, 30.0f, 1.0f, 5.0f, 1.0f);
            }

            sysInfo = ChInfo();
            sysInfo.sw = false;

            sysInfo.threshold = 0.3f;
            sysInfo.ratio = 2.0f;
            sysInfo.volume = 2.0f;
            
            sysInfo.envFreq = 30.0f;
            sysInfo.envQ = 1.0f;
            sysInfo.gainFreq = 5.0f;
            sysInfo.gainQ = 1.0f;
            sysInfo.envfilterL = CMyFilter();
            sysInfo.envfilterR = CMyFilter();   // 音圧を検知するために使うローパスフィルタ
            sysInfo.gainfilterL = CMyFilter();
            sysInfo.gainfilterR = CMyFilter(); // 急激な音量変化を避けるためのローパスフィルタ
            SetLowPass(-1, 30.0f, 1.0f, 5.0f, 1.0f);

        }

        void Mix(int ch, int& inL, int& inR, int wavelength = 1)
        {
            if (ch < 0) return;
            if (ch >= maxCh) return;
            if (chInfo == NULL) return;
            if (!chInfo[ch].sw) return;
            ChInfo info = chInfo[ch];

            fbuf[0] = inL / 21474.83647f;
            fbuf[1] = inR / 21474.83647f;

            // inL[]、inR[]、outL[]、outR[]はそれぞれ入力信号と出力信号のバッファ(左右)
            // wavelenghtはバッファのサイズ、サンプリング周波数は44100Hzとする

            // 入力信号にエフェクトをかける
            // 入力信号の絶対値をとったものをローパスフィルタにかけて音圧を検知する
            float tmpL = info.envfilterL.Process(fabs(fbuf[0]));
            float tmpR = info.envfilterR.Process(fabs(fbuf[1]));

            // 音圧をもとに音量(ゲイン)を調整(左)
            float gainL = 1.0f;

            if (tmpL > info.threshold)
            {
                // スレッショルドを超えたので音量(ゲイン)を調節(圧縮)
                gainL = info.threshold + (tmpL - info.threshold) / info.ratio;
            }
            // 音量(ゲイン)が急激に変化しないようローパスフィルタを通す
            gainL = info.gainfilterL.Process(gainL);

            // 左と同様に右も音圧をもとに音量(ゲイン)を調整
            float gainR = 1.0f;
            if (tmpR > info.threshold)
            {
                gainR = info.threshold + (tmpR - info.threshold) / info.ratio;
            }
            gainR = info.gainfilterR.Process(gainR);


            // 入力信号に音量(ゲイン)をかけ、さらに最終的な音量を調整し出力する
            fbuf[0] = info.volume * gainL * fbuf[0];
            fbuf[1] = info.volume * gainR * fbuf[1];
            inL = (int)(fbuf[0] * 21474.83647f);
            inR = (int)(fbuf[1] * 21474.83647f);
        }

        void Mix(int** buffer, int nsamples)
        {

            if (!sysInfo.sw) return;

            for (int i = 0; i < nsamples; i++)
            {
                fl = buffer[0][i * 2] / convInt;
                fr = buffer[1][i * 2] / convInt;

                float tmpL = sysInfo.envfilterL.Process(fabs(fl));
                float tmpR = sysInfo.envfilterR.Process(fabs(fr));

                // 音圧をもとに音量(ゲイン)を調整(左)
                float gainL = 1.0f;

                if (tmpL > sysInfo.threshold)
                {
                    // スレッショルドを超えたので音量(ゲイン)を調節(圧縮)
                    gainL = sysInfo.threshold + (tmpL - sysInfo.threshold) / sysInfo.ratio;
                }
                // 音量(ゲイン)が急激に変化しないようローパスフィルタを通す
                gainL = sysInfo.gainfilterL.Process(gainL);

                // 左と同様に右も音圧をもとに音量(ゲイン)を調整
                float gainR = 1.0f;
                if (tmpR > sysInfo.threshold)
                {
                    gainR = sysInfo.threshold + (tmpR - sysInfo.threshold) / sysInfo.ratio;
                }
                gainR = sysInfo.gainfilterR.Process(gainR);


                // 入力信号に音量(ゲイン)をかけ、さらに最終的な音量を調整し出力する
                fl = sysInfo.volume * gainL * fl;
                fr = sysInfo.volume * gainR * fr;

                buffer[0][i * 2] = (int)(fl * convInt);
                buffer[1][i * 2] = (int)(fr * convInt);
            }
        }

        void SetReg(bool isSysIns, uint32_t adr, uint8_t data)
        {
            if (!isSysIns && adr == 0)
            {
                currentCh = my_max(my_min(data & 0x3f, 38), 0);
                if ((data & 0x80) != 0) Init();
                return;
            }

            ChInfo info = sysInfo;
            if (!isSysIns)
            {
                info = chInfo[currentCh];
            }

            if (adr == 1)
            {
                info.sw = ((data & 0x80) != 0);
                info.volume = (data & 0x7f) / (127.0f / 4.0f);
            }
            else if (adr == 2)
            {
                info.threshold = my_max(data / 255.0f, 0.1f);
            }
            else if (adr == 3)
            {
                info.ratio = my_max(data / (255.0f / 10.0f), 1.0f);
            }
            else if (adr == 4)
            {
                info.envFreq = data / (255.0f / 80.0f);
                info.envfilterL.LowPass(info.envFreq, info.envQ, samplerate);
                info.envfilterR.LowPass(info.envFreq, info.envQ, samplerate);
            }
            else if (adr == 5)
            {
                info.envQ = QTable[data];
                info.envfilterL.LowPass(info.envFreq, info.envQ, samplerate);
                info.envfilterR.LowPass(info.envFreq, info.envQ, samplerate);
            }
            else if (adr == 6)
            {
                info.gainFreq = data / (255.0f / 80.0f);
                info.gainfilterL.LowPass(info.gainFreq, info.gainQ, samplerate);
                info.gainfilterR.LowPass(info.gainFreq, info.gainQ, samplerate);
            }
            else if (adr == 7)
            {
                info.gainQ = QTable[data];
                info.gainfilterL.LowPass(info.gainFreq, info.gainQ, samplerate);
                info.gainfilterR.LowPass(info.gainFreq, info.gainQ, samplerate);
            }
    }

    private:
        void SetLowPass(int ch, float envFreq, float envQ, float gainFreq, float gainQ)
        {
            if (ch == -1)
            {
                // ローパスフィルターを設定

                // カットオフ周波数が高いほど音圧変化に敏感になる。目安は10～50Hz程度
                sysInfo.envfilterL.LowPass(envFreq, envQ, samplerate);
                sysInfo.envfilterR.LowPass(envFreq, envQ, samplerate);
                // カットオフ周波数が高いほど急激な音量変化になる。目安は5～50Hz程度
                sysInfo.gainfilterL.LowPass(gainFreq, gainQ, samplerate);
                sysInfo.gainfilterR.LowPass(gainFreq, gainQ, samplerate);
                return;
            }

            // ローパスフィルターを設定

            // カットオフ周波数が高いほど音圧変化に敏感になる。目安は10～50Hz程度
            chInfo[ch].envfilterL.LowPass(envFreq, envQ, samplerate);
            chInfo[ch].envfilterR.LowPass(envFreq, envQ, samplerate);
            // カットオフ周波数が高いほど急激な音量変化になる。目安は5～50Hz程度
            chInfo[ch].gainfilterL.LowPass(gainFreq, gainQ, samplerate);
            chInfo[ch].gainfilterR.LowPass(gainFreq, gainQ, samplerate);
        }
};
