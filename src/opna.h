#pragma once
#include "fmgen.h"
#include "Timer.h"
#include "psg.h"
#include "psg2.h"

// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

// ---------------------------------------------------------------------------
//	class OPN/OPNA
//	OPN/OPNA に良く似た音を生成する音源ユニット
//	
//	interface:
//	bool Init(uint32_t clock, uint32_t rate, bool, const char* path);
//		初期化．このクラスを使用する前にかならず呼んでおくこと．
//		OPNA の場合はこの関数でリズムサンプルを読み込む
//
//		clock:	OPN/OPNA/OPNB のクロック周波数(Hz)
//
//		rate:	生成する PCM の標本周波数(Hz)
//
//		path:	リズムサンプルのパス(OPNA のみ有効)
//				省略時はカレントディレクトリから読み込む
//				文字列の末尾には '\' や '/' などをつけること
//
//		返り値	初期化に成功すれば true
//
//	bool LoadRhythmSample(const char* path)
//		(OPNA ONLY)
//		Rhythm サンプルを読み直す．
//		path は Init の path と同じ．
//		
//	bool SetRate(uint32_t clock, uint32_t rate, bool)
//		クロックや PCM レートを変更する
//		引数等は Init を参照のこと．
//	
//	void Mix(FM_SAMPLETYPE* dest, int nsamples)
//		Stereo PCM データを nsamples 分合成し， dest で始まる配列に
//		加える(加算する)
//		・dest には sample*2 個分の領域が必要
//		・格納形式は L, R, L, R... となる．
//		・あくまで加算なので，あらかじめ配列をゼロクリアする必要がある
//		・FM_SAMPLETYPE が short 型の場合クリッピングが行われる.
//		・この関数は音源内部のタイマーとは独立している．
//		  Timer は Count と GetNextEvent で操作する必要がある．
//	
//	void Reset()
//		音源をリセット(初期化)する
//
//	void SetReg(uint32_t reg, uint32_t data)
//		音源のレジスタ reg に data を書き込む
//	
//	uint32_t GetReg(uint32_t reg)
//		音源のレジスタ reg の内容を読み出す
//		読み込むことが出来るレジスタは PSG, ADPCM の一部，ID(0xff) とか
//	
//	uint32_t ReadStatus()/ReadStatusEx()
//		音源のステータスレジスタを読み出す
//		ReadStatusEx は拡張ステータスレジスタの読み出し(OPNA)
//		busy フラグは常に 0
//	
//	bool Count(uint32 t)
//		音源のタイマーを t [μ秒] 進める．
//		音源の内部状態に変化があった時(timer オーバーフロー)
//		true を返す
//
//	uint32 GetNextEvent()
//		音源のタイマーのどちらかがオーバーフローするまでに必要な
//		時間[μ秒]を返す
//		タイマーが停止している場合は ULONG_MAX を返す… と思う
//	
//	void SetVolumeFM(int db)/SetVolumePSG(int db) ...
//		各音源の音量を＋－方向に調節する．標準値は 0.
//		単位は約 1/2 dB，有効範囲の上限は 20 (10dB)
//

//	OPN Base -------------------------------------------------------
class OPNBase : public Timer
{
    public:
        fmgen::Chip chip;
        
        OPNBase()
        {
            prescale = 0;
            psg = PSG();
            chip = Chip();
        }

