#pragma once
#include "opna.h"

#include "eff/chorus.h"
#include "eff/Compressor.h"
#include "eff/distortion.h"
#include "eff/reverb.h"
#include "eff/eq3band.h"
#include "eff/HPFLPF.h"
#include "eff/ReversePhase.h"

#include "psg2.h"
#include "ADPCMA.h"
#include "ADPCMB.h"
#include "FM6.h"

//	YM2609(OPNA2) ---------------------------------------------------
class OPNA2 : public OPNABase
{
    //プリセット保持向け
    //public List<uint8_t[]> presetRhythmPCMData = NULL;

    public:
        static constexpr float panTable[4] = { 1.0f, 0.7512f, 0.4512f, 0.0500f };

        // ---------------------------------------------------------------------------
        //	構築
        //
        OPNA2(uint8_t ChipID, reverb* reverb, distortion* distortion, chorus* chorus, eq3band* ep3band, HPFLPF* hpflpf, ReversePhase* reversePhase, Compressor* compressor)
        {
            this->reverb = reverb;
            this->distortion = distortion;
            this->chorus = chorus;
            this->ep3band = ep3band;
            this->hpflpf = hpflpf;
            this->reversePhase = reversePhase;
            this->compressor = compressor;

            for(int i = 0; i < 2; i++)
            {
                fm6[i] = FM6(0, reverb, distortion, chorus, hpflpf,reversePhase,compressor, i * 6);
            }

            for(int i = 0; i < 4; i++)
            {
                psg2[i] = PSG2(0, reverb, distortion, chorus, hpflpf,reversePhase,compressor, 12 + i * 3);
            }

            for(int i = 0; i < 3; i++)
            {
                adpcmb[i] = ADPCMB(0, reverb, distortion, chorus, hpflpf,reversePhase,compressor, 24 + i);
            }

            for(int i = 0; i < 6; i++)
            {
                rhythm[i] = Rhythm(0, reverb, distortion, chorus, hpflpf,reversePhase,compressor, 27 + i);
            }
            
            adpcma = ADPCMA(0, reverb, distortion, chorus, hpflpf, reversePhase, compressor, 33);

            for (int i = 0; i < 6; i++)
            {
                rhythm[i].sample = NULL;
                rhythm[i].pos = 0;
                rhythm[i].size = 0;
                rhythm[i].volume = 0;
            }
            rhythmtvol = 0;

            for (int i = 0; i < 2; i++)
            {
                fm6[i].parent = this;
            }

            for (int i = 0; i < 3; i++)
            {
                adpcmb[i].adpcmmask = (uint32_t)((i == 0) ? 0x3ffff : 0xffffff);
                adpcmb[i].adpcmnotice = 4;
                adpcmb[i].deltan = 256;
                adpcmb[i].adpcmvol = 0;
                adpcmb[i].control2 = 0;
                adpcmb[i].shiftBit = (i == 0) ? 6 : 9;
                adpcmb[i].parent = this;
            }

            csmch = &ch[2];
            this->chipID = ChipID;
        }

        ~OPNA2()
        {
            adpcmbuf = NULL;

            for (int i = 0; i < 6; i++)
            {
                delete[] rhythm[i].sample;
                rhythm[i].sample = NULL;
            }
            
            if(adpcma.buf != NULL)
            {
                delete[] adpcma.buf;
                adpcma.buf = NULL;
            }

            for (int i = 0; i < 3; i++)
            {
                if(adpcmb[i].adpcmbuf != NULL)
                {
                    delete[] adpcmb[i].adpcmbuf;
                    adpcmb[i].adpcmbuf = NULL;
                }
            }
        }

        class Rhythm
        {
            public:
                uint8_t pan;      // ぱん
                int8_t level;     // おんりょう
                int volume;     // おんりょうせってい
                int* sample;      // さんぷる
                uint32_t size;      // さいず
                uint32_t pos;       // いち
                uint32_t step;      // すてっぷち
                uint32_t rate;      // さんぷるのれーと
                reverb* rever;
                distortion* distort;
                chorus* chor;
                HPFLPF* hpflpf;
                int efcCh;
                int num;
                ReversePhase* reversePhase;
                Compressor* compressor;

