#pragma once

#include "pantable_opna.h"
#include "macros.h"
#include "Timer.h"
#include "fmvgen.h"

class FM6
{
    public:
        //OPNA2* parent = NULL;

        int fmvolume;
        fmvgen::Channel4 ch[6];
        Chip chip;

        Timer tim;

        int wavetype = 0;
        int waveCh = 0;
        int wavecounter = 0;
        bool waveSetDic = false;

        void Reset()
        {
            uint32_t i;
            for (i = 0x20; i < 0x28; i++) SetReg(i, 0);
            for (i = 0x30; i < 0xc0; i++) SetReg(i, 0);
            for (i = 0x130; i < 0x1c0; i++) SetReg(i, 0);
            for (i = 0; i < 6; i++)
            {
                pan[i] = 3;
                panL[i] = panTable_opna[0];
                panR[i] = panTable_opna[0];
                ch[i].Reset();
            }
        }

        FM6()
        {

        }

        FM6(int n, reverb* rever = NULL, distortion* distort = NULL, chorus* chor = NULL, HPFLPF* hpflpf = NULL, ReversePhase* reversePhase = NULL, Compressor* compressor = NULL, int efcStartCh = 0)
        {
            this->num = n;
            this->reverb = rever;
            this->distortion = distort;
            this->chorus = chor;
            this->hpflpf = hpflpf;
            this->reversePhase = reversePhase;
            this->efcStartCh = efcStartCh;
            this->compressor = compressor;

            chip = Chip();

            for (int i = 0; i < 6; i++)
            {
                ch[i] = fmvgen::Channel4(i + n * 6);
                ch[i].SetChip(chip);
                //ch[i].SetType(fmgen::OpType.typeN);
                ch[i].SetType((OpType)0);
            }

            csmch = 2;
        }

        void Mix(int** buffer, int nsamples, uint8_t regtc)
        {
            if (fmvolume <= 0) return;

            this->regtc = regtc;
            // 準備
            // Set F-Number
            if ((tim.regtc & 0xc0) == 0)
                ch[csmch].SetFNum(fnum[2]);// csmch - ch]);
                //ch[2].SetFNum(fnum[2]);
            else
            {
                // 効果音モード
                ch[csmch].op[0].SetFNum(fnum3[1]);
                ch[csmch].op[1].SetFNum(fnum3[2]);
                ch[csmch].op[2].SetFNum(fnum3[0]);
                ch[csmch].op[3].SetFNum(fnum[2]);
            }

            int act = (((ch[2].Prepare() << 2) | ch[1].Prepare()) << 2) | ch[0].Prepare();
            if ((reg29 & 0x80) != 0)
                act |= (ch[3].Prepare() | ((ch[4].Prepare() | (ch[5].Prepare() << 2)) << 2)) << 6;
            if ((reg22 & 0x08) == 0)
                act &= 0x555;

            if ((act & 0x555) == 0) return;

            Mix6(buffer, nsamples, act);
        }