        void SetPrescaler(uint32_t p)
        {
            int8_t table[3][2] = {{ 6, 4 }, { 3, 2 }, { 2, 1 } };
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
                chip.SetRatio(ratio);
                psg.SetClock((int)(clock / table[p][1]), (int)psgrate);

                for (int i = 0; i < 8; i++)
                {
                    lfotable[i] = (ratio << (2 + FM_LFOCBITS - FM_RATIOBITS)) / table2[i];
                }
            }
        }

        //	初期化
        bool Init(uint32_t c, uint32_t r)
        {
            clock = c;
            psgrate = r;

            return true;
        }

        void Reset() override
        {
            status = 0;
            SetPrescaler(0);
            Timer::Reset();
            psg.Reset();
        }

        void ChangePSGMode(int mode)
        {
            if (mode == 0)
            {
                psg = PSG();
            }
            else
            {
                psg = PSG2();
            }
            int8_t* table = new int8_t[3] { 4, 2, 1 };
            psg.SetClock((int)(clock / table[prescale]), (int)psgrate);
            psg.Reset();
            psg.SetVolume(psg_db);
            delete[] table;
        }

        //	音量設定
        void SetVolumeFM(int db)
        {
            db = std::min(db, 20);
            if (db > -192)
                fmvolume = (int)(16384.0 * pow(10.0, db / 40.0));
            else
                fmvolume = 0;
        }

        void SetVolumePSG(int db)
        {
            psg.SetVolume(db);
            psg_db = db;
        }

        void SetLPFCutoff(uint32_t freq)
        {
        }    // obsolete

        int visVolume[2] = { 0, 0 };

        PSG psg;
        int psg_db = 0;

    protected:
        void SetParameter(fmgen::Channel4 ch, uint32_t addr, uint32_t data)
        {
            uint32_t slottable[4] = { 0, 2, 1, 3 };
            uint8_t sltable[16] = {
                0,   4,   8,  12,  16,  20,  24,  28,
                32,  36,  40,  44,  48,  52,  56, 124
            };

            if ((addr & 3) < 3)
            {
                uint32_t slot = slottable[(addr >> 2) & 3];
                fmgen::Operator op = ch.op[slot];

                switch ((addr >> 4) & 15)
                {
                    case 3: // 30-3E DT/MULTI
                        op.SetDT((data >> 4) & 0x07);
                        op.SetMULTI(data & 0x0f);
                        break;

                    case 4: // 40-4E TL
                        op.SetTL(data & 0x7f, ((regtc & 0x80)!=0) && (csmch == ch));
                        break;

                    case 5: // 50-5E KS/AR
                        op.SetKS((data >> 6) & 3);
                        op.SetAR((data & 0x1f) * 2);
                        break;

                    case 6: // 60-6E DR/AMON
                        op.SetDR((data & 0x1f) * 2);
                        op.SetAMON((data & 0x80) != 0);
                        break;

                    case 7: // 70-7E SR
                        op.SetSR((data & 0x1f) * 2);
                        break;

                    case 8: // 80-8E SL/RR
                        op.SetSL(sltable[(data >> 4) & 15]);
                        op.SetRR((data & 0x0f) * 4 + 2);
                        break;

                    case 9: // 90-9E SSG-EC
                        op.SetSSGEC(data & 0x0f);
                        break;
                }
            }
        }

        void RebuildTimeTable()
        {
            int p = prescale;
            prescale = 0xff;//-1;
            SetPrescaler((uint32_t)p);
        }



        int fmvolume;

        uint32_t clock;             // OPN クロック
        uint32_t rate;              // FM 音源合成レート
        uint32_t psgrate;           // FMGen  出力レート
        uint32_t status;
        fmgen::Channel4 csmch;

        uint32_t lfotable[8];
        uint8_t prescale;

    //	タイマー時間処理
    private:
        void TimerA() //TODO:: add REAL CSM mode functionality (reset envelope too)
        {
            if ((regtc & 0x80)!=0)
            {
                csmch.KeyControl(0x00);
                csmch.KeyControl(0x0f);
            }
        }
};

//	OPN2 Base ------------------------------------------------------
class OPNABase : public OPNBase
{
    public:
        bool NO_BITTYPE_EMULATION = false;

        static int amtable[FM_LFOENTS];
        static int pmtable[FM_LFOENTS];
        static int tltable[FM_TLENTS + FM_TLPOS];

        OPNABase()
        {
            amtable[0] = -1;
            tablehasmade = false;

            adpcmbuf = NULL;
            memaddr = 0;
            startaddr = 0;
            deltan = 256;

            adpcmvol = 0;
            control2 = 0;

            MakeTable2();
            BuildLFOTable();

            for (int i = 0; i < 6; i++)
            {
                ch[i] = new fmgen.Channel4();
                ch[i].SetChip(chip);
                ch[i].SetType(fmgen.OpType.typeN);
            }
        }