                Rhythm(int num = 0, reverb* rever = NULL, distortion* distort = NULL, chorus* chor = NULL, HPFLPF* hpflpf = NULL, ReversePhase* reversePhase = NULL, Compressor* compressor = NULL, int efcCh = 0)
                {
                    this->rever = rever;
                    this->distort = distort;
                    this->chor = chor;
                    this->hpflpf = hpflpf;
                    this->efcCh = efcCh;
                    this->reversePhase = reversePhase;
                    this->compressor = compressor;
                    this->num = num;
                }
        };

        bool Init(uint32_t c, uint32_t r)
        {
            return Initt(c, r, false);
        }

        bool Initt(uint32_t c, uint32_t r, bool ipflag)
        {
            return Init(c, r, ipflag, NULL, 0);
        }

        bool Init(uint32_t c, uint32_t r, bool ipflag, uint8_t* _adpcma = NULL, int _adpcma_size = 0)
        {
            rate = 8000;
            //LoadRhythmSample(appendFileReaderCallback);
            //dicOpeWav.Clear();

            if (adpcmb[0].adpcmbuf == NULL)
                adpcmb[0].adpcmbuf = new uint8_t[0x40000];
            if (adpcmb[0].adpcmbuf == NULL)
                return false;
            if (adpcmb[1].adpcmbuf == NULL)
                adpcmb[1].adpcmbuf = new uint8_t[0x1000000];
            if (adpcmb[1].adpcmbuf == NULL)
                return false;
            if (adpcmb[2].adpcmbuf == NULL)
                adpcmb[2].adpcmbuf = new uint8_t[0x1000000];
            if (adpcmb[2].adpcmbuf == NULL)
                return false;

            setAdpcmA(_adpcma, _adpcma_size);

            if (!SetRate(c, r, ipflag))
                return false;
            if (!OPNABase::Init(c, r, ipflag))
                return false;

            Reset();

            SetVolumeFM(0);
            SetVolumePSG(0);
            SetVolumeADPCM(0);
            SetVolumeRhythmTotal(0);
            for (int i = 0; i < 6; i++)
                SetVolumeRhythm(0, 0);
            SetChannelMask(0);

            return true;
        }

        void setAdpcmA(uint8_t* _adpcma, int _adpcma_size)
        {
            adpcma.buf = _adpcma;
            adpcma.size = _adpcma_size;
        }

        void setAdpcm012(int p, uint8_t* _adpcmb)
        {
            adpcmb[p].adpcmbuf = _adpcmb;
        }

        void setOperatorWave(uint8_t* buf, uint32_t len)
        {
            int waveCh=0;
            int wavetype=0;
            int wavecounter = 0;

            for(uint32_t b = 0; b < len; b++)
            {
                int cnt = wavecounter / 2;
                int d = wavecounter % 2;

                uint32_t s;
                if (d == 0) s = b;
                else s = ((fmvgen::sinetable[waveCh][wavetype][cnt] & 0xff) | (uint32_t)((b & 0x1f) << 8));

                fmvgen::sinetable[waveCh][wavetype][cnt] = s;
                wavecounter++;
                if (wavecounter > fmvgen::waveBufSize * 2)
                {
                    wavecounter = 0;
                    wavetype++;
                    if (wavetype > fmvgen::waveTypeSize)
                    {
                        wavetype = 0;
                        waveCh = 0;
                    }
                }
            }
        }

        uint32_t* getOperatorWave(int waveCh,int wavetype)
        {
            return fmvgen::sinetable[waveCh][wavetype];
        }

        void setOperatorWaveDic(int n, uint8_t* buf, uint32_t len)
        {
            int wavecounter = 0;

            /*if (dicOpeWav.ContainsKey(n))
            {
                dicOpeWav.Remove(n);
            }*/

            /*uint32_t* ubuf = new uint32_t[fmvgen::waveBufSize];
            uint32_t s;
            foreach (uint8_t b in buf)
            {
                int cnt = wavecounter / 2;
                int d = wavecounter % 2;

                if (d == 0) s = b;
                else s = ((ubuf[cnt] & 0xff) | (uint32_t)((b & 0x1f) << 8));

                ubuf[cnt] = s;
                wavecounter++;
                if (wavecounter > fmvgen.waveBufSize * 2)
                {
                    wavecounter = 0;
                }
            }

            dicOpeWav.Add(n,ubuf);*/
        }