        // ---------------------------------------------------------------------------
        //	レジスタアレイにデータを設定
        //
        void SetReg(uint32_t addr, uint32_t data)
        {
            if (addr < 0x20) return;
            if (addr >= 0x100 && addr < 0x120) return;

            int c = (int)(addr & 3);
            uint32_t modified;

            int cnt;
            int d;

            switch (addr)
            {

                // Timer -----------------------------------------------------------------
                case 0x24:
                case 0x25:
                    tim.SetTimerA(addr, data);
                    break;

                case 0x26:
                    tim.SetTimerB(data);
                    break;

                case 0x27:
                    tim.SetTimerControl(data);
                    break;

                // Misc ------------------------------------------------------------------
                case 0x28:      // Key On/Off
                    if ((data & 3) < 3)
                    {
                        c = (int)((data & 3) + ((data & 4) != 0 ? 3 : 0));
                        ch[c].KeyControl(data >> 4);
                    }
                    break;

                // Status Mask -----------------------------------------------------------
                case 0x29:
                    reg29 = data;
                    //		UpdateStatus(); //?
                    break;

                case 0x2a:
                    break;

                // WaveType -----------------------------------------------------------
                case 0x2b:
                    wavetype = (int)(data & 0x3);
                    waveCh = (int)((data >> 4) & 0xf);
                    waveCh = my_max(my_min(waveCh, 11), 0);
                    wavecounter = 0;
                    if ((data & 0x4) != 0) fmvgen::waveReset(waveCh, wavetype);
                    waveSetDic = ((data & 0x8) != 0);
                    break;

                // Write WaveData -----------------------------------------------------------
                case 0x2c:

                    if (waveSetDic)
                    {
                        /*if (dicOpeWav.ContainsKey((int)data))
                        {
                            fmvgen::sinetable[waveCh][wavetype] = dicOpeWav[(int)data];
                        }*/
                        break;
                    }

                    cnt = wavecounter / 2;
                    d = wavecounter % 2;

                    uint32_t s;
                    if (d == 0) {
                        s = (uint8_t)data;
                    }
                    else {
                        s = ((sinetable_opna[waveCh][wavetype][cnt] & 0xff) | ((data & 0x1f) << 8));
                    }

                    sinetable_opna[waveCh][wavetype][cnt] = s;
                    wavecounter++;

                    if (FM_OPSINENTS * 2 <= wavecounter) wavecounter = 0;
                    break;

                // Prescaler -------------------------------------------------------------
                /*case 0x2d:
                case 0x2e:
                case 0x2f:
                    parent->SetPrescaler(addr - 0x2d);
                    break;*/

                // F-Number --------------------------------------------------------------
                case 0x1a0:
                case 0x1a1:
                case 0x1a2:
                    c += 3;
                    fnum[c] = (uint32_t)(data + fnum2[c] * 0x100);
                    ch[c].SetFNum(fnum[c]);
                    break;
                case 0xa0:
                case 0xa1:
                case 0xa2:
                    fnum[c] = (uint32_t)(data + fnum2[c] * 0x100);
                    ch[c].SetFNum(fnum[c]);
                    break;

                case 0x1a4:
                case 0x1a5:
                case 0x1a6:
                    c += 3;
                    fnum2[c] = (uint8_t)(data);
                    panL[c] = panTable_opna[(data >> 6) & 3];
                    break;
                case 0xa4:
                case 0xa5:
                case 0xa6:
                    fnum2[c] = (uint8_t)(data);
                    panL[c] = panTable_opna[(data >> 6) & 3];
                    break;

                case 0xa8:
                case 0xa9:
                case 0xaa:
                    fnum3[c] = (uint32_t)(data + fnum2[c + 6] * 0x100);
                    break;

                case 0xac:
                case 0xad:
                case 0xae:
                    fnum2[c + 6] = (uint8_t)(data);
                    break;

                case 0x1ac:
                case 0x1ad:
                case 0x1ae:
                    c += 3;
                    break;

                // Algorithm -------------------------------------------------------------

                case 0x1b0:
                case 0x1b1:
                case 0x1b2:
                    c += 3;
                    ch[c].SetFB((data >> 3) & 7);
                    ch[c].SetAlgorithm(data & 7);
                    panR[c] = panTable_opna[(data >> 6) & 3];
                    break;
                case 0xb0:
                case 0xb1:
                case 0xb2:
                    ch[c].SetFB((data >> 3) & 7);
                    ch[c].SetAlgorithm(data & 7);
                    panR[c] = panTable_opna[(data >> 6) & 3];
                    break;

                case 0x1b4:
                case 0x1b5:
                case 0x1b6:
                    c += 3;
                    pan[c] = (uint8_t)((data >> 6) & 3);
                    ch[c].SetMS(data);
                    ch[c].SetAC((data & 0x08) != 0);
                    break;
                case 0xb4:
                case 0xb5:
                case 0xb6:
                    pan[c] = (uint8_t)((data >> 6) & 3);
                    ch[c].SetMS(data);
                    ch[c].SetAC((data & 0x08) != 0);
                    break;

                // LFO -------------------------------------------------------------------
                case 0x22:
                    modified = reg22 ^ data;
                    reg22 = (uint8_t)data;
                    if ((modified & 0x8) != 0)
                        lfocount = 0;
                    lfodcount = (reg22 & 8) != 0 ? lfotable[reg22 & 7] : 0;
                    break;

                // 音色 ------------------------------------------------------------------
                default:
                    if (c < 3)
                    {
                        if ((addr & 0x100) != 0)
                            c += 3;
                        SetParameter(&ch[c], addr, data, c);
                    }
                    break;
            }
        }