        ~OPNABase()
        {
        }

        uint32_t ReadStatus()
        {
            return status & 0x03;
        }

        // ---------------------------------------------------------------------------
        //	拡張ステータスを読みこむ
        //
        uint32_t ReadStatusEx()
        {
            uint32_t a = status | 8;
            uint32_t b = a & stmask;
            uint32_t c = (uint32_t)(adpcmplay ? 0x20 : 0);

            uint32_t r = b | c;
            status |= statusnext;
            statusnext = 0;
            return r;
        }

        // ---------------------------------------------------------------------------
        //	チャンネルマスクの設定
        //
        void SetChannelMask(uint32_t mask)
        {
            for (int i = 0; i < 6; i++)
                ch[i].Mute(!((mask & (1 << i)) == 0));
            psg.SetChannelMask((int)(mask >> 6));
            adpcmmask_ = (mask & (1 << 9)) != 0;
            rhythmmask_ = (int)((mask >> 10) & ((1 << 6) - 1));
        }

    private:
        void Intr(bool f)
        {
        }

        // ---------------------------------------------------------------------------
        //	テーブル作成
        //
        void MakeTable2()
        {
            if (!tablehasmade)
            {
                for (int i = -FM_TLPOS; i < FM_TLENTS; i++)
                {
                    tltable[i + FM_TLPOS] = (int)((uint32_t)(65536.0 * pow(2.0, i * -16.0 / FM_TLENTS))) - 1;
                }

                tablehasmade = true;
            }
        }

    // ---------------------------------------------------------------------------
    //	初期化
    //
    protected:
        bool Init(uint32_t c, uint32_t r, bool f)
        {
            RebuildTimeTable();

            Reset();

            SetVolumeFM(0);
            SetVolumePSG(0);
            SetChannelMask(0);
            return true;
        }

        // ---------------------------------------------------------------------------
        //	サンプリングレート変更
        //
        bool SetRate(uint32_t c, uint32_t r, bool f)
        {
            c /= 2;     // 従来版との互換性を重視したけりゃコメントアウトしよう

            Timer::Init(c, r);

            adplbase = (uint32_t)((int)(8192.0 * (clock / 72.0) / r));
            adpld = (int)(deltan * adplbase >> 16);

            RebuildTimeTable();

            lfodcount = (reg22 & 0x08) != 0 ? lfotable[reg22 & 7] : 0;
            return true;
        }

        // ---------------------------------------------------------------------------
        //	リセット
        //
        void Reset()
        {
            uint32_t i;

            Timer::Reset();
            for (i = 0x20; i < 0x28; i++) SetReg(i, 0);
            for (i = 0x30; i < 0xc0; i++) SetReg(i, 0);
            for (i = 0x130; i < 0x1c0; i++) SetReg(i, 0);
            for (i = 0x100; i < 0x110; i++) SetReg(i, 0);
            for (i = 0x10; i < 0x20; i++) SetReg(i, 0);
            for (i = 0; i < 6; i++)
            {
                pan[i] = 3;
                ch[i].Reset();
            }

            stmask = 0x73;// ~0x1c;
            statusnext = 0;
            memaddr = 0;
            adpcmd = 127;
            adpcmx = 0;
            lfocount = 0;
            adpcmplay = false;
            adplc = 0;
            adpld = 0x100;
            status = 0;
            UpdateStatus();
        }