        uint8_t* getPSGuserWave(int p, int n)
        {
            return psg2[p].GetUserWave(n);
        }

        // ---------------------------------------------------------------------------
        //	サンプリングレート変更
        //
        bool SetRate(uint32_t c, uint32_t r, bool ipflag = false)
        {
            if (!OPNABase::SetRate(c, r, ipflag))
                return false;

            RebuildTimeTable();
            for (int i = 0; i < 6; i++)
            {
                rhythm[i].step = rhythm[i].rate * 1024 / r;
            }

            for (int i = 0; i < 3; i++)
            {
                adpcmb[i].adplbase = (uint32_t)((int)(8192.0 * (clock / 72.0) / r));
                adpcmb[i].adpld = (int)(adpcmb[i].deltan * adpcmb[i].adplbase >> 16);
            }

            adpcma.step = (int)((double)(c) / 54 * 8192 / r);
            return true;
        }

        // ---------------------------------------------------------------------------
        //	合成
        //	in:		buffer		合成先
        //			nsamples	合成サンプル数
        //
        void Mix(int** buffer, int nsamples)
        {

            fm6[0].Mix(buffer, nsamples, regtc);
            fm6[1].Mix(buffer, nsamples, regtc);
            psg2[0].Mix(buffer, nsamples);
            psg2[1].Mix(buffer, nsamples);
            psg2[2].Mix(buffer, nsamples);
            psg2[3].Mix(buffer, nsamples);
            adpcmb[0].Mix(buffer, nsamples);
            adpcmb[1].Mix(buffer, nsamples);
            adpcmb[2].Mix(buffer, nsamples);
            RhythmMix(buffer, nsamples);
            adpcma.Mix(buffer, (uint32_t)nsamples);
            ep3band->Mix(buffer, nsamples);
            compressor->Mix(buffer, nsamples);
        }




        // ---------------------------------------------------------------------------
        //	リセット
        //
        void Reset()
        {
            reg29 = 0x1f;
            rhythmkey = 0;
            limitaddr = 0x3ffff;
            OPNABase::Reset();

            SetPrescaler(0);

            fm6[0].Reset();
            fm6[1].Reset();

            psg2[0].Reset();
            psg2[1].Reset();
            psg2[2].Reset();
            psg2[3].Reset();

            for (int i = 0; i < 3; i++)
            {
                adpcmb[i].statusnext = 0;
                adpcmb[i].memaddr = 0;
                adpcmb[i].adpcmd = 127;
                adpcmb[i].adpcmx = 0;
                adpcmb[i].adpcmplay = false;
                adpcmb[i].adplc = 0;
                adpcmb[i].adpld = 0x100;
            }
        }

        void SetPrescaler(uint32_t p)
        {
            OPNABase::SetPrescaler(p);

            int8_t table[3][2] = { { 6, 4 }, { 3, 2 }, { 2, 1 }, };
            uint8_t table2[8] = { 108, 77, 71, 67, 62, 44, 8, 5 };
            // 512
            if (prescale != p)
            {
                prescale = (uint8_t)p;
                //assert(0 <= prescale && prescale< 3);

                uint32_t fmclock = (uint32_t)(clock / table[p][0] / 12);

                rate = psgrate;

                // 合成周波数と出力周波数の比
                //assert(fmclock< (0x80000000 >> FM_RATIOBITS));
                uint32_t ratio = ((fmclock << FM_RATIOBITS) + rate / 2) / rate;

                SetTimerBase(fmclock);
                //		MakeTimeTable(ratio);
                fm6[0].chip.SetRatio(ratio);
                fm6[1].chip.SetRatio(ratio);

                psg2[0].SetClock((int)(clock / table[p][1]), (int)psgrate);
                psg2[1].SetClock((int)(clock / table[p][1]), (int)psgrate);
                psg2[2].SetClock((int)(clock / table[p][1]), (int)psgrate);
                psg2[3].SetClock((int)(clock / table[p][1]), (int)psgrate);

                for (int i = 0; i < 8; i++)
                {
                    lfotable[i] = (ratio << (2 + FM_LFOCBITS - FM_RATIOBITS)) / table2[i];
                }
            }
        }

