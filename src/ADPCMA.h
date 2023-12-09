#pragma once

#include "opna2.h"

class ADPCMA
{
    public:
        OPNA2* parent = NULL;

        class Channel
        {
            public:
                float panL;      // ぱん
                float panR;      // ぱん
                int8_t level;     // おんりょう
                int volume;     // おんりょうせってい
                uint32_t pos;       // いち
                uint32_t step;      // すてっぷち

                uint32_t start;     // 開始
                uint32_t stop;      // 終了
                uint32_t nibble;        // 次の 4 bit
                int16_t adpcmx;     // 変換用
                int16_t adpcmd;     // 変換用
        };

        Channel channel[6];

        uint8_t* buf;       // ADPCMA ROM
        int size;
        int8_t tl;      // ADPCMA 全体の音量
        int tvol;
        uint8_t key;        // ADPCMA のキー
        int step;
        uint8_t reg[32];
        static int16_t jedi_table[(48 + 1) * 16];

        ADPCMA(int num = 0, reverb* reverb = NULL, distortion* distortion = NULL, chorus* chorus = NULL, HPFLPF* hpflpf = NULL, ReversePhase* reversePhase = NULL, Compressor* compressor = NULL, int revStartCh = 0)
        {
            this->num = num;
            this->reversePhase = reversePhase;
            this->reverb = reverb;
            this->distortion = distortion;
            this->chorus = chorus;
            this->hpflpf = hpflpf;
            this->compressor = compressor;
            
            this->revStartCh = revStartCh;
            this->buf = NULL;
            this->size = 0;

            for (int i = 0; i < 6; i++)
            {
                channel[i].panL = 1.0f;
                channel[i].panR = 1.0f;
                channel[i].level = 0;
                channel[i].volume = 0;
                channel[i].pos = 0;
                channel[i].step = 0;
                channel[i].volume = 0;
                channel[i].start = 0;
                channel[i].stop = 0;
                channel[i].adpcmx = 0;
                channel[i].adpcmd = 0;
            }

            this->tl = 0;
            this->key = 0;
            this->tvol = 0;

            InitADPCMATable();
        }

        void InitADPCMATable()
        {
            for (int i = 0; i <= 48; i++)
            {
                int s = (int)(16.0 * pow(1.1, i) * 3);

                for (int j = 0; j < 16; j++)
                {
                    jedi_table[i * 16 + j] = (int16_t)(s * table2[j] / 8);
                }
            }
        }


        // ---------------------------------------------------------------------------
        //	ADPCMA 合成
        //
        void Mix(int** buffer, uint32_t count)
        {
            if (tvol < 128 && (key & 0x3f) != 0)
            {
                //Sample* limit = buffer + count * 2;
                uint32_t limit = count * 2;
                int revSampleL = 0;
                int revSampleR = 0;
                for (int i = 0; i < 6; i++)
                {
                    Channel r = channel[i];
                    if ((key & (1 << i)) != 0 && (uint8_t)r.level < 128)
                    {
                        //uint32_t maskl = (uint32_t)(r.panL == 0f ? -1 : 0);
                        //uint32_t maskr = (uint32_t)(r.panR == 0f ? -1 : 0);

                        int db = fmvgen::Limit(tl + tvol + r.level + r.volume, 127, -31);
                        int vol = OPNABase::tltable[FM_TLPOS + (db << (FM_TLBITS - 7))] >> 4;

                        //Sample* dest = buffer;
                        uint32_t dest = 0;
                        for (; dest < limit; dest += 2)
                        {
                            r.step += (uint32_t)step;
                            if (r.pos >= r.stop)
                            {
                                //SetStatus((uint32_t)(0x100 << i));
                                key &= (uint8_t)~(1 << i);
                                break;
                            }

                            for (; r.step > 0x10000; r.step -= 0x10000)
                            {
                                int data;
                                if ((r.pos & 1) == 0)
                                {
                                    r.nibble = buf[r.pos >> 1];
                                    data = (int)(r.nibble >> 4);
                                }
                                else
                                {
                                    data = (int)(r.nibble & 0x0f);
                                }
                                r.pos++;

                                r.adpcmx += jedi_table[r.adpcmd + data];
                                r.adpcmx = (int16_t)fmvgen::Limit(r.adpcmx, 2048 * 3 - 1, -2048 * 3);
                                r.adpcmd += (int16_t)decode_tableA1[data];
                                r.adpcmd = (int16_t)fmvgen::Limit(r.adpcmd, 48 * 16, 0);
                            }

                            int sampleL = (int)((r.adpcmx * vol) >> 10);
                            int sampleR = (int)((r.adpcmx * vol) >> 10);
                            distortion->Mix(revStartCh + i, sampleL, sampleR);
                            chorus->Mix(revStartCh + i, sampleL, sampleR);
                            hpflpf->Mix(revStartCh + i, sampleL, sampleR);
                            compressor->Mix(revStartCh + i, sampleL, sampleR);

                            sampleL = (int)(sampleL * r.panL) * reversePhase->AdpcmA[i][0];
                            sampleR = (int)(sampleR * r.panR) * reversePhase->AdpcmA[i][1];
                            fmvgen::StoreSample(buffer[0][dest], sampleL);
                            fmvgen::StoreSample(buffer[1][dest], sampleR);
                            revSampleL += (int)(sampleL * reverb->SendLevel[revStartCh + i] * 0.6);
                            revSampleR += (int)(sampleR * reverb->SendLevel[revStartCh + i] * 0.6);
                            //visRtmVolume[0] = (int)(sample & maskl);
                            //visRtmVolume[1] = (int)(sample & maskr);
                        }
                    }
                }
                
                reverb->StoreDataC(revSampleL, revSampleR);
            }
        }

