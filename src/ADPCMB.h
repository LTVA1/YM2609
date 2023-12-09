#pragma once

#include "pantable_opna.h"
#include "macros.h"

class ADPCMB
{
    public:
        //OPNA2* parent = NULL;

        bool NO_BITTYPE_EMULATION = false;

        uint32_t stmask;
        uint32_t statusnext;

        uint8_t* adpcmbuf;        // ADPCM RAM
        uint32_t adpcmmask;     // メモリアドレスに対するビットマスク
        uint32_t adpcmnotice;   // ADPCM 再生終了時にたつビット
        uint8_t control2;     // ADPCM コントロールレジスタ２
        uint32_t memaddr;       // 再生中アドレス

        int adpcmlevel;     // ADPCM 音量
        int adpcmvolume;
        int adpcmvol;
        uint32_t deltan;            // ⊿N
        int adplc;          // 周波数変換用変数
        int adpld;          // 周波数変換用変数差分値
        uint32_t adplbase;      // adpld の元
        int adpcmx;         // ADPCM 合成用 x
        int adpcmd;         // ADPCM 合成用 ⊿
        int shiftBit = 6;    //メモリ

        bool adpcmmask_;
        bool adpcmplay;     // ADPCM 再生中

        ADPCMB(int num = 0, reverb* reverb = NULL, distortion* distortion = NULL, chorus* chorus = NULL, HPFLPF* hpflpf = NULL, ReversePhase* reversePhase = NULL, Compressor* compressor = NULL, int efcCh = 0)
        {
            this->num = num;
            this->reverb = reverb;
            this->distortion = distortion;
            this->chorus = chorus;
            this->hpflpf = hpflpf;
            this->reversePhase = reversePhase;
            this->compressor = compressor;
            this->efcCh = efcCh;
        }

        void Mix(int** dest, int count)
        {
            uint32_t maskl = (uint32_t)((control2 & 0x80) != 0 ? -1 : 0);
            uint32_t maskr = (uint32_t)((control2 & 0x40) != 0 ? -1 : 0);
            if (adpcmmask_)
            {
                maskl = maskr = 0;
            }

            if (adpcmplay)
            {
                int ptrDest = 0;
                //		LOG2("ADPCM Play: %d   DeltaN: %d\n", adpld, deltan);
                if (adpld <= 8192)      // fplay < fsamp
                {
                    for (; count > 0; count--)
                    {
                        if (adplc < 0)
                        {
                            adplc += 8192;
                            DecodeADPCMB();
                            if (!adpcmplay)
                                break;
                        }

                        int s = (adplc * apout0 + (8192 - adplc) * apout1) >> 13;
                        int sL = (int)(s & maskl);
                        int sR = (int)(s & maskr);
                        distortion->Mix(efcCh, sL, sR);
                        chorus->Mix(efcCh, sL, sR);
                        hpflpf->Mix(efcCh, sL, sR);
                        compressor->Mix(efcCh, sL, sR);

                        sL = (int)(sL * panL) * reversePhase->Adpcm[num][0];
                        sR = (int)(sR * panR) * reversePhase->Adpcm[num][1];
                        int revSampleL = (int)(sL * reverb->SendLevel[efcCh]);
                        int revSampleR = (int)(sR * reverb->SendLevel[efcCh]);
                        fmvgen::StoreSample(dest[0][ptrDest], sL);
                        fmvgen::StoreSample(dest[1][ptrDest], sR);
                        reverb->StoreDataC(revSampleL, revSampleR);
                        //visAPCMVolume[0] = (int)(s & maskl);
                        //visAPCMVolume[1] = (int)(s & maskr);
                        ptrDest += 2;
                        adplc -= adpld;
                    }
                    for (; count > 0 && apout0 != 0; count--)
                    {
                        if (adplc < 0)
                        {
                            apout0 = apout1;
                            apout1 = 0;
                            adplc += 8192;
                        }
                        int s = (adplc * apout1) >> 13;
                        int sL = (int)(s & maskl);
                        int sR = (int)(s & maskr);
                        distortion->Mix(efcCh, sL, sR);
                        chorus->Mix(efcCh, sL, sR);
                        hpflpf->Mix(efcCh, sL, sR);
                        compressor->Mix(efcCh, sL, sR);

                        sL = (int)(sL * panL) * reversePhase->Adpcm[num][0];
                        sR = (int)(sR * panR) * reversePhase->Adpcm[num][1];
                        int revSampleL = (int)(sL * reverb->SendLevel[efcCh]);
                        int revSampleR = (int)(sR * reverb->SendLevel[efcCh]);
                        fmvgen::StoreSample(dest[0][ptrDest], sL);
                        fmvgen::StoreSample(dest[1][ptrDest], sR);
                        reverb->StoreDataC(revSampleL, revSampleR);
                        //visAPCMVolume[0] = (int)(s & maskl);
                        //visAPCMVolume[1] = (int)(s & maskr);
                        ptrDest += 2;
                        adplc -= adpld;
                    }
                }
                else    // fplay > fsamp	(adpld = fplay/famp*8192)
                {
                    int t = (-8192 * 8192) / adpld;
                    for (; count > 0; count--)
                    {
                        int s = apout0 * (8192 + adplc);
                        while (adplc < 0)
                        {
                            DecodeADPCMB();
                            if (!adpcmplay)
                                goto stop;
                            s -= apout0 * my_max(adplc, t);
                            adplc -= t;
                        }
                        adplc -= 8192;
                        s >>= 13;
                        int sL = (int)(s & maskl);
                        int sR = (int)(s & maskr);
                        distortion->Mix(efcCh, sL, sR);
                        chorus->Mix(efcCh, sL, sR);
                        hpflpf->Mix(efcCh, sL, sR);
                        compressor->Mix(efcCh, sL, sR);

                        sL = (int)(sL * panL) * reversePhase->Adpcm[num][0];
                        sR = (int)(sR * panR) * reversePhase->Adpcm[num][1];
                        int revSampleL = (int)(sL * reverb->SendLevel[efcCh]);
                        int revSampleR = (int)(sR * reverb->SendLevel[efcCh]);
                        fmvgen::StoreSample(dest[0][ptrDest], sL);
                        fmvgen::StoreSample(dest[1][ptrDest], sR);
                        reverb->StoreDataC(revSampleL, revSampleR);
                        //visAPCMVolume[0] = (int)(s & maskl);
                        //visAPCMVolume[1] = (int)(s & maskr);
                        ptrDest += 2;
                    }
                stop:
                    ;
                }
            }
            if (!adpcmplay)
            {
                apout0 = apout1 = adpcmout = 0;
                adplc = 0;
            }
        }