        // ---------------------------------------------------------------------------
        //	レジスタアレイにデータを設定
        //
        void SetReg(uint32_t addr, uint32_t data)
        {
            addr &= 0x3ff;

            if (addr < 0x10)
            {
                psg2[0].SetReg(addr, (uint8_t)data);
                return;
            }
            else if (addr >= 0x10 && addr < 0x20)
            {
                RhythmSetReg(addr, (uint8_t)data);
                return;
            }
            else if (addr >= 0xc0 && addr < 0xcc)
            {
                ep3band->SetReg(addr & 0xf, (uint8_t)data);
                return;
            }
            else if (addr >= 0xcc && addr < 0xd9)
            {
                reversePhase->SetReg(addr - 0xcc, (uint8_t)data);
                return;
            }
            else if (addr >= 0x100 && addr < 0x111)
            {
                AdpcmbSetReg(0, addr - 0x100, (uint8_t)data);
                return;
            }
            else if (addr >= 0x111 && addr < 0x118)
            {
                adpcma.SetReg(addr - 0x111, (uint8_t)data);
                return;
            }
            else if (addr >= 0x118 && addr < 0x120)
            {
                return;
            }
            else if (addr >= 0x120 && addr < 0x130)
            {
                psg2[1].SetReg(addr - 0x120, (uint8_t)data);
                return;
            }
            else if (addr >= 0x1c0 && addr < 0x1c7)
            {
                compressor->SetReg(true, addr - 0x1c0 + 1, (uint8_t)data);
                return;
            }
            else if (addr >= 0x200 && addr < 0x210)
            {
                psg2[2].SetReg(addr - 0x200, (uint8_t)data);
                return;
            }
            else if (addr >= 0x210 && addr < 0x220)
            {
                psg2[3].SetReg(addr - 0x210, (uint8_t)data);
                return;
            }
            else if (addr >= 0x300 && addr < 0x311)
            {
                AdpcmbSetReg(1, addr - 0x300, (uint8_t)data);
                return;
            }
            else if (addr >= 0x311 && addr < 0x322)
            {
                AdpcmbSetReg(2, addr - 0x311, (uint8_t)data);
                return;
            }
            else if (addr >= 0x322 && addr < 0x325)
            {
                reverb->SetReg(addr - 0x322, (uint8_t)data);
                if (addr == 0x323)
                {
                    distortion->SetReg(0, (uint8_t)data);//channel変更はアドレスを共有
                    chorus->SetReg(0, (uint8_t)data);
                    hpflpf->SetReg(0, (uint8_t)data);
                    compressor->SetReg(false,0, (uint8_t)data);
                }
                return;
            }
            else if (addr >= 0x325 && addr < 0x328)
            {
                distortion->SetReg(addr - 0x324, (uint8_t)data);//distortionのアドレス0はリバーブと共有
                return;
            }
            else if (addr >= 0x328 && addr < 0x32C)
            {
                chorus->SetReg(addr - 0x327, (uint8_t)data);//chorusのアドレス0はリバーブと共有
                return;
            }
            else if (addr >= 0x32C && addr < 0x330)
            {
                return;
            }
            else if (addr >= 0x3c0 && addr < 0x3c6)
            {
                hpflpf->SetReg(addr - 0x3bf, (uint8_t)data);
                return;
            }
            else if (addr >= 0x3c6 && addr < 0x3cd)
            {
                compressor->SetReg(false, addr - 0x3c5, (uint8_t)data);
                return;
            }

            if (addr < 0x200)
            {
                FmSetReg(0, addr, (uint8_t)data);
            }
            else
            {
                FmSetReg(1, addr - 0x200, (uint8_t)data);
            }
        }

        void FmSetReg(int ch, uint32_t addr, uint8_t data)
        {
            fm6[ch].SetReg(addr, data);
        }

