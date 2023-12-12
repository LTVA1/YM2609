#pragma once

#include "psg.h"
#include "eff/reverb.h"
#include "eff/chorus.h"
#include "eff/distortion.h"
#include "eff/ReversePhase.h"
#include "eff/HPFLPF.h"
#include "eff/Compressor.h"
#include "fmvgen.h"
#include <cmath>

static const float panTable[8] = { 1.0f, 0.8756f, 0.7512f, 0.6012f, 0.4512f, 0.2506f, 0.0500f, 0.0250f };

class PSG2 : public PSG
{
    protected:
        uint8_t panpot[3];
        uint8_t panpotLM[3];
        uint8_t panpotRM[3];
        uint8_t phaseReset[3];
        bool phaseResetBefore[3];
        uint8_t duty[3];
        double ncountDbl;

    private:
        int GetSampleFromUserDef(int k, uint32_t lv)
        {
            if (chenable[k] == 0) return 0;

            //ユーザー定義
            uint32_t pos = (scount[k] >> (toneshift + oversampling - 3 - 2)) & 63;
            int n = user[duty[k] - 10][pos];
            int x = n - 128;
            return (int)((lv * x) >> 7);
        }

        int GetSampleFromSaw(int k, uint32_t lv)
        {
            if (chenable[k] == 0) return 0;

            int n = ((int)(scount[k] >> (toneshift + oversampling - 3)) & chenable[k]);
            //のこぎり波
            int x = n < 7 ? n : (n - 16);
            return (int)((lv * x) >> 2);
        }

        int GetSampleFromTriangle(int k, uint32_t lv)
        {
            if (chenable[k] == 0) return 0;

            int n = ((int)(scount[k] >> (toneshift + oversampling - 3)) & chenable[k]);
            //三角波
            int x = n < 8 ? (n - 4) : (15 - 4 - n);
            return (int)((lv * x) >> 1);
        }

        int GetSampleFromDuty(int k, uint32_t lv)
        {
            if (chenable[k] == 0) return 0;

            int n = ((int)(scount[k] >> (toneshift + oversampling - 3)) & chenable[k]);
            //矩形波
            int x = n > duty[k] ? 0 : -1;
            return (int)((lv + x) ^ x);
        }

        reverb* rever;
        distortion* distort;
        chorus* chor;
        HPFLPF* hpflpf;
        ReversePhase* reversePhase;
        Compressor* compressor;
        int efcStartCh;
        uint8_t user[6][64];
        int userDefCounter = 0;
        int userDefNum = 0;
        //Func* tblGetSample;
        int num;
        uint8_t chenable[3];
        uint8_t nenable[3];
        uint32_t p[3];
        static constexpr const double ncountDiv = 32.0;

        void makeTblGetSample()
        {
            /*tblGetSample = new Func[] {
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromDuty,
                GetSampleFromTriangle,
                GetSampleFromSaw,
                GetSampleFromUserDef,
                GetSampleFromUserDef,
                GetSampleFromUserDef,
                GetSampleFromUserDef,
                GetSampleFromUserDef,
                GetSampleFromUserDef
            };*/
        }