        // ---------------------------------------------------------------------------
        //	レジスタアレイにデータを設定
        //
        void SetReg(uint32_t addr, uint32_t data)
        {
            int c = (int)(addr & 3);
            uint32_t modified;

            switch (addr)
            {

                // Timer -----------------------------------------------------------------
                case 0x24:
                case 0x25:
                    SetTimerA(addr, data);
                    break;

                case 0x26:
                    SetTimerB(data);
                    break;

                case 0x27:
                    SetTimerControl(data);
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

                // Prescaler -------------------------------------------------------------
                case 0x2d:
                case 0x2e:
                case 0x2f:
                    SetPrescaler(addr - 0x2d);
                    break;

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
                    break;
                case 0xa4:
                case 0xa5:
                case 0xa6:
                    fnum2[c] = (uint8_t)(data);
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

                // Algorithm -------------------------------------------------------------

                case 0x1b0:
                case 0x1b1:
                case 0x1b2:
                    c += 3;
                    ch[c].SetFB((data >> 3) & 7);
                    ch[c].SetAlgorithm(data & 7);
                    break;
                case 0xb0:
                case 0xb1:
                case 0xb2:
                    ch[c].SetFB((data >> 3) & 7);
                    ch[c].SetAlgorithm(data & 7);
                    break;

                case 0x1b4:
                case 0x1b5:
                case 0x1b6:
                    c += 3;
                    pan[c] = (uint8_t)((data >> 6) & 3);
                    ch[c].SetMS(data);
                    break;
                case 0xb4:
                case 0xb5:
                case 0xb6:
                    pan[c] = (uint8_t)((data >> 6) & 3);
                    ch[c].SetMS(data);
                    break;

                // LFO -------------------------------------------------------------------
                case 0x22:
                    modified = reg22 ^ data;
                    reg22 = (uint8_t)data;
                    if ((modified & 0x8) != 0)
                        lfocount = 0;
                    lfodcount = (reg22 & 8) != 0 ? lfotable[reg22 & 7] : 0;
                    break;

                // PSG -------------------------------------------------------------------
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                    psg.SetReg(addr, (uint8_t)data);
                    break;

                // 音色 ------------------------------------------------------------------
                default:
                    if (c < 3)
                    {
                        if ((addr & 0x100) != 0)
                            c += 3;
                        base.SetParameter(ch[c], addr, data);
                    }
                    break;
            }
        }

        // ---------------------------------------------------------------------------
        //	ADPCM B
        //
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
                    startaddr = (uint32_t)((adpcmreg[1] * 256 + adpcmreg[0]) << 6);
                    if ((control1 & 0x40) != 0)
                    {
                        memaddr = startaddr;
                    }
                    //		LOG1("  startaddr %.6x", startaddr);
                    break;

                case 0x04:      // Stop Address L
                case 0x05:      // Stop Address H
                    adpcmreg[addr - 0x04 + 2] = (uint8_t)data;
                    stopaddr = (uint32_t)((adpcmreg[3] * 256 + adpcmreg[2] + 1) << 6);
                    //		LOG1("  stopaddr %.6x", stopaddr);
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
                    deltan = Math.Max(256, deltan);
                    adpld = (int)(deltan * adplbase >> 16);
                    break;

                case 0x0b:      // Level Control
                    adpcmlevel = (uint8_t)data;
                    adpcmvolume = (adpcmvol * adpcmlevel) >> 12;
                    break;

                case 0x0c:      // Limit Address L
                case 0x0d:      // Limit Address H
                    adpcmreg[addr - 0x0c + 6] = (uint8_t)data;
                    limitaddr = (uint32_t)((adpcmreg[7] * 256 + adpcmreg[6] + 1) << 6);
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

        // ---------------------------------------------------------------------------
        //	レジスタ取得
        //
        uint32_t GetReg(uint32_t addr)
        {
            if (addr < 0x10)
                return psg.GetReg(addr);

            if (addr == 0x108)
            {
                //		LOG1("%d:reg[108] ->   ", Diag::GetCPUTick());

                uint32_t data = adpcmreadbuf & 0xff;
                adpcmreadbuf >>= 8;
                if ((control1 & 0x60) == 0x20)
                {
                    adpcmreadbuf |= ReadRAM() << 8;
                    //			LOG2("Rd [0x%.6x:%.2x] ", memaddr, adpcmreadbuf >> 8);
                }
                //		LOG0("%.2x\n");
                return data;
            }

            if (addr == 0xff)
                return 1;

            return 0;
        }

