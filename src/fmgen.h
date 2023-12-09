#pragma once

#include <cmath>
#include <stdint.h>
#include <stdbool.h>

#include "macros.h"

#define FM_EG_BOTTOM 955
#define FM_LFOBITS 8				// 変更不可
#define FM_TLBITS 7
// ---------------------------------------------------------------------------
#define FM_TLENTS (1 << FM_TLBITS)
#define FM_LFOENTS (1 << FM_LFOBITS)
#define FM_TLPOS (FM_TLENTS / 4)
//	サイン波の精度は 2^(1/256)
#define FM_CLENTS (0x1000 * 2) // sin + TL + LFO
#define FM_PI 3.14159265358979323846
#define FM_SINEPRESIS 2         // EGとサイン波の精度の差  0(低)-2(高)
#define FM_OPSINBITS 10
#define FM_OPSINENTS (1 << FM_OPSINBITS)
#define FM_EGCBITS 18           // eg の count のシフト値
#define FM_LFOCBITS 14
#define FM_PGBITS 9
#define FM_RATIOBITS 7         // 8-12 くらいまで？
#define FM_EGBITS 16

enum OpType : int
{
    typeN = 0,
    typeM = 1
};

//	Chip resource

//int*** pmtable = NULL;
//uint32_t*** amtable = NULL;
int pmtable[2][8][FM_LFOENTS];
uint32_t amtable[2][4][FM_LFOENTS];
bool tablemade = false; //TODO: bring it inside the class but nothing should break when linking

class fmgen
{
    // ---------------------------------------------------------------------------
    //	FM Sound Generator - Core Unit
    //	Copyright (C) cisc 1998, 2003.
    // ---------------------------------------------------------------------------
    //	$Id: fmgen.cpp,v 1.49 2003/09/02 14:51:04 cisc Exp $
    // ---------------------------------------------------------------------------
    //	参考:
    //		FM sound generator for M.A.M.E., written by Tatsuyuki Satoh.
    //
    // 	謎:
    //		OPNB の CSM モード(仕様がよくわからない)
    //
    //	制限:
    //		・AR!=31 で SSGEC を使うと波形が実際と異なる可能性あり
    //
    //	謝辞:
    //		Tatsuyuki Satoh さん(fm.c)
    //		Hiromitsu Shioya さん(ADPCM-A)
    //		DMP-SOFT. さん(OPNB)
    //		KAJA さん(test program)
    //		ほか掲示板等で様々なご助言，ご支援をお寄せいただいた皆様に
    // ---------------------------------------------------------------------------
    //# include "headers.h"
    //# include "misc.h"
    //# include "fmgen.h"
    //# include "fmgeninl.h"

    //#define LOGNAME "fmgen"

    // ---------------------------------------------------------------------------

    public:
        // ---------------------------------------------------------------------------

        //namespace FM
        //{
        //	Types ----------------------------------------------------------------
        //typedef FM_SAMPLETYPE   Sample;
        //typedef int32           ISample;
        
        class Chip
        {
            private:
                uint32_t ratio_;
                uint32_t aml_;
                uint32_t pml_;
                int pmv_;
                uint32_t multable_[4][16];
                void MakeTable()
                {
                    int h, l;

                    // PG Part
                    float dt2lv[4] = { 1.0f, 1.414f, 1.581f, 1.732f };

                    for (h = 0; h < 4; h++)
                    {
                        //assert(2 + FM_RATIOBITS - FM_PGBITS >= 0);
                        double rr = dt2lv[h] * (double)(ratio_) / (1 << (2 + FM_RATIOBITS - FM_PGBITS));
                        for (l = 0; l < 16; l++)
                        {
                            int mul = l>0 ? l * 2 : 1;
                            multable_[h][l] = (uint32_t)(mul * rr);
                        }
                    }
                }
            
            public:
                OpType optype_;

                // ---------------------------------------------------------------------------
                //	チップ内で共通な部分
                //
                //Chip::Chip()
                //: ratio_(0), aml_(0), pml_(0), pmv_(0), optype_(typeN)
                //{
                //}
                Chip()
                {
                    ratio_ = 0;
                    aml_ = 0;
                    pml_ = 0;
                    pmv_ = 0;
                    optype_ = OpType::typeN;
                }

                //	クロック・サンプリングレート比に依存するテーブルを作成
                void SetRatio(uint32_t ratio)
                {
                    if (ratio_ != ratio)
                    {
                        ratio_ = ratio;
                        MakeTable();
                    }
                }

                // ---------------------------------------------------------------------------
                //	AM のレベルを設定
                void SetAML(uint32_t l)
                {
                    aml_ = l & (FM_LFOENTS - 1);
                }

                //	PM のレベルを設定
                void SetPML(uint32_t l)
                {
                    pml_ = l & (FM_LFOENTS - 1);
                }

                void SetPMV(int pmv)
                {
                    pmv_ = pmv;
                }

                uint32_t GetMulValue(uint32_t dt2, uint32_t mul)
                {
                    return multable_[dt2][mul];
                }

                uint32_t GetAML()
                {
                    return aml_;
                }

                uint32_t GetPML()
                {
                    return pml_;
                }

                int GetPMV()
                {
                    return pmv_;
                }

                uint32_t GetRatio()
                {
                    return ratio_;
                }
        };

        //	Operator -------------------------------------------------------------
        class Operator
        {
            public:
                static constexpr const uint8_t notetable[128] =
                    {
                    0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  3,  3,  3,  3,  3,  3,
                    4,  4,  4,  4,  4,  4,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,
                    8,  8,  8,  8,  8,  8,  8,  9, 10, 11, 11, 11, 11, 11, 11, 11,
                    12, 12, 12, 12, 12, 12, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15,
                    16, 16, 16, 16, 16, 16, 16, 17, 18, 19, 19, 19, 19, 19, 19, 19,
                    20, 20, 20, 20, 20, 20, 20, 21, 22, 23, 23, 23, 23, 23, 23, 23,
                    24, 24, 24, 24, 24, 24, 24, 25, 26, 27, 27, 27, 27, 27, 27, 27,
                    28, 28, 28, 28, 28, 28, 28, 29, 30, 31, 31, 31, 31, 31, 31, 31,
                    };