        void SetReg(uint32_t adr, uint8_t data)
        {
            switch(adr)
            {
                case 0x00:         // DM/KEYON
                    if ((data & 0x80) == 0)  // KEY ON
                    {
                        key |= (uint8_t)(data & 0x3f);
                        for (int c = 0; c < 6; c++)
                        {
                            if ((data & (1 << c)) != 0)
                            {
                                //ResetStatus((uint32_t)(0x100 << c));
                                channel[c].pos = channel[c].start;
                                channel[c].step = 0;
                                channel[c].adpcmx = 0;
                                channel[c].adpcmd = 0;
                                channel[c].nibble = 0;
                            }
                        }
                    }
                    else
                    {                   // DUMP
                        key &= (uint8_t)~data;
                    }
                    break;

                case 0x01:
                    tl = (int8_t)(~data & 63);
                    break;

                case 0x02:
                    currentCh = data % 6;
                    currentIsLSB = true;
                    break;

                case 0x03:
                    channel[currentCh].level = (int8_t)(~data & 31);
                    break;

                case 0x04:
                    channel[currentCh].panL = OPNA2::panTable[((data >> 5) & 3) & 3] * ((data >> 7) & 1);
                    channel[currentCh].panR = OPNA2::panTable[((data >> 2) & 3) & 3] * ((data >> 4) & 1);
                    break;

                case 0x05:
                    if (currentIsLSB)
                    {
                        channel[currentCh].start = data;
                        currentIsLSB = false;
                    }
                    else
                    {
                        channel[currentCh].start |= (uint32_t)(data * 0x100);
                        channel[currentCh].start <<= 9;
                        currentIsLSB = true;
                    }
                    break;

                case 0x06:
                    if (currentIsLSB)
                    {
                        channel[currentCh].stop = data;
                        currentIsLSB = false;
                    }
                    else
                    {
                        channel[currentCh].stop |= (uint32_t)(data * 0x100);
                        channel[currentCh].stop <<= 9;
                        currentIsLSB = true;
                    }
                    break;

            }
        }

    private:
        reverb* reverb = NULL;
        distortion* distortion = NULL;
        chorus* chorus = NULL;
        HPFLPF* hpflpf = NULL;
        int revStartCh = 0;
        int num = 0;
        ReversePhase* reversePhase;
        Compressor* compressor;

        int8_t table2[16] = 
        {
                1,  3,  5,  7,  9, 11, 13, 15,
            -1, -3, -5, -7, -9,-11,-13,-15,
        };

        int decode_tableA1[16] =
        {
            -1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16,
            -1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16
        };

        int currentCh;
        bool currentIsLSB;
};