        int getSample(uint8_t duty, int k, uint32_t lv)
        {
            switch(duty)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                {
                    return GetSampleFromDuty(k, lv);
                    break;
                }

                case 8:
                {
                    return GetSampleFromTriangle(k, lv);
                    break;
                }

                case 9:
                {
                    return GetSampleFromSaw(k, lv);
                    break;
                }

                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                {
                    return GetSampleFromUserDef(k, lv);
                    break;
                }

                default: return 0; break;
            }
        }

    public:

        PSG2()
        {
            
        }

        PSG2(int num, reverb* rever = NULL, distortion* distort = NULL, chorus* chor = NULL, HPFLPF* hpflpf = NULL, ReversePhase* reversePhase = NULL, Compressor* compressor = NULL, int efcStartCh = 0)
        {
            this->num = num;
            this->rever = rever;
            this->distort = distort;
            this->chor = chor;
            this->hpflpf = hpflpf;
            this->reversePhase = reversePhase;
            this->compressor = compressor;
            this->efcStartCh = efcStartCh;
            makeTblGetSample();
        }

        ~PSG2()
        {
        }

        void SetReg(uint32_t regnum, uint8_t data) override
        {
            if (regnum >= 0x10) return;

            reg[regnum] = data;
            int tmp;
            switch (regnum)
            {
                case 0:     // ChA Fine Tune
                case 1:     // ChA Coarse Tune
                    tmp = ((reg[0] + reg[1] * 256) & 0xfff);
                    speriod[0] = (uint32_t)(tmp != 0 ? tperiodbase / tmp : tperiodbase);
                    duty[0] = (uint8_t)(reg[1] >> 4);
                    duty[0] = (uint8_t)(duty[0] < 8 ? (7 - duty[0]) : duty[0]);
                    break;

                case 2:     // ChB Fine Tune
                case 3:     // ChB Coarse Tune
                    tmp = ((reg[2] + reg[3] * 256) & 0xfff);
                    speriod[1] = (uint32_t)(tmp != 0 ? tperiodbase / tmp : tperiodbase);
                    duty[1] = (uint8_t)(reg[3] >> 4);
                    duty[1] = (uint8_t)(duty[1] < 8 ? (7 - duty[1]) : duty[1]);
                    break;

                case 4:     // ChC Fine Tune
                case 5:     // ChC Coarse Tune
                    tmp = ((reg[4] + reg[5] * 256) & 0xfff);
                    speriod[2] = (uint32_t)(tmp != 0 ? tperiodbase / tmp : tperiodbase);
                    duty[2] = (uint8_t)(reg[5] >> 4);
                    duty[2] = (uint8_t)(duty[2] < 8 ? (7 - duty[2]) : duty[2]);
                    break;

                case 6:     // Noise generator control
                    data &= 0x1f;
                    nperiod = data != 0 ? nperiodbase / data : nperiodbase;
                    break;

                case 7:
                    if ((data & 0x09) == 0) { phaseResetBefore[0] = false; }
                    if ((data & 0x12) == 0) { phaseResetBefore[1] = false; }
                    if ((data & 0x24) == 0) { phaseResetBefore[2] = false; }
                    break;
                case 8:
                    olevel[0] = (uint32_t)((mask & 1) != 0 ? EmitTable[(data & 15) * 2 + 1] : 0);
                    panpot[0] = (uint8_t)(data >> 6);
                    panpot[0] = (uint8_t)(panpot[0] == 0 ? 3 : panpot[0]);
                    phaseReset[0] = (uint8_t)((data & 0x20) != 0 ? 1 : 0);
                    break;

                case 9:
                    olevel[1] = (uint32_t)((mask & 2) != 0 ? EmitTable[(data & 15) * 2 + 1] : 0);
                    panpot[1] = (uint8_t)(data >> 6);
                    panpot[1] = (uint8_t)(panpot[1] == 0 ? 3 : panpot[1]);
                    phaseReset[1] = (uint8_t)((data & 0x20) != 0 ? 1 : 0);
                    break;

                case 10:
                    olevel[2] = (uint32_t)((mask & 4) != 0 ? EmitTable[(data & 15) * 2 + 1] : 0);
                    panpot[2] = (uint8_t)(data >> 6);
                    panpot[2] = (uint8_t)(panpot[2] == 0 ? 3 : panpot[2]);
                    phaseReset[2] = (uint8_t)((data & 0x20) != 0 ? 1 : 0);
                    break;

                case 11:    // Envelop period
                case 12:
                    tmp = ((reg[11] + reg[12] * 256) & 0xffff);
                    eperiod = (uint32_t)(tmp != 0 ? eperiodbase / tmp : eperiodbase * 2);
                    break;

                case 13:    // Envelop shape
                    ecount = 0;
                    envelop = enveloptable[data & 15];
                    if ((data & 0x80) != 0) userDefCounter = 0;
                    userDefNum = ((data & 0x70) >> 4) % 6;
                    break;

                case 14:    // Define Wave Data
                    user[userDefNum][userDefCounter & 63] = data;
                    //Console.WriteLine("{3} : WF {0} {1} {2} ", ((data & 0x70) >> 4) % 6, userDefCounter & 63, (uint8_t)(data & 0xf), data);
                    userDefCounter++;
                    break;

                case 15:    // Pan mul
                    int ch = (data >> 6) & 0x3;
                    if (ch == 3) break;
                    panpotLM[ch] = (uint8_t)((data >> 3) & 7);
                    panpotRM[ch] = (uint8_t)(data & 7);
                    break;
            }

        }

        void Mix(int** dest, int nsamples) override
        {
            uint8_t r7 = (uint8_t)~reg[7];

            if (((r7 & 0x3f) | ((reg[8] | reg[9] | reg[10]) & 0x1f)) != 0)
            {
                chenable[0] = (uint8_t)((((r7 & 0x01) != 0) && (speriod[0] <= (uint32_t)(1 << toneshift))) ? 15 : 0);
                chenable[1] = (uint8_t)((((r7 & 0x02) != 0) && (speriod[1] <= (uint32_t)(1 << toneshift))) ? 15 : 0);
                chenable[2] = (uint8_t)((((r7 & 0x04) != 0) && (speriod[2] <= (uint32_t)(1 << toneshift))) ? 15 : 0);
                nenable[0] = (uint8_t)((r7 & 0x08) != 0 ? 1 : 0);
                nenable[1] = (uint8_t)((r7 & 0x10) != 0 ? 1 : 0);
                nenable[2] = (uint8_t)((r7 & 0x20) != 0 ? 1 : 0);
                p[0] = ((mask & 1) != 0 && (reg[8] & 0x10) != 0) ? (uint32_t)3 : 0;
                p[1] = ((mask & 2) != 0 && (reg[9] & 0x10) != 0) ? (uint32_t)3 : 1;
                p[2] = ((mask & 4) != 0 && (reg[10] & 0x10) != 0) ? (uint32_t)3 : 2;
                if (!phaseResetBefore[0] && phaseReset[0] != 0 && (r7 & 0x09) != 0) { scount[0] = 0; phaseResetBefore[0] = true; }
                if (!phaseResetBefore[1] && phaseReset[1] != 0 && (r7 & 0x12) != 0) { scount[1] = 0; phaseResetBefore[1] = true; }
                if (!phaseResetBefore[2] && phaseReset[2] != 0 && (r7 & 0x24) != 0) { scount[2] = 0; phaseResetBefore[2] = true; }

                int noise, sample, sampleL, sampleR, revSampleL, revSampleR;
                uint32_t env;
                int nv = 0;

                if (p[0] != 3 && p[1] != 3 && p[2] != 3)
                {
                    // エンベロープ無し
                    if ((r7 & 0x38) == 0)
                    {
                        int ptrDest = 0;
                        // ノイズ無し
                        for (int i = 0; i < nsamples; i++)
                        {
                            sampleL = 0;
                            sampleR = 0;
                            revSampleL = 0;
                            revSampleR = 0;

                            for (int j = 0; j < (1 << oversampling); j++)
                            {
                                for (int k = 0; k < 3; k++)
                                {
                                    sample = getSample(duty[k], k, olevel[k]);
                                    int L = sample;
                                    int R = sample;
                                    distort->Mix(efcStartCh + k, L, R);
                                    chor->Mix(efcStartCh + k, L, R);
                                    hpflpf->Mix(efcStartCh + k, L, R);
                                    compressor->Mix(efcStartCh + k, L, R);
                                    L = (panpot[k] & 2) != 0 ? (int)(L * panTable[panpotLM[k]]) : 0;
                                    R = (panpot[k] & 1) != 0 ? (int)(R * panTable[panpotRM[k]]) : 0;
                                    L *= reversePhase->SSG[num][k][0];
                                    R *= reversePhase->SSG[num][k][1];
                                    revSampleL += (int)(L * rever->SendLevel[efcStartCh + k] * 0.6);
                                    revSampleR += (int)(R * rever->SendLevel[efcStartCh + k] * 0.6);
                                    sampleL += L;
                                    sampleR += R;
                                    scount[k] += speriod[k];
                                }

                            }
                            sampleL /= (1 << oversampling);
                            sampleR /= (1 << oversampling);
                            revSampleL /= (1 << oversampling);
                            revSampleR /= (1 << oversampling);

                            fmvgen::StoreSample(dest[0][ptrDest], sampleL);
                            fmvgen::StoreSample(dest[1][ptrDest], sampleR);
                            rever->StoreDataC(revSampleL, revSampleR);
                            ptrDest += 2;

                            visVolume = sampleL;

                        }
                    }
                    else
                    {
                        int ptrDest = 0;
                        // ノイズ有り
                        for (int i = 0; i < nsamples; i++)
                        {
                            sampleL = 0;
                            sampleR = 0;
                            revSampleL = 0;
                            revSampleR = 0;
                            sample = 0;
                            for (int j = 0; j < (1 << oversampling); j++)
                            {
                                noise = (int)(noisetable[((uint32_t)ncountDbl >> (int)((noiseshift + oversampling + 6)) & (noisetablesize - 1))]
                                    >> (int)((uint32_t)ncountDbl >> (noiseshift + oversampling + 1)));

                                ncountDbl += ((double)nperiod / ((reg[6] & 0x20) != 0 ? ncountDiv : 1.0));

                                for (int k = 0; k < 3; k++)
                                {
                                    sample = getSample(duty[k], k, olevel[k]);
                                    int L = sample;
                                    int R = sample;

                                    //ノイズ
                                    nv = ((int)(scount[k] >> (toneshift + oversampling)) & 0 | (nenable[k] & noise)) - 1;
                                    sample = (int)((olevel[k] + nv) ^ nv);
                                    L += sample;
                                    R += sample;

                                    distort->Mix(efcStartCh + k, L, R);
                                    chor->Mix(efcStartCh + k, L, R);
                                    hpflpf->Mix(efcStartCh + k, L, R);
                                    compressor->Mix(efcStartCh + k, L, R);
                                    L = (panpot[k] & 2) != 0 ? (int)(L * panTable[panpotLM[k]]) : 0;
                                    R = (panpot[k] & 1) != 0 ? (int)(R * panTable[panpotRM[k]]) : 0;
                                    L *= reversePhase->SSG[num][k][0];
                                    R *= reversePhase->SSG[num][k][1];
                                    revSampleL += (int)(L * rever->SendLevel[efcStartCh + k] * 0.6);
                                    revSampleR += (int)(R * rever->SendLevel[efcStartCh + k] * 0.6);
                                    sampleL += L;
                                    sampleR += R;
                                    scount[k] += speriod[k];
                                }
                            }

                            sampleL /= (1 << oversampling);
                            sampleR /= (1 << oversampling);
                            fmvgen::StoreSample(dest[0][ptrDest], sampleL);
                            fmvgen::StoreSample(dest[1][ptrDest], sampleR);
                            rever->StoreDataC(revSampleL, revSampleR);
                            ptrDest += 2;

                            visVolume = sampleL;

                        }
                    }

                    // エンベロープの計算をさぼった帳尻あわせ
                    ecount = (uint32_t)((ecount >> 8) + (eperiod >> (8 - oversampling)) * nsamples);
                    if (ecount >= (1 << (envshift + 6 + oversampling - 8)))
                    {
                        if ((reg[0x0d] & 0x0b) != 0x0a)
                            ecount |= (1 << (envshift + 5 + oversampling - 8));
                        ecount &= (1 << (envshift + 6 + oversampling - 8)) - 1;
                    }
                    ecount <<= 8;
                }
                else
                {
                    int ptrDest = 0;
                    // エンベロープあり
                    for (int i = 0; i < nsamples; i++)
                    {
                        sampleL = 0;
                        sampleR = 0;
                        revSampleL = 0;
                        revSampleR = 0;

                        for (int j = 0; j < (1 << oversampling); j++)
                        {
                            env = envelop[ecount >> (envshift + oversampling)];
                            ecount += eperiod;
                            if (ecount >= (1 << (envshift + 6 + oversampling)))
                            {
                                if ((reg[0x0d] & 0x0b) != 0x0a)
                                    ecount |= (1 << (envshift + 5 + oversampling));
                                ecount &= (1 << (envshift + 6 + oversampling)) - 1;
                            }
                            noise = (int)(noisetable[((uint32_t)ncountDbl >> (int)((noiseshift + oversampling + 6)) & (noisetablesize - 1))]
                                >> (int)((uint32_t)ncountDbl >> (noiseshift + oversampling + 1)));
                            ncountDbl += (nperiod / ((reg[6] & 0x20) != 0 ? ncountDiv : 1.0));

                            for (int k = 0; k < 3; k++)
                            {
                                uint32_t lv = (p[k] == 3 ? env : olevel[k]);
                                sample = getSample(duty[k], k, lv);
                                int L = sample;
                                int R = sample;

                                //ノイズ
                                nv = ((int)(scount[k] >> (toneshift + oversampling)) & 0 | (nenable[k] & noise)) - 1;
                                sample = (int)((lv + nv) ^ nv);
                                L += sample;
                                R += sample;

                                distort->Mix(efcStartCh + k, L, R);
                                chor->Mix(efcStartCh + k, L, R);
                                hpflpf->Mix(efcStartCh + k, L, R);
                                compressor->Mix(efcStartCh + k, L, R);
                                L = (panpot[k] & 2) != 0 ? (int)(L * panTable[panpotLM[k]]) : 0;
                                R = (panpot[k] & 1) != 0 ? (int)(R * panTable[panpotRM[k]]) : 0;
                                L *= reversePhase->SSG[num][k][0];
                                R *= reversePhase->SSG[num][k][1];
                                revSampleL += (int)(L * rever->SendLevel[efcStartCh + k] * 0.6);
                                revSampleR += (int)(R * rever->SendLevel[efcStartCh + k] * 0.6);
                                sampleL += L;
                                sampleR += R;
                                scount[k] += speriod[k];
                            }

                        }
                        sampleL /= (1 << oversampling);
                        sampleR /= (1 << oversampling);
                        revSampleL /= (1 << oversampling);
                        revSampleR /= (1 << oversampling);

                        fmvgen::StoreSample(dest[0][ptrDest], sampleL);
                        fmvgen::StoreSample(dest[1][ptrDest], sampleR);
                        rever->StoreDataC(revSampleL, revSampleR);
                        ptrDest += 2;

                        visVolume = sampleL;

                    }
                }
            }
        }

        uint8_t* GetUserWave(int n)
        {
            return user[my_min(my_max(n, 0), 5)];
        }
};