        void AdpcmbSetReg(int ch, uint32_t addr, uint8_t data)
        {
            adpcmb[ch].SetADPCMBReg(addr, data);
        }

        void RhythmSetReg(uint32_t addr, uint8_t data)
        {
            switch (addr)
            {
                // Rhythm ----------------------------------------------------------------
                case 0x10:          // DM/KEYON
                    if ((data & 0x80) == 0)  // KEY ON
                    {
                        rhythmkey |= (uint8_t)(data & 0x3f);
                        if ((data & 0x01) != 0) rhythm[0].pos = 0;
                        if ((data & 0x02) != 0) rhythm[1].pos = 0;
                        if ((data & 0x04) != 0) rhythm[2].pos = 0;
                        if ((data & 0x08) != 0) rhythm[3].pos = 0;
                        if ((data & 0x10) != 0) rhythm[4].pos = 0;
                        if ((data & 0x20) != 0) rhythm[5].pos = 0;
                    }
                    else
                    {                   // DUMP
                        rhythmkey &= (uint8_t)(~(uint8_t)data);
                    }
                    break;

                case 0x11:
                    rhythmtl = (int8_t)(~data & 63);
                    break;

                case 0x18:      // Bass Drum
                case 0x19:      // Snare Drum
                case 0x1a:      // Top Cymbal
                case 0x1b:      // Hihat
                case 0x1c:      // Tom-tom
                case 0x1d:      // Rim shot
                    rhythm[addr & 7].pan = (uint8_t)((data >> 6) & 3);
                    rhythm[addr & 7].level = (int8_t)(~data & 31);
                    break;
            }
        }

        uint32_t GetReg(uint32_t addr)
        {
            return 0;
        }

        //	音量設定
        void SetVolumeFM(int db)
        {
            db = std::min(db, 20);
            if (db > -192)
            {
                fm6[0].fmvolume = (int)(16384.0 * pow(10.0, db / 40.0));
                fm6[1].fmvolume = (int)(16384.0 * pow(10.0, db / 40.0));
            }
            else
            {
                fm6[0].fmvolume = 0;
                fm6[1].fmvolume = 0;
            }
        }

        void SetVolumePSG(int db)
        {
            psg2[0].SetVolume(db);
            psg2[1].SetVolume(db);
            psg2[2].SetVolume(db);
            psg2[3].SetVolume(db);
        }

        void SetVolumeADPCM(int db)
        {
            db = std::min(db, 20);
            if (db > -192)
            {
                adpcmb[0].adpcmvol = (int)(65536.0 * pow(10.0, db / 40.0));
                adpcmb[1].adpcmvol = (int)(65536.0 * pow(10.0, db / 40.0));
                adpcmb[2].adpcmvol = (int)(65536.0 * pow(10.0, db / 40.0));
            }
            else
            {
                adpcmb[0].adpcmvol = 0;
                adpcmb[1].adpcmvol = 0;
                adpcmb[2].adpcmvol = 0;
            }

            adpcmb[0].adpcmvolume = (adpcmb[0].adpcmvol * adpcmb[0].adpcmlevel) >> 12;
            adpcmb[1].adpcmvolume = (adpcmb[1].adpcmvol * adpcmb[1].adpcmlevel) >> 12;
            adpcmb[2].adpcmvolume = (adpcmb[2].adpcmvol * adpcmb[2].adpcmlevel) >> 12;
        }

        // ---------------------------------------------------------------------------
        //	チャンネルマスクの設定
        //
        void SetChannelMask(int mask)
        {
            for (int i = 0; i < 6; i++)
            {
                fm6[0].ch[i].Mute(!((mask & (1 << i)) == 0));
                fm6[1].ch[i].Mute(!((mask & (1 << i)) == 0));
            }

            psg2[0].SetChannelMask((int)(mask >> 6));
            psg2[1].SetChannelMask((int)(mask >> 6));
            psg2[2].SetChannelMask((int)(mask >> 6));
            psg2[3].SetChannelMask((int)(mask >> 6));

            adpcmb[0].adpcmmask_ = (mask & (1 << 9)) != 0;
            adpcmb[1].adpcmmask_ = (mask & (1 << 9)) != 0;
            adpcmb[2].adpcmmask_ = (mask & (1 << 9)) != 0;

            rhythmmask_ = (int)((mask >> 10) & ((1 << 6) - 1));
        }