                static constexpr const int8_t dttable[256] =
                    {
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  4,
                    4,  6,  6,  6,  8,  8,  8, 10, 10, 12, 12, 14, 16, 16, 16, 16,
                    2,  2,  2,  2,  4,  4,  4,  4,  4,  6,  6,  6,  8,  8,  8, 10,
                    10, 12, 12, 14, 16, 16, 18, 20, 22, 24, 26, 28, 32, 32, 32, 32,
                    4,  4,  4,  4,  4,  6,  6,  6,  8,  8,  8, 10, 10, 12, 12, 14,
                    16, 16, 18, 20, 22, 24, 26, 28, 32, 34, 38, 40, 44, 44, 44, 44,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0, -2, -2, -2, -2, -2, -2, -2, -2, -4, -4, -4, -4,
                    -4, -6, -6, -6, -8, -8, -8,-10,-10,-12,-12,-14,-16,-16,-16,-16,
                    -2, -2, -2, -2, -4, -4, -4, -4, -4, -6, -6, -6, -8, -8, -8,-10,
                    -10,-12,-12,-14,-16,-16,-18,-20,-22,-24,-26,-28,-32,-32,-32,-32,
                    -4, -4, -4, -4, -4, -6, -6, -6, -8, -8, -8,-10,-10,-12,-12,-14,
                    -16,-16,-18,-20,-22,-24,-26,-28,-32,-34,-38,-40,-44,-44,-44,-44,
                    };

                static constexpr const int8_t decaytable1[64][8] = 
                {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
                {1, 1, 1, 1, 1, 1, 1, 1 },{     1, 1, 1, 1, 1, 1, 1, 1},
                {1, 1, 1, 1, 1, 1, 1, 1 },{     1, 1, 1, 1, 1, 1, 1, 1},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 0, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 0, 1, 0, 1, 0, 1, 0 },{     1, 1, 1, 0, 1, 0, 1, 0},
                {1, 1, 1, 0, 1, 1, 1, 0 },{     1, 1, 1, 1, 1, 1, 1, 0},
                {1, 1, 1, 1, 1, 1, 1, 1 },{     2, 1, 1, 1, 2, 1, 1, 1},
                {2, 1, 2, 1, 2, 1, 2, 1 },{     2, 2, 2, 1, 2, 2, 2, 1},
                {2, 2, 2, 2, 2, 2, 2, 2 },{     4, 2, 2, 2, 4, 2, 2, 2},
                {4, 2, 4, 2, 4, 2, 4, 2 },{     4, 4, 4, 2, 4, 4, 4, 2},
                {4, 4, 4, 4, 4, 4, 4, 4 },{     8, 4, 4, 4, 8, 4, 4, 4},
                {8, 4, 8, 4, 8, 4, 8, 4 },{     8, 8, 8, 4, 8, 8, 8, 4},
                {16,16,16,16,16,16,16,16 },{    16,16,16,16,16,16,16,16},
                {16,16,16,16,16,16,16,16 },{    16,16,16,16,16,16,16,16}};

                static constexpr const int decaytable2[16] = 
                {
                    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2047, 2047, 2047, 2047, 2047
                };