        void SetADPCMBReg(uint32_t addr, uint32_t data)
        {
            switch (addr)
            {
                case 0x00:      // Control Register 1
                    if (((data & 0x80) != 0) && !adpcmplay)
                    {
                        adpcmplay = true;
                        memaddr = startaddr;
                        adpcmx = 0;
                        adpcmd = 127;
                        adplc = 0;
                    }
                    if ((data & 1) != 0)
                    {
                        adpcmplay = false;
                    }
                    control1 = (uint8_t)data;
                    break;

                case 0x01:      // Control Register 2
                    control2 = (uint8_t)data;
                    granuality = (int8_t)((control2 & 2) != 0 ? 1 : 4);
                    break;

                case 0x02:      // Start Address L
                case 0x03:      // Start Address H
                    adpcmreg[addr - 0x02 + 0] = (uint8_t)data;
                    startaddr = (uint32_t)((adpcmreg[1] * 256 + adpcmreg[0]) << shiftBit);
                    memaddr = startaddr;
                    //		LOG1("  startaddr %.6x", startaddr);
                    break;

                case 0x04:      // Stop Address L
                case 0x05:      // Stop Address H
                    adpcmreg[addr - 0x04 + 2] = (uint8_t)data;
                    stopaddr = (uint32_t)((adpcmreg[3] * 256 + adpcmreg[2] + 1) << shiftBit);
                    //		LOG1("  stopaddr %.6x", stopaddr);
                    break;

                case 0x07:
                    panL = panTable_opna[(data >> 6) & 0x3];
                    panR = panTable_opna[(data >> 4) & 0x3];
                    break;

                case 0x08:      // ADPCM data
                    if ((control1 & 0x60) == 0x60)
                    {
                        //			LOG2("  Wr [0x%.5x] = %.2x", memaddr, data);
                        WriteRAM(data);
                    }
                    break;

                case 0x09:      // delta-N L
                case 0x0a:      // delta-N H
                    adpcmreg[addr - 0x09 + 4] = (uint8_t)data;
                    deltan = (uint32_t)(adpcmreg[5] * 256 + adpcmreg[4]);
                    deltan = my_max(256, deltan);
                    adpld = (int)(deltan * adplbase >> 16);
                    break;

                case 0x0b:      // Level Control
                    adpcmlevel = (uint8_t)data;
                    adpcmvolume = (adpcmvol * adpcmlevel) >> 12;
                    break;

                case 0x0c:      // Limit Address L
                case 0x0d:      // Limit Address H
                    adpcmreg[addr - 0x0c + 6] = (uint8_t)data;
                    limitaddr = (uint32_t)((adpcmreg[7] * 256 + adpcmreg[6] + 1) << shiftBit);
                    //		LOG1("  limitaddr %.6x", limitaddr);
                    break;

                case 0x10:      // Flag Control
                    if ((data & 0x80) != 0)
                    {
                        status = 0;
                        UpdateStatus();
                    }
                    else
                    {
                        stmask = ~(data & 0x1f);
                        //			UpdateStatus();					//???
                    }
                    break;
            }
        }