        uint8_t* GetADPCMBuffer()
        {
            return adpcmbuf;
        }

        // ---------------------------------------------------------------------------
        //	リズム合成
        //

        // ---------------------------------------------------------------------------
        //	音量設定
        //
        void SetVolumeRhythmTotal(int db)
        {
            db = std::min(db, 20);
            rhythmtvol = -(db * 2 / 3);
        }

        void SetVolumeRhythm(int index, int db)
        {
            db = std::min(db, 20);
            rhythm[index].volume = -(db * 2 / 3);
        }

        void SetTimerA(uint32_t addr, uint32_t data)
        {
            OPNABase::SetTimerA(addr, data);
        }

        void SetTimerB(uint32_t data)
        {
            OPNABase::SetTimerB(data);
        }

        void SetTimerControl(uint32_t data)
        {
            OPNABase::SetTimerControl(data);
        }

    // リズム音源関係
    private:
        OPNA2::Rhythm rhythm[6];

        int8_t rhythmtl;      // リズム全体の音量
        int rhythmtvol;
        uint8_t rhythmkey;     // リズムのキー
        uint8_t chipID;

        reverb* reverb;
        distortion* distortion;
        chorus* chorus;
        eq3band* ep3band;
        ReversePhase* reversePhase;
        HPFLPF* hpflpf;
        Compressor* compressor;

        void RhythmMix(int** buffer, int count)
        {
            if (rhythmtvol < 128 && rhythm[0].sample != NULL && ((rhythmkey & 0x3f) != 0))
            {
                int limit = (int)count * 2;
                //visRtmVolume[0] = 0;
                //visRtmVolume[1] = 0;
                for (int i = 0; i < 6; i++)
                {
                    Rhythm r = rhythm[i];
                    if ((rhythmkey & (1 << i)) != 0 && (uint8_t)r.level < 128)
                    {
                        int db = fmvgen::Limit(rhythmtl + rhythmtvol + r.level + r.volume, 127, -31);
                        int vol = tltable[FM_TLPOS + (db << (FM_TLBITS - 7))] >> 4;
                        int maskl = -((r.pan >> 1) & 1);
                        int maskr = -(r.pan & 1);

                        if ((rhythmmask_ & (1 << i)) != 0)
                        {
                            maskl = maskr = 0;
                        }

                        for (int dest = 0; dest < limit && r.pos < r.size; dest += 2)
                        {
                            int sample = (r.sample[r.pos / 1024] * vol) >> 12;
                            r.pos += r.step;

                            int sL = sample;
                            int sR = sample;
                            distortion->Mix(r.efcCh, sL, sR);
                            chorus->Mix(r.efcCh, sL, sR);
                            hpflpf->Mix(r.efcCh, sL, sR);
                            compressor->Mix(r.efcCh, sL, sR);

                            sL = sL & maskl;
                            sR = sR & maskr;
                            sL *= reversePhase->Rhythm[i][0];
                            sR *= reversePhase->Rhythm[i][1];
                            int revSampleL = (int)(sL * reverb->SendLevel[r.efcCh]);
                            int revSampleR = (int)(sR * reverb->SendLevel[r.efcCh]);
                            fmvgen::StoreSample(buffer[0][dest], sL);
                            fmvgen::StoreSample(buffer[1][dest], sR);
                            reverb->StoreDataC(revSampleL, revSampleR);
                            //visRtmVolume[0] += sample & maskl;
                            //visRtmVolume[1] += sample & maskr;
                        }
                    }
                }
            }
        }

    protected:
        FM6 fm6[2];
        PSG2 psg2[4];
        ADPCMB adpcmb[3];
        ADPCMA adpcma;

        uint8_t prescale;

        void RebuildTimeTable()
        {
            OPNABase::RebuildTimeTable();

            int p = prescale;
            prescale = 0xff;//-1;
            SetPrescaler((uint32_t)p);
        }
    
    //public Dictionary<int, uint32_t[]> dicOpeWav = new Dictionary<int, uint32_t[]>();

};