    protected:
        uint8_t csmch;
        uint32_t fnum[6];
        uint32_t fnum3[3];
        uint8_t fnum2[9];
        uint8_t reg22;
        uint32_t reg29;     // OPNA only?
        uint8_t pan[6];
        //protected float[] panTable = new float[4] { 1.0f, 0.5012f, 0.2512f, 0.1000f };
        float panL[6];
        float panR[6];
        //protected bool[] ac = new bool[6];
        uint32_t lfocount;
        uint32_t lfodcount;
        uint8_t regtc;
        uint32_t lfotable[8];

        void SetParameter(fmvgen::Channel4* ch, uint32_t addr, uint32_t data, int c)
        {
            uint32_t slottable[4] = { 0, 2, 1, 3 };
            uint8_t sltable[16]{
                0,   4,   8,  12,  16,  20,  24,  28,
                32,  36,  40,  44,  48,  52,  56, 124
            };

            if ((addr & 3) < 3)
            {
                uint32_t slot = slottable[(addr >> 2) & 3];
                fmvgen::Operator op = ch->op[slot];

                switch ((addr >> 4) & 15)
                {
                    case 3: // 30-3E DT/MULTI
                        op.SetDT((data >> 4) & 0x07);
                        op.SetMULTI(data & 0x0f);
                        op.SetWaveTypeL((uint8_t)(data >> 7));
                        break;

                    case 4: // 40-4E TL
                        op.SetTL(data & 0x7f, ((regtc & 0x80) != 0) && (c == 2));
                        op.SetWaveTypeH((uint8_t)(data >> 7));
                        break;

                    case 5: // 50-5E KS/AR
                        op.SetKS((data >> 6) & 3);
                        op.SetPhaseReset(data & 0x20);
                        op.SetAR((data & 0x1f) * 2);
                        break;

                    case 6: // 60-6E DR/AMON
                        op.SetDR((data & 0x1f) * 2);
                        op.SetAMON((data & 0x80) != 0);
                        op.SetDT2((data & 0x60) >> 5);
                        break;

                    case 7: // 70-7E SR
                        op.SetSR((data & 0x1f) * 2);
                        op.SetFB((data >> 5) & 7);
                        break;

                    case 8: // 80-8E SL/RR
                        op.SetSL(sltable[(data >> 4) & 15]);
                        op.SetRR((data & 0x0f) * 4 + 2);
                        break;

                    case 9: // 90-9E SSG-EC
                        op.SetSSGEC(data & 0x0f);
                        op.SetALGLink(data >> 4);
                        ch->buildAlg();
                        break;
                }
            }
        }

        void Mix6(int** buffer, int nsamples, int activech)
        {

            // Mix
            idest[0] = pan[0];
            idest[1] = pan[1];
            idest[2] = pan[2];
            idest[3] = pan[3];
            idest[4] = pan[4];
            idest[5] = pan[5];

            int limit = nsamples << 1;
            int v;
            for (int dest = 0; dest < limit; dest += 2)
            {
                //0,1 素
                //2,3 rev
                ibuf[0] = ibuf[1] = ibuf[2] = ibuf[3] = 0;
                if ((activech & 0xaaa) != 0)
                {
                    LFO();
                    MixSubSL(activech, idest, ibuf);
                }
                else
                {
                    MixSubS(activech, idest, ibuf);
                }

                v = ((fmvgen::Limit(ibuf[0], 0x7fff, -0x8000) * fmvolume) >> 14);
                fmvgen::StoreSample(buffer[0][dest], v);
                //visVolume[0] = v;

                v = ((fmvgen::Limit(ibuf[1], 0x7fff, -0x8000) * fmvolume) >> 14);
                fmvgen::StoreSample(buffer[1][dest], v);
                //visVolume[1] = v;

                int rvL = ((fmvgen::Limit(ibuf[2], 0x7fff, -0x8000) * fmvolume) >> 14);
                int rvR = ((fmvgen::Limit(ibuf[3], 0x7fff, -0x8000) * fmvolume) >> 14);
                
                reverb->StoreDataC(rvL, rvR);
            }
        }