    protected:
        uint32_t startaddr;     // Start address
        uint32_t stopaddr;      // Stop address
        uint32_t limitaddr;     // Limit address/mask
        int adpcmout;       // ADPCM 合成後の出力
        int apout0;         // out(t-2)+out(t-1)
        int apout1;         // out(t-1)+out(t)
        uint32_t status;
        uint32_t adpcmreadbuf;  // ADPCM リード用バッファ
        int8_t granuality;
        uint8_t control1;     // ADPCM コントロールレジスタ１
        uint8_t adpcmreg[8];  // ADPCM レジスタの一部分
        //protected float[] panTable = new float[4] { 1.0f, 0.5012f, 0.2512f, 0.1000f };
        float panL = 1.0f;
        float panR = 1.0f;

        // ---------------------------------------------------------------------------
        //	ADPCM RAM への書込み操作
        //
        void WriteRAM(uint32_t data)
        {
            if (NO_BITTYPE_EMULATION)
            {
                if ((control2 & 2) == 0)
                {
                    // 1 bit mode
                    //adpcmbuf[(memaddr >> 4) & 0x3ffff] = (uint8_t)data;
                    adpcmbuf[(memaddr >> 4) & adpcmmask] = (uint8_t)data;
                    memaddr += 16;
                }
                else
                {
                    // 8 bit mode
                    //uint8* p = &adpcmbuf[(memaddr >> 4) & 0x7fff];
                    int p = (int)((memaddr >> 4) & 0x7fff);
                    uint32_t bank = (memaddr >> 1) & 7;
                    uint8_t mask = (uint8_t)(1 << (int)bank);
                    data <<= (int)bank;

                    adpcmbuf[p + 0x00000] = (uint8_t)((adpcmbuf[p + 0x00000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x08000] = (uint8_t)((adpcmbuf[p + 0x08000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x10000] = (uint8_t)((adpcmbuf[p + 0x10000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x18000] = (uint8_t)((adpcmbuf[p + 0x18000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x20000] = (uint8_t)((adpcmbuf[p + 0x20000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x28000] = (uint8_t)((adpcmbuf[p + 0x28000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x30000] = (uint8_t)((adpcmbuf[p + 0x30000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p + 0x38000] = (uint8_t)((adpcmbuf[p + 0x38000] & ~mask) | ((uint8_t)(data) & mask));
                    memaddr += 2;
                }
            }
            else
            {
                //adpcmbuf[(memaddr >> granuality) & 0x3ffff] = (uint8_t)data;
                adpcmbuf[(memaddr >> granuality) & adpcmmask] = (uint8_t)data;
                memaddr += (uint32_t)(1 << granuality);
            }

            if (memaddr == stopaddr)
            {
                SetStatus(4);
                statusnext = 0x04;  // EOS
                memaddr &= (uint32_t)((shiftBit == 6) ? 0x3fffff : 0x1ffffff);
            }
            if (memaddr == limitaddr)
            {
                //		LOG1("Limit ! (%.8x)\n", limitaddr);
                memaddr = 0;
            }
            SetStatus(8);
        }

        // ---------------------------------------------------------------------------
        //	ADPCM 展開
        //
        void DecodeADPCMB()
        {
            apout0 = apout1;
            int n = (ReadRAMN() * adpcmvolume) >> 13;
            apout1 = adpcmout + n;
            adpcmout = n;
        }

        // ---------------------------------------------------------------------------
        //	ADPCM RAM からの nibble 読み込み及び ADPCM 展開
        //
        int ReadRAMN()
        {
            uint32_t data;
            if (granuality > 0)
            {
                if (NO_BITTYPE_EMULATION)
                {
                    if ((control2 & 2) == 0)
                    {
                        //data = adpcmbuf[(memaddr >> 4) & 0x3ffff];
                        data = adpcmbuf[(memaddr >> 4) & adpcmmask];
                        memaddr += 8;
                        if ((memaddr & 8) != 0)
                            return DecodeADPCMBSample(data >> 4);
                        data &= 0x0f;
                    }
                    else
                    {
                        //uint8* p = &adpcmbuf[(memaddr >> 4) & 0x7fff] + ((~memaddr & 1) << 17);
                        int p = (int)(((memaddr >> 4) & 0x7fff) + ((~memaddr & 1) << 17));
                        uint32_t bank = (memaddr >> 1) & 7;
                        uint8_t mask = (uint8_t)(1 << (int)bank);

                        data = (uint32_t)(adpcmbuf[p + 0x18000] & mask);
                        data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x10000] & mask));
                        data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x08000] & mask));
                        data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x00000] & mask));
                        data >>= (int)bank;
                        memaddr++;
                        if ((memaddr & 1) != 0)
                            return DecodeADPCMBSample(data);
                    }
                }
                else
                {
                    data = adpcmbuf[(memaddr >> granuality) & adpcmmask];
                    memaddr += (uint32_t)(1 << (granuality - 1));
                    if ((memaddr & (1 << (granuality - 1))) != 0)
                        return DecodeADPCMBSample(data >> 4);
                    data &= 0x0f;
                }
            }
            else
            {
                data = adpcmbuf[(memaddr >> 1) & adpcmmask];
                ++memaddr;
                if ((memaddr & 1) != 0)
                    return DecodeADPCMBSample(data >> 4);
                data &= 0x0f;
            }

            DecodeADPCMBSample(data);

            // check
            if (memaddr == stopaddr)
            {
                if ((control1 & 0x10) != 0)
                {
                    memaddr = startaddr;
                    data = (uint32_t)adpcmx;
                    adpcmx = 0;
                    adpcmd = 127;
                    return (int)data;
                }
                else
                {
                    memaddr &= adpcmmask;   //0x3fffff;
                    SetStatus(adpcmnotice);
                    adpcmplay = false;
                }
            }

            if (memaddr == limitaddr)
                memaddr = 0;

            return adpcmx;
        }

        int table1[16] =
        {
            1,   3,   5,   7,   9,  11,  13,  15,
            -1,  -3,  -5,  -7,  -9, -11, -13, -15,
        };

        int table2[16] =
        {
            57,  57,  57,  57,  77, 102, 128, 153,
            57,  57,  57,  57,  77, 102, 128, 153,
        };

        int DecodeADPCMBSample(uint32_t data)
        {
            adpcmx = fmvgen::Limit(adpcmx + table1[data] * adpcmd / 8, 32767, -32768);
            adpcmd = fmvgen::Limit(adpcmd * table2[data] / 64, 24576, 127);
            return adpcmx;
        }

        // ---------------------------------------------------------------------------
        //	ステータスフラグ設定
        //
        void SetStatus(uint32_t bits)
        {
            if ((status & bits) == 0)
            {
                //		LOG2("SetStatus(%.2x %.2x)\n", bits, stmask);
                status |= bits & stmask;
                UpdateStatus();
            }
            //	else
            //		LOG1("SetStatus(%.2x) - ignored\n", bits);
        }

        void ResetStatus(uint32_t bits)
        {
            status &= ~bits;
            //	LOG1("ResetStatus(%.2x)\n", bits);
            UpdateStatus();
        }

        void UpdateStatus()
        {
            //	LOG2("%d:INT = %d\n", Diag::GetCPUTick(), (status & stmask & reg29) != 0);
            //Intr((status & stmask & reg29) != 0);
        }

    private:
        reverb* reverb;
        distortion* distortion;
        chorus* chorus;
        HPFLPF* hpflpf;
        int efcCh;
        int num;
        ReversePhase* reversePhase;
        Compressor* compressor;

        void Intr(bool f)
        {
        }

};