        // ---------------------------------------------------------------------------
        //	合成
        //	in:		buffer		合成先
        //			nsamples	合成サンプル数
        //
        void FMMix(int** buffer, int nsamples)
        {
            if (fmvolume > 0)
            {
                // 準備
                // Set F-Number
                if ((regtc & 0xc0) == 0)
                    csmch.SetFNum(fnum[2]);// csmch - ch]);
                else
                {
                    // 効果音モード
                    csmch.op[0].SetFNum(fnum3[1]);
                    csmch.op[1].SetFNum(fnum3[2]);
                    csmch.op[2].SetFNum(fnum3[0]);
                    csmch.op[3].SetFNum(fnum[2]);
                }

                int act = (((ch[2].Prepare() << 2) | ch[1].Prepare()) << 2) | ch[0].Prepare();
                if ((reg29 & 0x80)!=0)
                    act |= (ch[3].Prepare() | ((ch[4].Prepare() | (ch[5].Prepare() << 2)) << 2)) << 6;
                if ((reg22 & 0x08)==0)
                    act &= 0x555;

                if ((act & 0x555)!=0)
                {
                    Mix6(buffer, nsamples, act);
                }
            }
        }

        void Mix6(int** buffer, int nsamples, int activech)
        {
            // Mix
            int ibuf[4];
            int idest[6];
            idest[0] = pan[0];
            idest[1] = pan[1];
            idest[2] = pan[2];
            idest[3] = pan[3];
            idest[4] = pan[4];
            idest[5] = pan[5];

            int limit = nsamples * 2;
            for (int dest = 0; dest < limit; dest += 2)
            {
                ibuf[1] = ibuf[2] = ibuf[3] = 0;
                if ((activech & 0xaaa) != 0)
                {
                    LFO();
                    MixSubSL(activech, idest, ibuf);
                }
                else
                {
                    MixSubS(activech, idest, ibuf);
                }

                int v = ((fmgen::Limit(ibuf[2] + ibuf[3], 0x7fff, -0x8000) * fmvolume) >> 14);
                //int v = fmgen.Limit((ibuf[2] + ibuf[3]) , 0x7fff, -0x8000);
                fmgen::StoreSample(buffer[0][dest], v);// ((fmgen.Limit(ibuf[2] + ibuf[3], 0x7fff, -0x8000) * fmvolume) >> 14));
                visVolume[0] = v;

                v = ((fmgen::Limit(ibuf[1] + ibuf[3], 0x7fff, -0x8000) * fmvolume) >> 14);
                //v = fmgen.Limit((ibuf[1] + ibuf[3]) , 0x7fff, -0x8000);
                fmgen::StoreSample(buffer[1][dest], v);// ((fmgen.Limit(ibuf[1] + ibuf[3], 0x7fff, -0x8000) * fmvolume) >> 14));
                visVolume[1] = v;
            }
        }

        void MixSubS(int activech, int* dest, int* buf)
        {
            if ((activech & 0x001) != 0) buf[dest[0]] = ch[0].Calc();
            if ((activech & 0x004) != 0) buf[dest[1]] += ch[1].Calc();
            if ((activech & 0x010) != 0) buf[dest[2]] += ch[2].Calc();
            if ((activech & 0x040) != 0) buf[dest[3]] += ch[3].Calc();
            if ((activech & 0x100) != 0) buf[dest[4]] += ch[4].Calc();
            if ((activech & 0x400) != 0) buf[dest[5]] += ch[5].Calc();
        }

        void MixSubSL(int activech, int* dest, int* buf)
        {
            if ((activech & 0x001) != 0) buf[dest[0]] = ch[0].CalcL();
            if ((activech & 0x004) != 0) buf[dest[1]] += ch[1].CalcL();
            if ((activech & 0x010) != 0) buf[dest[2]] += ch[2].CalcL();
            if ((activech & 0x040) != 0) buf[dest[3]] += ch[3].CalcL();
            if ((activech & 0x100) != 0) buf[dest[4]] += ch[4].CalcL();
            if ((activech & 0x400) != 0) buf[dest[5]] += ch[5].CalcL();
        }

        // ---------------------------------------------------------------------------
        //	ステータスフラグ設定
        //
        void SetStatus(uint32_t bits)
        {
            if ((status & bits)==0)
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
            Intr((status & stmask & reg29) != 0);
        }