                static constexpr const int8_t attacktable[64][8] = 
                {{  -1,-1,-1,-1,-1,-1,-1,-1},{ -1,-1,-1,-1,-1,-1,-1,-1},
                    {   4, 4, 4, 4, 4, 4, 4, 4},{  4, 4, 4, 4, 4, 4, 4, 4},
                    {   4, 4, 4, 4, 4, 4, 4, 4},{  4, 4, 4, 4, 4, 4, 4, 4},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4,-1, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4,-1, 4,-1, 4,-1, 4,-1},{  4, 4, 4,-1, 4,-1, 4,-1},
                    {   4, 4, 4,-1, 4, 4, 4,-1},{  4, 4, 4, 4, 4, 4, 4,-1},
                    {   4, 4, 4, 4, 4, 4, 4, 4},{  3, 4, 4, 4, 3, 4, 4, 4},
                    {   3, 4, 3, 4, 3, 4, 3, 4},{  3, 3, 3, 4, 3, 3, 3, 4},
                    {   3, 3, 3, 3, 3, 3, 3, 3},{  2, 3, 3, 3, 2, 3, 3, 3},
                    {   2, 3, 2, 3, 2, 3, 2, 3},{  2, 2, 2, 3, 2, 2, 2, 3},
                    {   2, 2, 2, 2, 2, 2, 2, 2},{  1, 2, 2, 2, 1, 2, 2, 2},
                    {   1, 2, 1, 2, 1, 2, 1, 2},{  1, 1, 1, 2, 1, 1, 1, 2},
                    {   0, 0, 0, 0, 0, 0, 0, 0},{  0, 0 ,0, 0, 0, 0, 0, 0},
                    {   0, 0, 0, 0, 0, 0, 0, 0},{  0, 0 ,0, 0, 0, 0, 0, 0}};

                static constexpr const int ssgenvtable[8][2][3][2] = 
                {
                    {
                        {  { 1,  1 }, {  1,  1 }, {  1,  1 } },      // 08 
                        {  { 0,  1 }, {  1,  1 }, {  1,  1 } }      // 08 56~
                    },
                    {
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } },      // 09
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } }      // 09
                    },
                    {
                        {  { 1, -1 }, {  0,  1 }, {  1, -1 } },      // 10
                        {  { 0,  1 }, {  1, -1 }, {  0,  1 } }      // 10 60~
                    },
                    {
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } },      // 11
                        {  { 0,  1 }, {  0,  0 }, {  0,  0 } }      // 11 60~
                    },
                    {
                        {  { 2, -1 }, {  2, -1 }, {  2, -1 } },      // 12
                        {  { 1, -1 }, {  2, -1 }, {  2, -1 } }      // 12 56~
                    },
                    {
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } },      // 13
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } }      // 13
                    },
                    {
                        {  { 0,  1 }, {  1, -1 }, {  0,  1 } },      // 14
                        {  { 1, -1 }, {  0,  1 }, {  1, -1 } }      // 14 60~
                    },
                    {
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } },      // 15
                        {  { 1, -1 }, {  2,  0 }, {  2,  0 } }      // 15 60~
                    }
                };

                // ---------------------------------------------------------------------------
                //	Operator
                //
                //bool tablehasmade = false;
                uint32_t sinetable[1024];
                int cltable[FM_CLENTS];

                OpType type_;       // OP の種類 (M, N...)
                bool amon_;     // enable Amplitude Modulation
                bool param_changed_;    // パラメータが更新された
                //	１サンプル合成

                //	ISample を envelop count (2π) に変換するシフト量
                static const int IS2EC_SHIFT = ((20 + FM_PGBITS) - 13);
                int out_, out2_;
                //	Envelop Generator ---------------------------------------------------
                enum EGPhase { next, attack, decay, sustain, release, off };

                
                int dbgopout_;
                int dbgpgout_;

                uint32_t ms_;

                //	構築
                //FM::Operator::Operator()
                //: chip_(0)
                Operator()
                {
                    //if (!tablehasmade)
                        MakeTable();

                    //sinetable=new uint32_t[1024];
                    //cltable=new int[FM_CLENTS];

                    //rate_table = new uint32_t[16];
                    //multable = new uint32_t*[4] { new uint32_t[16], new uint32_t[16], new uint32_t[16], new uint32_t[16] };

                    // EG Part
                    ar_ = dr_ = sr_ = rr_ = key_scale_rate_ = 0;
                    ams_ = amtable[0][0];
                    mute_ = false;
                    keyon_ = false;
                    //tl_out_ = false;
                    tl_out_ = 0;
                    ssg_type_ = 0;

                    // PG Part
                    multiple_ = 0;
                    detune_ = 0;
                    detune2_ = 0;

                    // LFO
                    ms_ = 0;

                    //	Reset();
                }

                //	初期化
                void Reset()
                {
                    // EG part
                    tl_ = tl_latch_ = 127;
                    ShiftPhase(EGPhase::off);
                    eg_count_ = 0;
                    eg_curve_count_ = 0;
                    ssg_phase_ = 0;

                    // PG part
                    pg_count_ = 0;

                    // OP part
                    out_ = out2_ = 0;

                    param_changed_ = true;
                    //PARAMCHANGE(0);
                }

                void MakeTable()
                {
                    // 対数テーブルの作成
                    //assert(FM_CLENTS >= 256);

                    int p = 0;
                    int i;
                    for (i = 0; i < 256; i++)
                    {
                        int v = (int)(floor(pow(2.0, 13.0 - i / 256.0)));
                        v = (v + 2) & ~3;
                        cltable[p++] = v;
                        cltable[p++] = -v;
                    }
                    while (p < FM_CLENTS)
                    {
                        cltable[p] = cltable[p - 512] / 2;
                        p++;
                    }

                    //	for (i=0; i<13*256; i++)
                    //		printf("%4d, %d, %d\n", i, cltable[i*2], cltable[i*2+1]);

                    // サインテーブルの作成
                    double log2 = log(2.0);
                    for (i = 0; i < FM_OPSINENTS / 2; i++)
                    {
                        double r = (i * 2 + 1) * FM_PI / FM_OPSINENTS;
                        double q = -256 * log(sin(r)) / log2;
                        uint32_t s = (uint32_t)((int)(floor(q + 0.5)) + 1);
                        //		printf("%d, %d\n", s, cltable[s * 2] / 8);
                        sinetable[i] = s * 2;
                        sinetable[FM_OPSINENTS / 2 + i] = s * 2 + 1;
                    }

                    fmgen::MakeLFOTable();

                    //tablehasmade = true;
                }

                void SetDPBN(uint32_t dp, uint32_t bn)
                {
                    dp_ = dp;
                    bn_ = bn;
                    param_changed_ = true;
                    //PARAMCHANGE(1);
                }

                //	準備
                void Prepare()
                {
                    if (param_changed_)
                    {
                        param_changed_ = false;
                        //	PG Part
                        pg_diff_ = (uint32_t)(dp_ + dttable[detune_ + bn_]) * chip_.GetMulValue(detune2_, multiple_);
                        pg_diff_lfo_ = (int)(pg_diff_ >> 11);

                        // EG Part
                        key_scale_rate_ = bn_ >> (int)(3 - ks_);
                        tl_out_ = (int)(mute_ ? 0x3ff : tl_ * 8);

                        switch (eg_phase_)
                        {
                            case EGPhase::attack:
                                SetEGRate(ar_!=0 ? my_min(63, ar_ + key_scale_rate_) : 0);
                                break;
                            case EGPhase::decay:
                                SetEGRate(dr_!=0 ? my_min(63, dr_ + key_scale_rate_) : 0);
                                eg_level_on_next_phase_ = (int)(sl_ * 8);
                                break;
                            case EGPhase::sustain:
                                SetEGRate(sr_!=0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                                break;
                            case EGPhase::release:
                                SetEGRate(my_min(63, rr_ + key_scale_rate_));
                                break;
                            default: break;
                        }

                        // SSG-EG
                        if (ssg_type_!=0 && (eg_phase_ != EGPhase::release))
                        {
                            int m = (ar_ >= ((ssg_type_ == 8 || ssg_type_ == 12) ? 56 : 60) ? 1 : 0);

                            //assert(0 <= ssg_phase_ && ssg_phase_ <= 2);
                            int phase = (ssg_phase_ >= 0 && ssg_phase_ <= 2) ? ssg_phase_ : 0;
                            const int* table = ssgenvtable[ssg_type_ & 7][m][phase];

                            ssg_offset_ = table[0] * 0x200;
                            ssg_vector_ = table[1];
                        }
                        // LFO
                        ams_ = amtable[(int)type_][amon_ ? (ms_ >> 4) & 3 : 0];
                        EGUpdate();

                        dbgopout_ = 0;
                    }
                }

                //	envelop の eg_phase_ 変更
                void ShiftPhase(EGPhase nextphase)
                {
                    switch (nextphase)
                    {
                        case EGPhase::attack:        // Attack Phase
                            //Console.WriteLine("a1");
                            tl_ = tl_latch_;
                            if (ssg_type_!=0)
                            {
                                //Console.WriteLine("a2");
                                ssg_phase_ = ssg_phase_ + 1;
                                if (ssg_phase_ > 2)
                                    ssg_phase_ = 1;

                                int m = (ar_ >= ((ssg_type_ == 8 || ssg_type_ == 12) ? 56 : 60) ? 1 : 0);

                                //assert(0 <= ssg_phase_ && ssg_phase_ <= 2);
                                int phase = (ssg_phase_ >= 0 && ssg_phase_ <= 2) ? ssg_phase_ : 0;
                                const int* table = ssgenvtable[ssg_type_ & 7][m][phase];

                                ssg_offset_ = table[0] * 0x200;
                                ssg_vector_ = table[1];
                            }
                            //Console.WriteLine("a3");
                            if ((ar_ + key_scale_rate_) < 62)
                            {
                                SetEGRate(ar_!=0 ? my_min(63, ar_ + key_scale_rate_) : 0);
                                eg_phase_ = EGPhase::attack;
                                break;
                            }

                            //Console.WriteLine("a4");
                            //Console.WriteLine("key_scale_rate_[{0}]", key_scale_rate_);
                            if (sl_ != 0)
                            {
                                eg_level_ = 0;
                                eg_level_on_next_phase_ = (int)(ssg_type_ != 0 ? my_min(sl_ * 8, 0x200) : sl_ * 8);

                                SetEGRate(dr_ != 0 ? my_min(63, dr_ + key_scale_rate_) : 0);
                                eg_phase_ = EGPhase::decay;
                                break;
                            }

                            //Console.WriteLine("a5");
                            eg_level_ = (int)(sl_ * 8);
                            eg_level_on_next_phase_ = ssg_type_ != 0 ? 0x200 : 0x400;

                            SetEGRate(sr_ != 0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                            eg_phase_ = EGPhase::sustain;
                            break;
                        case EGPhase::decay:         // Decay Phase
                            if (sl_!=0)
                            {
                                eg_level_ = 0;
                                eg_level_on_next_phase_ = (int)(ssg_type_!=0 ? my_min(sl_ * 8, 0x200) : sl_ * 8);

                                SetEGRate(dr_!=0 ? my_min(63, dr_ + key_scale_rate_) : 0);
                                eg_phase_ = EGPhase::decay;
                                break;
                            }

                            eg_level_ = (int)(sl_ * 8);
                            eg_level_on_next_phase_ = ssg_type_ != 0 ? 0x200 : 0x400;

                            SetEGRate(sr_ != 0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                            eg_phase_ = EGPhase::sustain;
                            break;
                        case EGPhase::sustain:       // Sustain Phase
                            eg_level_ = (int)(sl_ * 8);
                            eg_level_on_next_phase_ = ssg_type_!=0 ? 0x200 : 0x400;

                            SetEGRate(sr_!=0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                            eg_phase_ = EGPhase::sustain;
                            break;

                        case EGPhase::release:       // Release Phase
                            if (ssg_type_!=0)
                            {
                                eg_level_ = eg_level_ * ssg_vector_ + ssg_offset_;
                                ssg_vector_ = 1;
                                ssg_offset_ = 0;
                            }
                            if (eg_phase_ == EGPhase::attack || (eg_level_ < FM_EG_BOTTOM)) //0x400/* && eg_phase_ != off*/))
                            {
                                eg_level_on_next_phase_ = 0x400;
                                SetEGRate(my_min(63, rr_ + key_scale_rate_));
                                eg_phase_ = EGPhase::release;
                                break;
                            }

                            eg_level_ = FM_EG_BOTTOM;
                            eg_level_on_next_phase_ = FM_EG_BOTTOM;
                            EGUpdate();
                            SetEGRate(0);
                            eg_phase_ = EGPhase::off;
                            break;

                        case EGPhase::off:           // off
                        default:
                            eg_level_ = FM_EG_BOTTOM;
                            eg_level_on_next_phase_ = FM_EG_BOTTOM;
                            EGUpdate();
                            SetEGRate(0);
                            eg_phase_ = EGPhase::off;
                            break;
                    }
                }

                //	Block/F-Num
                void SetFNum(uint32_t f)
                {
                    dp_ = (f & 2047) << (int)((f >> 11) & 7);
                    bn_ = notetable[(f >> 7) & 127];
                    param_changed_ = true;
                    //PARAMCHANGE(2);
                }

                // 入力: s = 20+FM_PGBITS = 29
                uint32_t Sine(int s) {
                    return sinetable[((s) >> (20 + FM_PGBITS - FM_OPSINBITS)) & (FM_OPSINENTS - 1)];
                }

                int SINE(int s) {
                    return (int)sinetable[(s) & (FM_OPSINENTS - 1)];
                }


                int LogToLin(uint32_t a)
                {
                    //#if 1 // FM_CLENTS < 0xc00		// 400 for TL, 400 for ENV, 400 for LFO.
                        return (a < FM_CLENTS) ? cltable[a] : 0;
                    //#else
                    //return cltable[a];
                    //#endif
                }

                void EGUpdate()
                {
                    if (ssg_type_==0)
                    {
                        eg_out_ = my_min(tl_out_ + eg_level_, 0x3ff) << (1 + 2);
                    }
                    else
                    {
                        eg_out_ = my_min(tl_out_ + eg_level_ * ssg_vector_ + ssg_offset_, 0x3ff) << (1 + 2);
                    }
                }

                void SetEGRate(uint32_t rate)
                {
                    eg_rate_ = (int)rate;
                    eg_count_diff_ = (int)(decaytable2[rate / 4] * chip_.GetRatio());
                }

                //	EG 計算
                void EGCalc()
                {
                    eg_count_ = (2047 * 3) << FM_RATIOBITS;             // ##この手抜きは再現性を低下させる

                    if (eg_phase_ == EGPhase::attack)
                    {
                        int c = attacktable[eg_rate_][eg_curve_count_ & 7];
                        if (c >= 0)
                        {
                            eg_level_ -= 1 + (eg_level_ >> c);
                            if (eg_level_ <= 0)
                                ShiftPhase( EGPhase::decay);
                        }
                        EGUpdate();
                    }
                    else
                    {
                        if (ssg_type_==0)
                        {
                            eg_level_ += decaytable1[eg_rate_][eg_curve_count_ & 7];
                            if (eg_level_ >= eg_level_on_next_phase_)
                                ShiftPhase((EGPhase)(eg_phase_ + 1));
                            EGUpdate();
                        }
                        else
                        {
                            eg_level_ += 4 * decaytable1[eg_rate_][eg_curve_count_ & 7];
                            if (eg_level_ >= eg_level_on_next_phase_)
                            {
                                EGUpdate();
                                switch (eg_phase_)
                                {
                                    case EGPhase::decay:
                                        ShiftPhase( EGPhase::sustain);
                                        break;
                                    case EGPhase::sustain:
                                        ShiftPhase( EGPhase::attack);
                                        break;
                                    case EGPhase::release:
                                        ShiftPhase( EGPhase::off);
                                        break;
                                    default: break;
                                }
                            }
                        }
                    }

                    eg_curve_count_++;
                }

                void EGStep()
                {
                    eg_count_ -= eg_count_diff_;

                    // EG の変化は全スロットで同期しているという噂もある
                    if (eg_count_ <= 0)
                        EGCalc();
                }

                //	PG 計算
                //	ret:2^(20+PGBITS) / cycle
                uint32_t PGCalc()
                {
                    uint32_t ret = pg_count_;
                    pg_count_ += pg_diff_;
                    dbgpgout_ = (int)ret;
                    return ret;
                }

                uint32_t PGCalcL()
                {
                    uint32_t ret = pg_count_;
                    pg_count_ += (uint32_t)(pg_diff_ + ((pg_diff_lfo_ * chip_.GetPMV()) >> 5));// & -(1 << (2+IS2EC_SHIFT)));
                    dbgpgout_ = (int)ret;
                    return ret /* + pmv * pg_diff_;*/;
                }

                //	OP 計算
                //	in: ISample (最大 8π)
                int Calc(int In)
                {
                    EGStep();
                    out2_ = out_;

                    int pgin = (int)(PGCalc() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(pgin)));

                    dbgopout_ = out_;
                    return out_;
                }

                int CalcL(int In)
                {
                    EGStep();

                    int pgin = (int)(PGCalcL() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(pgin) + ams_[chip_.GetAML()]));

                    dbgopout_ = out_;
                    return out_;
                }

                int CalcN(uint32_t noise)
                {
                    EGStep();

                    int lv = my_max(0, 0x3ff - (tl_out_ + eg_level_)) << 1;

                    // noise & 1 ? lv : -lv と等価 
                    noise = (noise & 1) - 1;
                    out_ = (int)((lv + noise) ^ noise);

                    dbgopout_ = out_;
                    return out_;
                }

                //	OP (FB) 計算
                //	Self Feedback の変調最大 = 4π
                int CalcFB(uint32_t fb)
                {
                    EGStep();

                    int In = out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalc() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    if (fb < 31)
                    {
                        pgin += ((In << (int)(1 + IS2EC_SHIFT)) >> (int)fb) >> (20 + FM_PGBITS - FM_OPSINBITS);
                    }
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(pgin)));
                    dbgopout_ = out2_;

                    return out2_;
                }

                int CalcFBL(uint32_t fb)
                {
                    EGStep();

                    int In = out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalcL() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    if (fb < 31)
                    {
                        pgin += ((In << (int)(1 + IS2EC_SHIFT)) >> (int)fb) >> (20 + FM_PGBITS - FM_OPSINBITS);
                    }

                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(pgin) + ams_[chip_.GetAML()]));
                    dbgopout_ = out_;

                    return out_;
                }

                void ResetFB()
                {
                    out_ = out2_ = 0;
                }

                //	キーオン
                void KeyOn()
                {
                    if (!keyon_)
                    {
                        keyon_ = true;
                        if (eg_phase_ == EGPhase::off || eg_phase_ == EGPhase::release)
                        {
                            ssg_phase_ = -1;
                            ShiftPhase( EGPhase::attack);
                            EGUpdate();
                            in2_ = out_ = out2_ = 0;
                            pg_count_ = 0;
                        }
                    }
                }

                //	キーオフ
                void KeyOff()
                {
                    if (keyon_)
                    {
                        keyon_ = false;
                        ShiftPhase(EGPhase::release);
                    }
                }

                //	オペレータは稼働中か？
                int IsOn()
                {
                    return eg_phase_ - EGPhase::off;
                }

                //	Detune (0-7)
                void SetDT(uint32_t dt)
                {
                    detune_ = dt * 0x20;
                    param_changed_ = true;
                    //PARAMCHANGE(4);
                }

                //	DT2 (0-3)
                void SetDT2(uint32_t dt2)
                {
                    detune2_ = dt2 & 3;
                    param_changed_ = true;
                    //PARAMCHANGE(5);
                }

                //	Multiple (0-15)
                void SetMULTI(uint32_t mul)
                {
                    multiple_ = mul;
                    param_changed_ = true;
                    //PARAMCHANGE(6);
                }

                //	Total Level (0-127) (0.75dB step)
                void SetTL(uint32_t tl, bool csm)
                {
                    if (!csm)
                    {
                        tl_ = tl;
                        param_changed_ = true;
                        //PARAMCHANGE(7);
                    }
                    tl_latch_ = tl;
                }

                //	Attack Rate (0-63)
                void SetAR(uint32_t ar)
                {
                    ar_ = ar;
                    param_changed_ = true;
                    //PARAMCHANGE(8);
                }

                //	Decay Rate (0-63)
                void SetDR(uint32_t dr)
                {
                    dr_ = dr;
                    param_changed_ = true;
                    //PARAMCHANGE(9);
                }

                //	Sustain Rate (0-63)
                void SetSR(uint32_t sr)
                {
                    sr_ = sr;
                    param_changed_ = true;
                    //PARAMCHANGE(10);
                }

                //	Sustain Level (0-127)
                void SetSL(uint32_t sl)
                {
                    sl_ = sl;
                    param_changed_ = true;
                    //PARAMCHANGE(11);
                }

                //	Release Rate (0-63)
                void SetRR(uint32_t rr)
                {
                    rr_ = rr;
                    param_changed_ = true;
                    //PARAMCHANGE(12);
                }

                //	Keyscale (0-3)
                void SetKS(uint32_t ks)
                {
                    ks_ = ks;
                    param_changed_ = true;
                    //PARAMCHANGE(13);
                }

                //	SSG-type Envelop (0-15)
                void SetSSGEC(uint32_t ssgec)
                {
                    if ((ssgec & 8)!=0)
                        ssg_type_ = ssgec;
                    else
                        ssg_type_ = 0;
                }

                void SetAMON(bool amon)
                {
                    amon_ = amon;
                    param_changed_ = true;
                    //PARAMCHANGE(14);
                }

                void Mute(bool mute)
                {
                    mute_ = mute;
                    param_changed_ = true;
                    //PARAMCHANGE(15);
                }

                void SetMS(uint32_t ms)
                {
                    ms_ = ms;
                    param_changed_ = true;
                    //PARAMCHANGE(16);
                }

                void SetChip(Chip chip)
                {
                    chip_ = chip;
                }

                static void MakeTimeTable(uint32_t ratio)
                {
                }

                void SetMode(bool modulator)
                {
                }

                int Out()
                {
                    return out_;
                }

                int dbgGetIn2()
                {
                    return in2_;
                }

                void dbgStopPG()
                {
                    pg_diff_ = 0; pg_diff_lfo_ = 0;
                }

                //	friends --------------------------------------------------------------
                //private class Channel4;

                int* dbgGetClTable()
                {
                    return cltable;
                }

                uint32_t* dbgGetSineTable()
                {
                    return sinetable;
                }

            private:
                uint32_t bn_;       // Block/Note
                int eg_level_;  // EG の出力値
                int eg_level_on_next_phase_;    // 次の eg_phase_ に移る値
                int eg_count_;      // EG の次の変移までの時間
                int eg_count_diff_; // eg_count_ の差分
                int eg_out_;        // EG+TL を合わせた出力値
                int tl_out_;        // TL 分の出力値
                                            //private bool tl_out_;        // TL 分の出力値
                                            //		int		pm_depth_;		// PM depth
                                            //		int		am_depth_;		// AM depth
                int eg_rate_;
                int eg_curve_count_;
                int ssg_offset_;
                int ssg_vector_;
                int ssg_phase_;


                uint32_t key_scale_rate_;       // key scale rate
                EGPhase eg_phase_;
                uint32_t* ams_;

                uint32_t tl_;           // Total Level	 (0-127)
                uint32_t tl_latch_;     // Total Level Latch (for CSM mode)
                uint32_t ar_;           // Attack Rate   (0-63)
                uint32_t dr_;           // Decay Rate    (0-63)
                uint32_t sr_;           // Sustain Rate  (0-63)
                uint32_t sl_;           // Sustain Level (0-127)
                uint32_t rr_;           // Release Rate  (0-63)
                uint32_t ks_;           // Keyscale      (0-3)
                uint32_t ssg_type_; // SSG-Type Envelop Control

                bool keyon_;
                bool mute_;

                Chip chip_;
                int in2_;

                //	Phase Generator ------------------------------------------------------
                uint32_t dp_;       // ΔP
                uint32_t detune_;       // Detune
                uint32_t detune2_;  // DT2
                uint32_t multiple_; // Multiple
                uint32_t pg_count_;   // Phase 現在値
                uint32_t pg_diff_;    // Phase 差分値
                int pg_diff_lfo_; // Phase 差分値 >> x

                //	Tables ---------------------------------------------------------------
                uint32_t rate_table[16];
                uint32_t multable[4][16];

                void SSGShiftPhase(int mode)
                {
                }

                int FBCalc(int fb)
                {
                    return -1;
                }

                void FM_NextPhase(Operator op)
                {
                }

        };

        //	4-op Channel ---------------------------------------------------------
        class Channel4
        {
            // ---------------------------------------------------------------------------
            //	4-op Channel
            //
            private:
                static constexpr const uint8_t fbtable[8] = { 31, 7, 6, 5, 4, 3, 2, 1 };
                //static bool tablehasmade;
                uint32_t fb;
                int buf[4];
                int In[3];          // 各 OP の入力ポインタ
                int Out[3];         // 各 OP の出力ポインタ
                int* pms;
                int algo_;
                Chip chip_;

            public:
                int kftable[64];
                Operator* op;

                Channel4()
                {
                    //tablehasmade = false;
                    //if (!tablehasmade)
                        MakeTable();

                    op = new Operator[4]{
                        Operator(),Operator(),Operator(),Operator()
                    };

                    SetAlgorithm(0);
                    pms = pmtable[0][0];

                    

                    //buf = new int[4];
                    //In = new int[3];          // 各 OP の入力ポインタ
                    //Out = new int[3]; 
                }

                ~Channel4()
                {
                    delete[] op;
                }

                void MakeTable()
                {
                    //kftable = new int[64];

                    for (int i = 0; i < 64; i++)
                    {
                        kftable[i] = (int)(0x10000 * pow(2.0, i / 768.0));
                    }
                }

                // リセット
                void Reset()
                {
                    op[0].Reset();
                    op[1].Reset();
                    op[2].Reset();
                    op[3].Reset();
                }

                //	Calc の用意
                int Prepare()
                {
                    op[0].Prepare();
                    op[1].Prepare();
                    op[2].Prepare();
                    op[3].Prepare();

                    pms = pmtable[(int)op[0].type_][op[0].ms_ & 7];
                    int key = (op[0].IsOn() | op[1].IsOn() | op[2].IsOn() | op[3].IsOn())!=0 ? 1 : 0;
                    int lfo = (op[0].ms_ & ((op[0].amon_ | op[1].amon_ | op[2].amon_ | op[3].amon_) ? 0x37 : 7)) != 0 ? 2 : 0;
                    return key | lfo;
                }

                //	F-Number/BLOCK を設定
                void SetFNum(uint32_t f)
                {
                    for (int i = 0; i < 4; i++)
                        op[i].SetFNum(f);
                }

                //	KC/KF を設定
                void SetKCKF(uint32_t kc, uint32_t kf)
                {
                    uint32_t* kctable = new uint32_t[16]
                    {
                        5197, 5506, 5833, 6180, 6180, 6547, 6937, 7349,
                        7349, 7786, 8249, 8740, 8740, 9259, 9810, 10394,
                    };

                    int oct = (int)(19 - ((kc >> 4) & 7));

                    //printf("%p", this);
                    uint32_t kcv = kctable[kc & 0x0f];
                    kcv = (kcv + 2) / 4 * 4;
                    //printf(" %.4x", kcv);
                    uint32_t dp = (uint32_t)(kcv * kftable[kf & 0x3f]);
                    //printf(" %.4x %.4x %.8x", kcv, kftable[kf & 0x3f], dp >> oct);
                    dp >>= 16 + 3;
                    dp <<= 16 + 3;
                    dp >>= oct;
                    uint32_t bn = (kc >> 2) & 31;
                    op[0].SetDPBN(dp, bn);
                    op[1].SetDPBN(dp, bn);
                    op[2].SetDPBN(dp, bn);
                    op[3].SetDPBN(dp, bn);
                    //printf(" %.8x\n", dp);
                }

                //	キー制御
                void KeyControl(uint32_t key)
                {
                    if ((key & 0x1)!=0) op[0].KeyOn();
                    else op[0].KeyOff();
                    if ((key & 0x2)!=0) op[1].KeyOn();
                    else op[1].KeyOff();
                    if ((key & 0x4)!=0) op[2].KeyOn();
                    else op[2].KeyOff();
                    if ((key & 0x8)!=0) op[3].KeyOn();
                    else op[3].KeyOff();
                }

                //	アルゴリズムを設定
                void SetAlgorithm(uint32_t algo)
                {
                    uint8_t table1[8][6] = {
                        { 0, 1, 1, 2, 2, 3 },
                        { 1, 0, 0, 1, 1, 2 },
                        { 1, 1, 1, 0, 0, 2 },
                        { 0, 1, 2, 1, 1, 2 },
                        { 0, 1, 2, 2, 2, 1 },
                        { 0, 1, 0, 1, 0, 1 },
                        { 0, 1, 2, 1, 2, 1 },
                        { 1, 0, 1, 0, 1, 0 },
                    };

                    In[0] = table1[algo][0];
                    Out[0] = table1[algo][1];
                    In[1] =table1[algo][2];
                    Out[1] = table1[algo][3];
                    In[2] = table1[algo][4];
                    Out[2] = table1[algo][5];

                    op[0].ResetFB();
                    algo_ = (int)algo;
                }

                //  合成
                int Calc()
                {
                    int r = 0;
                    switch (algo_)
                    {
                        case 0:
                            op[2].Calc(op[1].Out());
                            op[1].Calc(op[0].Out());
                            r = op[3].Calc(op[2].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 1:
                            op[2].Calc(op[0].Out() + op[1].Out());
                            op[1].Calc(0);
                            r = op[3].Calc(op[2].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 2:
                            op[2].Calc(op[1].Out());
                            op[1].Calc(0);
                            r = op[3].Calc(op[0].Out() + op[2].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 3:
                            op[2].Calc(0);
                            op[1].Calc(op[0].Out());
                            r = op[3].Calc(op[1].Out() + op[2].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 4:
                            op[2].Calc(0);
                            r = op[1].Calc(op[0].Out());
                            r += op[3].Calc(op[2].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 5:
                            r = op[2].Calc(op[0].Out());
                            r += op[1].Calc(op[0].Out());
                            r += op[3].Calc(op[0].Out());
                            op[0].CalcFB(fb);
                            break;
                        case 6:
                            r = op[2].Calc(0);
                            r += op[1].Calc(op[0].Out());
                            r += op[3].Calc(0);
                            op[0].CalcFB(fb);
                            break;
                        case 7:
                            r = op[2].Calc(0);
                            r += op[1].Calc(0);
                            r += op[3].Calc(0);
                            r += op[0].CalcFB(fb);
                            break;
                    }
                    return r;
                }

                //  合成
                int CalcL()
                {
                    chip_.SetPMV(pms[chip_.GetPML()]);

                    int r=0;
                    switch (algo_)
                    {
                        case 0:
                            op[2].CalcL(op[1].Out());
                            op[1].CalcL(op[0].Out());
                            r = op[3].CalcL(op[2].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 1:
                            op[2].CalcL(op[0].Out() + op[1].Out());
                            op[1].CalcL(0);
                            r = op[3].CalcL(op[2].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 2:
                            op[2].CalcL(op[1].Out());
                            op[1].CalcL(0);
                            r = op[3].CalcL(op[0].Out() + op[2].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 3:
                            op[2].CalcL(0);
                            op[1].CalcL(op[0].Out());
                            r = op[3].CalcL(op[1].Out() + op[2].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 4:
                            op[2].CalcL(0);
                            r = op[1].CalcL(op[0].Out());
                            r += op[3].CalcL(op[2].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 5:
                            r = op[2].CalcL(op[0].Out());
                            r += op[1].CalcL(op[0].Out());
                            r += op[3].CalcL(op[0].Out());
                            op[0].CalcFBL(fb);
                            break;
                        case 6:
                            r = op[2].CalcL(0);
                            r += op[1].CalcL(op[0].Out());
                            r += op[3].CalcL(0);
                            op[0].CalcFBL(fb);
                            break;
                        case 7:
                            r = op[2].CalcL(0);
                            r += op[1].CalcL(0);
                            r += op[3].CalcL(0);
                            r += op[0].CalcFBL(fb);
                            break;
                    }
                    return r;
                }

                //  合成
                int CalcN(uint32_t noise)
                {
                    buf[1] = buf[2] = buf[3] = 0;

                    buf[0] = op[0].out_; op[0].CalcFB(fb);
                    Out[0] += op[1].Calc(buf[In[0]]);
                    Out[1] += op[2].Calc(buf[In[1]]);
                    int o = op[3].out_;
                    op[3].CalcN(noise);
                    return buf[Out[2]] + o;
                }

                //  合成
                int CalcLN(uint32_t noise)
                {
                    chip_.SetPMV(pms[chip_.GetPML()]);
                    buf[1] = buf[2] = buf[3] = 0;

                    buf[0] = op[0].out_; op[0].CalcFBL(fb);
                    Out[0] += op[1].CalcL(buf[In[0]]);
                    Out[1] += op[2].CalcL(buf[In[1]]);
                    int o = op[3].out_;
                    op[3].CalcN(noise);
                    return buf[Out[2]] + o;
                }

                //	オペレータの種類 (LFO) を設定
                void SetType(OpType type)
                {
                    for (int i = 0; i < 4; i++)
                        op[i].type_ = type;
                }

                //	セルフ・フィードバックレートの設定 (0-7)
                void SetFB(uint32_t feedback)
                {
                    fb = fbtable[feedback];
                }

                //	OPNA 系 LFO の設定
                void SetMS(uint32_t ms)
                {
                    op[0].SetMS(ms);
                    op[1].SetMS(ms);
                    op[2].SetMS(ms);
                    op[3].SetMS(ms);
                }

                //	チャンネル・マスク
                void Mute(bool m)
                {
                    for (int i = 0; i < 4; i++)
                        op[i].Mute(m);
                }

                //	内部パラメータを再計算
                void Refresh()
                {
                    for (int i = 0; i < 4; i++)
                        op[i].param_changed_ = true;
                    //PARAMCHANGE(3);
                }

                void SetChip(Chip chip)
                {
                    chip_ = chip;
                    for (int i = 0; i < 4; i++)
                        op[i].SetChip(chip);
                }

                void dbgStopPG()
                {
                    for (int i = 0; i < 4; i++) op[i].dbgStopPG();
                }
        };

        fmgen()
        {
            tablemade = false;
            //pmtable = NULL;
            //amtable = NULL;
        }

        // ---------------------------------------------------------------------------
        //	Table/etc
        //
        //namespace FM
        //{
        // ---------------------------------------------------------------------------
        //	FM Sound Generator
        //	Copyright (C) cisc 1998, 2001.
        // ---------------------------------------------------------------------------
        //	$Id: fmgen.h,v 1.37 2003/08/25 13:33:11 cisc Exp $
        //# ifndef FM_GEN_H
        //#define FM_GEN_H
        //#include "types.h"
        // ---------------------------------------------------------------------------
        //	出力サンプルの型
        //
        //#define FM_SAMPLETYPE	int32				// int16 or int32
        // ---------------------------------------------------------------------------
        //	定数その１
        //	静的テーブルのサイズ

        //class Chip;

        //extern int paramcount[];
        //#define PARAMCHANGE(i) paramcount[i]++;
        void PARAMCHANGE(int i)
        {
        }

        //void StoreSample(ISample& dest, int data);
        // ---------------------------------------------------------------------------
        //
        //
        static void StoreSample(int& dest, int data)
        {
            dest = (int)Limit(dest + data, 0x7fff, -0x8000);
        }

        static int Limit(int v, int max, int min)
        {
            return v > max ? max : (v < min ? min : v);
        }

        // ---------------------------------------------------------------------------
        //	テーブル作成
        //
        static void MakeLFOTable()
        {
            /*if(pmtable == NULL)
            {
                pmtable = new int**[2] {
                    new int*[8] { new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS] },
                    new int*[8] { new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS], new int[FM_LFOENTS] }
                };
            }
            
            if(amtable == NULL)
            {
                amtable = new uint32_t**[2] {
                    new uint32_t*[4] { new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS] },
                    new uint32_t*[4] { new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS],new uint32_t[FM_LFOENTS] }
                };
            }*/

            if (tablemade)
                return;

            tablemade = true;

            int i;

            double pms[2][8] =  {
            {  0, 1/360.0, 2/360.0, 3/360.0,  4/360.0,  6/360.0, 12/360.0,  24/360.0, },	// OPNA
            //		{ 0, 1/240., 2/240., 4/240., 10/240., 20/240., 80/240., 140/240., },	// OPM
            { 0, 1/480.0, 2/480.0, 4/480.0, 10/480.0, 20/480.0, 80/480.0, 140/480.0, }    // OPM
            //		{ 0, 1/960., 2/960., 4/960., 10/960., 20/960., 80/960., 140/960., },	// OPM
            };
            //		 3		 6,      12      30       60       240      420		/ 720
            //	1.000963
            //	lfofref[level * max * wave];
            //	pre = lfofref[level][pms * wave >> 8];
            uint8_t amt[2][4] = {
                { 31, 6, 4, 3 }, // OPNA
                { 31, 2, 1, 0 }, //	OPM
            };

            for (int type = 0; type < 2; type++)
            {
                for (i = 0; i < 8; i++)
                {
                    double pmb = pms[type][i];
                    for (int j = 0; j < FM_LFOENTS; j++)
                    {
                        double v = pow(2.0, pmb * (2 * j - FM_LFOENTS + 1) / (FM_LFOENTS - 1));
                        double w = 0.6 * pmb * sin(2 * j * 3.14159265358979323846 / FM_LFOENTS) + 1;
                        //				pmtable[type][i][j] = int(0x10000 * (v - 1));
                        //				if (type == 0)
                        pmtable[type][i][j] = (int)(0x10000 * (w - 1));
                        //				else
                        //					pmtable[type][i][j] = int(0x10000 * (v - 1));

                        //				printf("pmtable[%d][%d][%.2x] = %5d  %7.5f %7.5f\n", type, i, j, pmtable[type][i][j], v, w);
                    }
                }
                for (i = 0; i < 4; i++)
                {
                    for (int j = 0; j < FM_LFOENTS; j++)
                    {
                        amtable[type][i][j] = (uint32_t)(((j * 4) >> amt[type][i]) * 2) << 2;
                    }
                }
            }
        }
};