        void MixSubS(int activech, int* dest, int* buf)
        {
            int v;
            int L, R;
            if ((activech & 0x001) != 0)
            {
                v = ch[0].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 0, L, R);
                chorus->Mix(efcStartCh + 0, L, R);
                hpflpf->Mix(efcStartCh + 0, L, R);
                compressor->Mix(efcStartCh + 0, L, R);
                buf[0] = (int)((dest[0] >> 1) * L * panL[0]) * reversePhase->FM[num][0][0];
                buf[1] = (int)((dest[0] & 0x1) * R * panR[0]) * reversePhase->FM[num][0][1];
                buf[2] = (int)(buf[0] * reverb->SendLevel[efcStartCh + 0]);
                buf[3] = (int)(buf[1] * reverb->SendLevel[efcStartCh + 0]);
            }

            if ((activech & 0x004) != 0)
            {
                v = ch[1].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 1, L, R);
                chorus->Mix(efcStartCh + 1, L, R);
                hpflpf->Mix(efcStartCh + 1, L, R);
                compressor->Mix(efcStartCh + 1, L, R);
                L = (int)((dest[1] >> 1) * L * panL[1]) * reversePhase->FM[num][1][0];
                R = (int)((dest[1] & 0x1) * R * panR[1]) * reversePhase->FM[num][1][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 1]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 1]);
            }

            if ((activech & 0x010) != 0)
            {
                v = ch[2].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 2, L, R);
                chorus->Mix(efcStartCh + 2, L, R);
                hpflpf->Mix(efcStartCh + 2, L, R);
                compressor->Mix(efcStartCh + 2, L, R);
                L = (int)((dest[2] >> 1) * L * panL[2]) * reversePhase->FM[num][2][0];
                R = (int)((dest[2] & 0x1) * R * panR[2]) * reversePhase->FM[num][2][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 2]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 2]);
            }

            if ((activech & 0x040) != 0)
            {
                v = ch[3].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 3, L, R);
                chorus->Mix(efcStartCh + 3, L, R);
                hpflpf->Mix(efcStartCh + 3, L, R);
                compressor->Mix(efcStartCh + 3, L, R);
                L = (int)((dest[3] >> 1) * L * panL[3]) * reversePhase->FM[num][3][0];
                R = (int)((dest[3] & 0x1) * R * panR[3]) * reversePhase->FM[num][3][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 3]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 3]);
            }

            if ((activech & 0x100) != 0)
            {
                v = ch[4].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 4, L, R);
                chorus->Mix(efcStartCh + 4, L, R);
                hpflpf->Mix(efcStartCh + 4, L, R);
                compressor->Mix(efcStartCh + 4, L, R);
                L = (int)((dest[4] >> 1) * L * panL[4]) * reversePhase->FM[num][4][0];
                R = (int)((dest[4] & 0x1) * R * panR[4]) * reversePhase->FM[num][4][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 4]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 4]);
            }

            if ((activech & 0x400) != 0)
            {
                v = ch[5].Calc();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 5, L, R);
                chorus->Mix(efcStartCh + 5, L, R);
                hpflpf->Mix(efcStartCh + 5, L, R);
                compressor->Mix(efcStartCh + 5, L, R);
                L = (int)((dest[5] >> 1) * L * panL[5]) * reversePhase->FM[num][5][0];
                R = (int)((dest[5] & 0x1) * R * panR[5]) * reversePhase->FM[num][5][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 5]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 5]);
            }
        }

        void MixSubSL(int activech, int* dest, int* buf)
        {
            int v,L,R;
            if ((activech & 0x001) != 0)
            {
                v = ch[0].CalcL();
                L = (int)v;
                R = (int)v;
                distortion->Mix(efcStartCh + 0, L, R);
                chorus->Mix(efcStartCh + 0, L, R);
                hpflpf->Mix(efcStartCh + 0, L, R);
                buf[0] = (int)((dest[0] >> 1) * L * panL[0]) * reversePhase->FM[num][0][0];
                buf[1] = (int)((dest[0] & 0x1) * R * panR[0]) * reversePhase->FM[num][0][1];
                buf[2] = (int)(buf[0] * reverb->SendLevel[efcStartCh + 0]);
                buf[3] = (int)(buf[1] * reverb->SendLevel[efcStartCh + 0]);
            }
            if ((activech & 0x004) != 0)
            {
                v = ch[1].CalcL();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 1, L, R);
                chorus->Mix(efcStartCh + 1, L, R);
                hpflpf->Mix(efcStartCh + 1, L, R);
                L = (int)((dest[1] >> 1) * L * panL[1]) * reversePhase->FM[num][1][0];
                R = (int)((dest[1] & 0x1) * R * panR[1]) * reversePhase->FM[num][1][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 1]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 1]);
            }
            if ((activech & 0x010) != 0)
            {
                v = ch[2].CalcL();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 2, L, R);
                chorus->Mix(efcStartCh + 2, L, R);
                hpflpf->Mix(efcStartCh + 2, L, R);
                L = (int)((dest[2] >> 1) * L * panL[2]) * reversePhase->FM[num][2][0];
                R = (int)((dest[2] & 0x1) * R * panR[2]) * reversePhase->FM[num][2][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 2]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 2]);
            }
            if ((activech & 0x040) != 0)
            {
                v = ch[3].CalcL();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 3, L, R);
                chorus->Mix(efcStartCh + 3, L, R);
                hpflpf->Mix(efcStartCh + 3, L, R);
                L = (int)((dest[3] >> 1) * L * panL[3]) * reversePhase->FM[num][3][0];
                R = (int)((dest[3] & 0x1) * R * panR[3]) * reversePhase->FM[num][3][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 3]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 3]);
            }
            if ((activech & 0x100) != 0)
            {
                v = ch[4].CalcL();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 4, L, R);
                chorus->Mix(efcStartCh + 4, L, R);
                hpflpf->Mix(efcStartCh + 4, L, R);
                L = (int)((dest[4] >> 1) * L * panL[4]) * reversePhase->FM[num][4][0];
                R = (int)((dest[4] & 0x1) * R * panR[4]) * reversePhase->FM[num][4][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 4]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 4]);
            }
            if ((activech & 0x400) != 0)
            {
                v = ch[5].CalcL();
                L = v;
                R = v;
                distortion->Mix(efcStartCh + 5, L, R);
                chorus->Mix(efcStartCh + 5, L, R);
                hpflpf->Mix(efcStartCh + 5, L, R);
                L = (int)((dest[5] >> 1) * L * panL[5]) * reversePhase->FM[num][5][0];
                R = (int)((dest[5] & 0x1) * R * panR[5]) * reversePhase->FM[num][5][1];
                buf[0] += L;
                buf[1] += R;
                buf[2] += (int)(L * reverb->SendLevel[efcStartCh + 5]);
                buf[3] += (int)(R * reverb->SendLevel[efcStartCh + 5]);
            }
        }

        void LFO()
        {
            //	LOG3("%4d - %8d, %8d\n", c, lfocount, lfodcount);

            //	Operator::SetPML(pmtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
            //	Operator::SetAML(amtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
            chip.SetPML((uint32_t)(pmtable_opna[(lfocount >> (FM_LFOCBITS + 1)) & 0xff]));
            chip.SetAML((uint32_t)(amtable_opna[(lfocount >> (FM_LFOCBITS + 1)) & 0xff]));
            lfocount += lfodcount;
        }
    private:
        reverb* reverb;
        distortion* distortion;
        chorus* chorus;
        HPFLPF* hpflpf;
        ReversePhase* reversePhase;
        Compressor* compressor;
        int efcStartCh;
        int num;

        int ibuf[4];
        int idest[6];

        void TimerA() //TODO:: add REAL CSM mode functionality (reset envelope too)
        {
            if ((tim.regtc & 0x80)!=0)
            {
                ch[csmch].KeyControl(0x00);
                ch[csmch].KeyControl(0x0f);
            }
        }

};