        void LFO()
        {
            //	LOG3("%4d - %8d, %8d\n", c, lfocount, lfodcount);

            //	Operator::SetPML(pmtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
            //	Operator::SetAML(amtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
            chip.SetPML((uint32_t)(pmtable[(lfocount >> (FM_LFOCBITS + 1)) & 0xff]));
            chip.SetAML((uint32_t)(amtable[(lfocount >> (FM_LFOCBITS + 1)) & 0xff]));
            lfocount += lfodcount;
        }

        static void BuildLFOTable()
        {
            if (amtable[0] == -1)
            {
                for (int c = 0; c < 256; c++)
                {
                    int v;
                    if (c < 0x40) v = c * 2 + 0x80;
                    else if (c < 0xc0) v = 0x7f - (c - 0x40) * 2 + 0x80;
                    else v = (c - 0xc0) * 2;
                    pmtable[c] = c;

                    if (c < 0x80) v = 0xff - c * 2;
                    else v = (c - 0x80) * 2;
                    amtable[c] = v & ~3;
                }
            }
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
        //	ADPCM 合成
        //	
        void ADPCMBMix(int** dest, uint32_t count)
        {
            uint32_t maskl = (uint32_t)((control2 & 0x80)!=0 ? -1 : 0);
            uint32_t maskr = (uint32_t)((control2 & 0x40)!=0 ? -1 : 0);
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
                        fmgen::StoreSample(dest[0][ptrDest], (int)(s & maskl));
                        fmgen::StoreSample(dest[1][ptrDest], (int)(s & maskr));
                        //visAPCMVolume[0] = (int)(s & maskl);
                        //visAPCMVolume[1] = (int)(s & maskr);
                        ptrDest += 2;
                        adplc -= adpld;
                    }
                    for (; count > 0 && apout0!=0; count--)
                    {
                        if (adplc < 0)
                        {
                            apout0 = apout1;
                            apout1 = 0;
                            adplc += 8192;
                        }
                        int s = (adplc * apout1) >> 13;
                        fmgen::StoreSample(dest[0][ptrDest], (int)(s & maskl));
                        fmgen::StoreSample(dest[1][ptrDest], (int)(s & maskr));
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
                            s -= apout0 * Math.Max(adplc, t);
                            adplc -= t;
                        }
                        adplc -= 8192;
                        s >>= 13;
                        fmgen::StoreSample(dest[0][ptrDest], (int)(s & maskl));
                        fmgen::StoreSample(dest[1][ptrDest], (int)(s & maskr));
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

        // ---------------------------------------------------------------------------
        //	ADPCM RAM への書込み操作
        //
        void WriteRAM(uint32_t data)
        {
            if (NO_BITTYPE_EMULATION) {
                if ((control2 & 2)==0)
                {
                    // 1 bit mode
                    adpcmbuf[(memaddr >> 4) & 0x3ffff] = (uint8_t)data;
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
                    adpcmbuf[p+0x08000] = (uint8_t)((adpcmbuf[p+0x08000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x10000] = (uint8_t)((adpcmbuf[p+0x10000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x18000] = (uint8_t)((adpcmbuf[p+0x18000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x20000] = (uint8_t)((adpcmbuf[p+0x20000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x28000] = (uint8_t)((adpcmbuf[p+0x28000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x30000] = (uint8_t)((adpcmbuf[p+0x30000] & ~mask) | ((uint8_t)(data) & mask));
                    data >>= 1;
                    adpcmbuf[p+0x38000] = (uint8_t)((adpcmbuf[p+0x38000] & ~mask) | ((uint8_t)(data) & mask));
                    memaddr += 2;
                }
            } else {
                adpcmbuf[(memaddr >> granuality) & 0x3ffff] = (uint8_t)data;
                memaddr += (uint32_t)(1 << granuality);
            }

            if (memaddr == stopaddr)
            {
                SetStatus(4);
                statusnext = 0x04;  // EOS
                memaddr &= 0x3fffff;
            }
            if (memaddr == limitaddr)
            {
                //		LOG1("Limit ! (%.8x)\n", limitaddr);
                memaddr = 0;
            }
            SetStatus(8);
        }

        // ---------------------------------------------------------------------------
        //	ADPCM RAM からの読み込み操作
        //
        uint32_t ReadRAM()
        {
            uint32_t data;
            if (NO_BITTYPE_EMULATION) {
                if ((control2 & 2)==0)
                {
                    // 1 bit mode
                    data = adpcmbuf[(memaddr >> 4) & 0x3ffff];
                    memaddr += 16;
                }
                else
                {
                    // 8 bit mode
                    //uint8* p = &adpcmbuf[(memaddr >> 4) & 0x7fff];
                    int p = (int)((memaddr >> 4) & 0x7fff);
                    uint32_t bank = (memaddr >> 1) & 7;
                    uint8_t mask = (uint8_t)(1 << (int)bank);

                    data = (uint32_t)(adpcmbuf[p+0x38000] & mask);
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x30000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x28000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x20000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x18000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x10000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x08000] & mask));
                    data = (uint32_t)(data * 2 + (adpcmbuf[p + 0x00000] & mask));
                    data >>= (int)bank;
                    memaddr += 2;
                }
            } else {
                data = adpcmbuf[(memaddr >> granuality) & 0x3ffff];
                memaddr += (uint32_t)(1 << granuality);
            }

            if (memaddr == stopaddr)
            {
                SetStatus(4);
                statusnext = 0x04;  // EOS
                memaddr &= 0x3fffff;
            }
            if (memaddr == limitaddr)
            {
                //		LOG1("Limit ! (%.8x)\n", limitaddr);
                memaddr = 0;
            }
            if (memaddr < stopaddr)
                SetStatus(8);
            return data;
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
                        data = adpcmbuf[(memaddr >> 4) & 0x3ffff];
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

        int DecodeADPCMBSample(uint32_t data)
        {
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

            adpcmx = fmgen.Limit(adpcmx + table1[data] * adpcmd / 8, 32767, -32768);
            adpcmd = fmgen.Limit(adpcmd * table2[data] / 64, 24576, 127);
            return adpcmx;
        }

        // FM 音源関係
        uint8_t pan[6];
        uint8_t fnum2[9];

        uint8_t reg22;
        uint32_t reg29;     // OPNA only?

        uint32_t stmask;
        uint32_t statusnext;

        uint32_t lfocount;
        uint32_t lfodcount;

        uint32_t fnum[6];
        uint32_t fnum3[3];

        // ADPCM 関係
        uint8_t* adpcmbuf;        // ADPCM RAM
        uint32_t adpcmmask;     // メモリアドレスに対するビットマスク
        uint32_t adpcmnotice;   // ADPCM 再生終了時にたつビット
        uint32_t startaddr;     // Start address
        uint32_t stopaddr;      // Stop address
        uint32_t memaddr;       // 再生中アドレス
        uint32_t limitaddr;     // Limit address/mask
        int adpcmlevel;     // ADPCM 音量
        int adpcmvolume;
        int adpcmvol;
        uint32_t deltan;            // ⊿N
        int adplc;          // 周波数変換用変数
        int adpld;          // 周波数変換用変数差分値
        uint32_t adplbase;      // adpld の元
        int adpcmx;         // ADPCM 合成用 x
        int adpcmd;         // ADPCM 合成用 ⊿
        int adpcmout;       // ADPCM 合成後の出力
        int apout0;         // out(t-2)+out(t-1)
        int apout1;         // out(t-1)+out(t)

        uint32_t adpcmreadbuf;  // ADPCM リード用バッファ
        bool adpcmplay;     // ADPCM 再生中
        int8_t granuality;
        bool adpcmmask_;

        uint8_t control1;     // ADPCM コントロールレジスタ１
        uint8_t control2;     // ADPCM コントロールレジスタ２
        uint8_t adpcmreg[8];  // ADPCM レジスタの一部分

        int rhythmmask_;

        fmgen::Channel4 ch[6];

        static bool tablehasmade;
};