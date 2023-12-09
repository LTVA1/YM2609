#pragma once
#include "fmgen.h"

#include "pantable_opna.h"

class fmvgen : public fmgen
{
    public:
        const int waveChSize = 12;
        const static int waveTypeSize = 4;
        const static int waveBufSize = 1024;

        /* = new uint32_t[12][][]{
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] },
            new uint32_t[4][] { new uint32_t[1024], new uint32_t[1024], new uint32_t[1024], new uint32_t[1024] }
        };*/

        //	Operator -------------------------------------------------------------
        class Operator : fmgen::Operator
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
                {
                { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
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
                {16,16,16,16,16,16,16,16 },{    16,16,16,16,16,16,16,16},
                };

                static constexpr const int decaytable2[16] =
                {
                    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2047, 2047, 2047, 2047, 2047
                };

                static constexpr const int8_t attacktable[64][8] =
                {
                {  -1,-1,-1,-1,-1,-1,-1,-1},{ -1,-1,-1,-1,-1,-1,-1,-1},
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
                {   0, 0, 0, 0, 0, 0, 0, 0},{  0, 0 ,0, 0, 0, 0, 0, 0},
                };

                static constexpr const int ssgenvtable[8][2][3][2] = {
                     {
                        {  { 1,  1 }, {  1,  1 }, {  1,  1 } },      // 08 
                        {  { 0,  1 }, {  1,  1 }, {  1,  1 } },      // 08 56~
                    },
                     {
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } },      // 09
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } },      // 09
                    },
                     {
                        {  { 1, -1 }, {  0,  1 }, {  1, -1 } },      // 10
                        {  { 0,  1 }, {  1, -1 }, {  0,  1 } },      // 10 60~
                    },
                     {
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } },      // 11
                        {  { 0,  1 }, {  0,  0 }, {  0,  0 } },      // 11 60~
                    },
                     {
                        {  { 2, -1 }, {  2, -1 }, {  2, -1 } },      // 12
                        {  { 1, -1 }, {  2, -1 }, {  2, -1 } },      // 12 56~
                    },
                     {
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } },      // 13
                        {  { 1, -1 }, {  0,  0 }, {  0,  0 } },      // 13
                    },
                     {
                        {  { 0,  1 }, {  1, -1 }, {  0,  1 } },      // 14
                        {  { 1, -1 }, {  0,  1 }, {  1, -1 } },      // 14 60~
                    },
                     {
                        {  { 0,  1 }, {  2,  0 }, {  2,  0 } },      // 15
                        {  { 1, -1 }, {  2,  0 }, {  2,  0 } },      // 15 60~
                    }
                };

                // ---------------------------------------------------------------------------
                //	Operator
                //
                //bool tablehasmade = false;
                //uint32_t[] sinetable = new uint32_t[1024];
                int cltable[FM_CLENTS];

                OpType type_;       // OP の種類 (M, N...)
                uint32_t ms_;
                uint32_t fb_;
                uint8_t algLink_;
                uint8_t wt_;
                bool amon_;     // enable Amplitude Modulation
                bool param_changed_;    // パラメータが更新された
                //	ISample を envelop count (2π) に変換するシフト量
                
                int out_, out2_;
                int dbgopout_;
                int dbgpgout_;
                //	Envelop Generator ---------------------------------------------------
                enum EGPhase { next, attack, decay, sustain, release, off };

                //	構築
                //FM::Operator::Operator()
                //: chip_(0)
                Operator()
                {
                    for(int i = 0; i < FM_CLENTS; i++)
                    {
                        cltable[i] = 0;
                    }

                    MakeTable();

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
                        //Console.Write("{0}:", cltable[p]);
                        p++;
                    }

                    for (int j = 0; j < 12; j++)
                    {
                        fmvgen::waveReset(j, 0);
                        fmvgen::waveReset(j, 1);
                        fmvgen::waveReset(j, 2);
                        fmvgen::waveReset(j, 3);
                    }
                    //}

                    fmvgen::MakeLFOTable();

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
                                SetEGRate(ar_ != 0 ? my_min(63, ar_ + key_scale_rate_) : 0);
                                break;
                            case EGPhase::decay:
                                SetEGRate(dr_ != 0 ? my_min(63, dr_ + key_scale_rate_) : 0);
                                eg_level_on_next_phase_ = (int)(sl_ * 8);
                                break;
                            case EGPhase::sustain:
                                SetEGRate(sr_ != 0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                                break;
                            case EGPhase::release:
                                SetEGRate(my_min(63, rr_ + key_scale_rate_));
                                break;
                        }

                        // SSG-EG
                        if (ssg_type_ != 0 && (eg_phase_ != EGPhase::release))
                        {
                            int m = ar_ >= ((ssg_type_ == 8 || ssg_type_ == 12) ? 56 : 60) ? 1 : 0;

                            //assert(0 <= ssg_phase_ && ssg_phase_ <= 2);
                            const int* table = ssgenvtable[ssg_type_ & 7][m][ssg_phase_];

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
                            tl_ = tl_latch_;
                            if (ssg_type_ != 0)
                            {
                                ssg_phase_ = ssg_phase_ + 1;
                                if (ssg_phase_ > 2)
                                    ssg_phase_ = 1;

                                int m = ar_ >= ((ssg_type_ == 8 || ssg_type_ == 12) ? 56 : 60) ? 1 : 0;

                                //assert(0 <= ssg_phase_ && ssg_phase_ <= 2);
                                const int* table = ssgenvtable[ssg_type_ & 7][m][ssg_phase_];

                                ssg_offset_ = table[0] * 0x200;
                                ssg_vector_ = table[1];
                            }
                            if ((ar_ + key_scale_rate_) < 62)
                            {
                                SetEGRate(ar_ != 0 ? my_min(63, ar_ + key_scale_rate_) : 0);
                                eg_phase_ = EGPhase::attack;
                                break;
                            }

                            if (sl_ != 0)
                            {
                                eg_level_ = 0;
                                eg_level_on_next_phase_ = (int)(ssg_type_ != 0 ? my_min(sl_ * 8, 0x200) : sl_ * 8);

                                SetEGRate(dr_ != 0 ? my_min(63, dr_ + key_scale_rate_) : 0);
                                eg_phase_ = EGPhase::decay;
                                break;
                            }

                            eg_level_ = (int)(sl_ * 8);
                            eg_level_on_next_phase_ = ssg_type_ != 0 ? 0x200 : 0x400;

                            SetEGRate(sr_ != 0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                            eg_phase_ = EGPhase::sustain;
                            break;
                        case EGPhase::decay:         // Decay Phase
                            if (sl_ != 0)
                            {
                                eg_level_ = 0;
                                eg_level_on_next_phase_ = (int)(ssg_type_ != 0 ? my_min(sl_ * 8, 0x200) : sl_ * 8);

                                SetEGRate(dr_ != 0 ? my_min(63, dr_ + key_scale_rate_) : 0);
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
                            eg_level_on_next_phase_ = ssg_type_ != 0 ? 0x200 : 0x400;

                            SetEGRate(sr_ != 0 ? my_min(63, sr_ + key_scale_rate_) : 0);
                            eg_phase_ = EGPhase::sustain;
                            break;

                        case EGPhase::release:       // Release Phase
                            if (ssg_type_ != 0)
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
                uint32_t Sine(int c,int s)
                {
                    return sinetable_opna[c][wt_][((s) >> (20 + FM_PGBITS - FM_OPSINBITS)) & (FM_OPSINENTS - 1)];
                }

                int SINE(int c,int s)
                {
                    return (int)sinetable_opna[c][wt_][(s) & (FM_OPSINENTS - 1)];
                }


                int LogToLin(uint32_t a)
                {
                    //#if 1 // FM_CLENTS < 0xc00		// 400 for TL, 400 for ENV, 400 for LFO.
                    return (a < FM_CLENTS) ? cltable[a] : 0;
                }

                void EGUpdate()
                {
                    if (ssg_type_ == 0)
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
                                ShiftPhase(EGPhase::decay);
                        }
                        EGUpdate();
                    }
                    else
                    {
                        if (ssg_type_ == 0)
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
                                        ShiftPhase(EGPhase::sustain);
                                        break;
                                    case EGPhase::sustain:
                                        ShiftPhase(EGPhase::attack);
                                        break;
                                    case EGPhase::release:
                                        ShiftPhase(EGPhase::off);
                                        break;
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
                int Calc(int ch,int In)
                {
                    EGStep();
                    int In2 = In + out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalc() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    //pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    if (fb_ < 31)
                    {
                        pgin += ((In2 << (int)(1 + IS2EC_SHIFT)) >> (int)fb_) >> (20 + FM_PGBITS - FM_OPSINBITS);
                        //Console.WriteLine("Calc:{0}", pgin);
                    }
                    else
                    {
                        pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    }
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(ch, pgin)));

                    dbgopout_ = out2_;
                    return out2_;
                }

                int CalcL(int ch,int In)
                {
                    EGStep();
                    int In2 = In + out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalcL() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    //pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    if (fb_ < 31)
                    {
                        //                        17                                       19
                        pgin += ((In2 << (int)(1 + IS2EC_SHIFT)) >> (int)fb_) >> (20 + FM_PGBITS - FM_OPSINBITS);
                    }
                    else
                    {
                        //                                      1
                        pgin += In >> (20 + FM_PGBITS - FM_OPSINBITS - (2 + IS2EC_SHIFT));
                    }
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(ch, pgin) + ams_[chip_.GetAML()]));

                    dbgopout_ = out2_;
                    return out2_;
                }

                int CalcN(uint32_t noise)
                {
                    EGStep();

                    int lv = my_max(0, 0x3ff - (tl_out_ + eg_level_)) << 1;

                    noise = (noise & 1) - 1;
                    out_ = (int)((lv + noise) ^ noise);

                    dbgopout_ = out_;
                    return out_;
                }

                //	OP (FB) 計算
                //	Self Feedback の変調最大 = 4π
                int CalcFB(int ch,uint32_t fb)
                {
                    EGStep();

                    int In = out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalc() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    if (fb < 31)
                    {
                        pgin += ((In << (int)(1 + IS2EC_SHIFT)) >> (int)fb) >> (20 + FM_PGBITS - FM_OPSINBITS);
                    }
                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(ch,pgin)));
                    dbgopout_ = out2_;

                    return out2_;
                }

                int CalcFBL(int ch,uint32_t fb)
                {
                    EGStep();

                    int In = out_ + out2_;
                    out2_ = out_;

                    int pgin = (int)(PGCalcL() >> (20 + FM_PGBITS - FM_OPSINBITS));
                    if (fb < 31)
                    {
                        pgin += ((In << (int)(1 + IS2EC_SHIFT)) >> (int)fb) >> (20 + FM_PGBITS - FM_OPSINBITS);
                    }

                    out_ = LogToLin((uint32_t)(eg_out_ + SINE(ch,pgin) + ams_[chip_.GetAML()]));
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
                    if (keyon_) return;

                    keyon_ = true;

                    if (phaseReset_ != 0)
                    {
                        //位相リセットスイッチ有効

                        ShiftPhase(EGPhase::off);
                        ssg_phase_ = -1;
                        ShiftPhase(EGPhase::attack);
                        EGUpdate();
                        in2_ = out_ = out2_ = 0;

                        pg_count_ = 0;

                        return;
                    }

                    //位相リセットスイッチ無効

                    if (eg_phase_ == EGPhase::off || eg_phase_ == EGPhase::release)
                    {
                        ssg_phase_ = -1;
                        ShiftPhase(EGPhase::attack);
                        EGUpdate();
                        in2_ = out_ = out2_ = 0;

                        pg_count_ = 0;
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
                }

                //	DT2 (0-3)
                void SetDT2(uint32_t dt2)
                {
                    detune2_ = dt2 & 3;
                    param_changed_ = true;
                }

                //	Multiple (0-15)
                void SetMULTI(uint32_t mul)
                {
                    multiple_ = mul;
                    param_changed_ = true;
                }

                //	Total Level (0-127) (0.75dB step)
                void SetTL(uint32_t tl, bool csm)
                {
                    if (!csm)
                    {
                        tl_ = tl;
                        param_changed_ = true;
                    }
                    tl_latch_ = tl;
                }

                //	Attack Rate (0-63)
                void SetAR(uint32_t ar)
                {
                    ar_ = ar;
                    param_changed_ = true;
                }

                //	Decay Rate (0-63)
                void SetDR(uint32_t dr)
                {
                    dr_ = dr;
                    param_changed_ = true;
                }

                //	Sustain Rate (0-63)
                void SetSR(uint32_t sr)
                {
                    sr_ = sr;
                    param_changed_ = true;
                }

                //	Sustain Level (0-127)
                void SetSL(uint32_t sl)
                {
                    sl_ = sl;
                    param_changed_ = true;
                }

                //	Release Rate (0-63)
                void SetRR(uint32_t rr)
                {
                    rr_ = rr;
                    param_changed_ = true;
                }

                //	Keyscale (0-3)
                void SetKS(uint32_t ks)
                {
                    ks_ = ks;
                    param_changed_ = true;
                }

                void SetPhaseReset(uint32_t prst)
                {
                    phaseReset_ = (uint32_t)(prst != 0 ? 1 : 0);
                    param_changed_ = true;
                }

                //	SSG-type Envelop (0-15)
                void SetSSGEC(uint32_t ssgec)
                {
                    if ((ssgec & 8) != 0)
                        ssg_type_ = ssgec;
                    else
                        ssg_type_ = 0;
                }

                void SetAMON(bool amon)
                {
                    amon_ = amon;
                    param_changed_ = true;
                }

                void SetFB(uint32_t fb)
                {
                    fb_ = Channel4::fbtable[fb];
                    param_changed_ = true;
                }

                void SetALGLink(uint32_t AlgLink)
                {
                    algLink_ = (uint8_t)AlgLink;
                    param_changed_ = true;
                }

                void SetWaveTypeL(uint8_t wt)
                {
                    wt_ = (uint8_t)((wt_ & 2) | (wt & 1));
                    param_changed_ = true;
                }

                void SetWaveTypeH(uint8_t wt)
                {
                    wt_ = (uint8_t)((wt_ & 1) | ((wt & 1) << 1));
                    param_changed_ = true;
                }

                void Mute(bool mute)
                {
                    mute_ = mute;
                    param_changed_ = true;
                }

                void SetMS(uint32_t ms)
                {
                    ms_ = ms;
                    param_changed_ = true;
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

            private:
                uint32_t bn_;       // Block/Note
                int eg_level_;  // EG の出力値
                int eg_level_on_next_phase_;    // 次の eg_phase_ に移る値
                int eg_count_;      // EG の次の変移までの時間
                int eg_count_diff_; // eg_count_ の差分
                int eg_out_;        // EG+TL を合わせた出力値
                int tl_out_;        // TL 分の出力値
                                            //bool tl_out_;        // TL 分の出力値
                                            //		int		pm_depth_;		// PM depth
                                            //		int		am_depth_;		// AM depth
                int eg_rate_;
                int eg_curve_count_;
                int ssg_offset_;
                int ssg_vector_;
                int ssg_phase_;

                static const int IS2EC_SHIFT = ((20 + FM_PGBITS) - 13);


                uint32_t key_scale_rate_;       // key scale rate
                EGPhase eg_phase_;
                //uint32_t* ams_;
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
                uint32_t phaseReset_;   // phaseReset(0/1)

                bool keyon_;
                bool mute_;

                //	１サンプル合成
                //typedef uint32 Counter;

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
                static uint32_t rate_table[16];
                static uint32_t multable[4][16];

                void SSGShiftPhase(int mode)
                {
                }

                int FBCalc(int fb)
                {
                    return -1;
                }

                //	friends --------------------------------------------------------------
                //private class Channel4;
                void FM_NextPhase(Operator op)
                {
                }

                int* dbgGetClTable()
                {
                    return cltable;
                }

                uint32_t* dbgGetSineTable(int c, int t)
                {
                    return sinetable_opna[c][t];
                }
    };

    static void waveReset(int waveCh, int wavetype)
    {
        double log2 = log(2.0);
        for (int i = 0; i < FM_OPSINENTS / 2; i++)
        {
            double r = (i * 2 + 1) * FM_PI / FM_OPSINENTS;
            double q = -256 * log(sin(r)) / log2;
            uint32_t s = (uint32_t)((int)(floor(q + 0.5)) + 1);
            sinetable_opna[waveCh][wavetype][i] = s * 2;
            sinetable_opna[waveCh][wavetype][FM_OPSINENTS / 2 + i] = s * 2 + 1;
        }
    }

    class Channel4 : fmgen::Channel4
    {
        // ---------------------------------------------------------------------------
        //	4-op Channel
        //
        public:
            static constexpr const uint8_t fbtable[8] = { 31, 7, 6, 5, 4, 3, 2, 1 };

            Operator op[4];

            Channel4(int ch = 0)
            {
                //if (!tablehasmade)
                    MakeTable();

                SetAlgorithm(0);
                pms = pmtable[0][0];
                this->ch = ch;

                for(int i = 0; i < 4; i++)
                {
                    op[i] = fmvgen::Operator();
                }
            }

            void MakeTable()
            {
                // 100/64 cent =  2^(i*100/64*1200)
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
                int key = (op[0].IsOn() | op[1].IsOn() | op[2].IsOn() | op[3].IsOn()) != 0 ? 1 : 0;
                int lfo = (op[0].ms_ & ((op[0].amon_ | op[1].amon_ | op[2].amon_ | op[3].amon_) ? 0x37 : 7)) != 0 ? 2 : 0;
                return key | lfo;
            }

            //	F-Number/BLOCK を設定
            void SetFNum(uint32_t f)
            {
                for (int i = 0; i < 4; i++)
                    op[i].SetFNum(f);
            }

            static constexpr const uint32_t kctable[16] = 
            {
                    5197, 5506, 5833, 6180, 6180, 6547, 6937, 7349,
                    7349, 7786, 8249, 8740, 8740, 9259, 9810, 10394,
            };

            //	KC/KF を設定
            void SetKCKF(uint32_t kc, uint32_t kf)
            {
                int oct = (int)(19 - ((kc >> 4) & 7));

                uint32_t kcv = kctable[kc & 0x0f];
                kcv = (kcv + 2) / 4 * 4;
                uint32_t dp = (uint32_t)(kcv * kftable[kf & 0x3f]);
                dp >>= 16 + 3;
                dp <<= 16 + 3;
                dp >>= oct;
                uint32_t bn = (kc >> 2) & 31;
                op[0].SetDPBN(dp, bn);
                op[1].SetDPBN(dp, bn);
                op[2].SetDPBN(dp, bn);
                op[3].SetDPBN(dp, bn);
            }

            //	キー制御
            void KeyControl(uint32_t key)
            {
                if ((key & 0x1) != 0) op[0].KeyOn();
                else op[0].KeyOff();
                if ((key & 0x2) != 0) op[1].KeyOn();
                else op[1].KeyOff();
                if ((key & 0x4) != 0) op[2].KeyOn();
                else op[2].KeyOff();
                if ((key & 0x8) != 0) op[3].KeyOn();
                else op[3].KeyOff();
            }

            static constexpr const uint8_t table1[8][6] = {
                    { 0, 1, 1, 2, 2, 3 },
                    { 1, 0, 0, 1, 1, 2 },
                    { 1, 1, 1, 0, 0, 2 },
                    { 0, 1, 2, 1, 1, 2 },
                    { 0, 1, 2, 2, 2, 1 },
                    { 0, 1, 0, 1, 0, 1 },
                    { 0, 1, 2, 1, 2, 1 },
                    { 1, 0, 1, 0, 1, 0 },
                };

            //	アルゴリズムを設定
            void SetAlgorithm(uint32_t algo)
            {
                In[0] = table1[algo][0];
                Out[0] = table1[algo][1];
                In[1] = table1[algo][2];
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

                if (!ac)
                {
                    switch (algo_)
                    {
                        case 0:
                            op[2].Calc(ch, op[1].Out());
                            op[1].Calc(ch, op[0].Out());
                            r = op[3].Calc(ch, op[2].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 1:
                            op[2].Calc(ch, op[0].Out() + op[1].Out());
                            op[1].Calc(ch, 0);
                            r = op[3].Calc(ch, op[2].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 2:
                            op[2].Calc(ch, op[1].Out());
                            op[1].Calc(ch, 0);
                            r = op[3].Calc(ch, op[0].Out() + op[2].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 3:
                            op[2].Calc(ch, 0);
                            op[1].Calc(ch, op[0].Out());
                            r = op[3].Calc(ch, op[1].Out() + op[2].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 4:
                            op[2].Calc(ch, 0);
                            r = op[1].Calc(ch, op[0].Out());
                            r += op[3].Calc(ch, op[2].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 5:
                            r = op[2].Calc(ch, op[0].Out());
                            r += op[1].Calc(ch, op[0].Out());
                            r += op[3].Calc(ch, op[0].Out());
                            op[0].CalcFB(ch, fb);
                            break;
                        case 6:
                            r = op[2].Calc(ch, 0);
                            r += op[1].Calc(ch, op[0].Out());
                            r += op[3].Calc(ch, 0);
                            op[0].CalcFB(ch, fb);
                            break;
                        case 7:
                            r = op[2].Calc(ch, 0);
                            r += op[1].Calc(ch, 0);
                            r += op[3].Calc(ch, 0);
                            r += op[0].CalcFB(ch, fb);
                            break;
                    }
                }
                else
                {
                    for(int n = 0; n < 4; n++)
                    {
                        int rn;
                        if (n == 0)
                        {
                            rn = op[n].CalcFB(ch, fb);
                            r += ((carrier & 0x1) != 0) ? rn : 0;
                            continue;
                        }
                        int v = 0;
                        v += ((op[n].algLink_ & 0x1) != 0) ? op[0].Out() : 0;
                        v += ((op[n].algLink_ & 0x2) != 0) ? op[1].Out() : 0;
                        v += ((op[n].algLink_ & 0x4) != 0) ? op[2].Out() : 0;
                        v += ((op[n].algLink_ & 0x8) != 0) ? op[3].Out() : 0;

                        rn = op[n].Calc(ch, v);
                        r += ((carrier & (0x1 << n)) != 0) ? rn : 0;
                    }
                }

                return r;
            }

            //  合成
            int CalcL()
            {
                chip_.SetPMV(pms[chip_.GetPML()]);

                int r = 0;
                if (!ac)
                {
                    switch (algo_)
                    {
                        case 0:
                            op[2].CalcL(ch, op[1].Out());
                            op[1].CalcL(ch, op[0].Out());
                            r = op[3].CalcL(ch, op[2].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 1:
                            op[2].CalcL(ch, op[0].Out() + op[1].Out());
                            op[1].CalcL(ch, 0);
                            r = op[3].CalcL(ch, op[2].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 2:
                            op[2].CalcL(ch, op[1].Out());
                            op[1].CalcL(ch, 0);
                            r = op[3].CalcL(ch, op[0].Out() + op[2].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 3:
                            op[2].CalcL(ch, 0);
                            op[1].CalcL(ch, op[0].Out());
                            r = op[3].CalcL(ch, op[1].Out() + op[2].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 4:
                            op[2].CalcL(ch, 0);
                            r = op[1].CalcL(ch, op[0].Out());
                            r += op[3].CalcL(ch, op[2].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 5:
                            r = op[2].CalcL(ch, op[0].Out());
                            r += op[1].CalcL(ch, op[0].Out());
                            r += op[3].CalcL(ch, op[0].Out());
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 6:
                            r = op[2].CalcL(ch, 0);
                            r += op[1].CalcL(ch, op[0].Out());
                            r += op[3].CalcL(ch, 0);
                            op[0].CalcFBL(ch, fb);
                            break;
                        case 7:
                            r = op[2].CalcL(ch, 0);
                            r += op[1].CalcL(ch, 0);
                            r += op[3].CalcL(ch, 0);
                            r += op[0].CalcFBL(ch, fb);
                            break;
                    }
                }
                else
                {
                    for(int n = 0; n < 4; n++)
                    {
                        int rn;
                        if (n == 0)
                        {
                            rn=op[n].CalcFBL(ch, fb);
                            r += ((carrier & 0x1) != 0) ? rn : 0;
                            continue;
                        }
                        int v = 0;
                        v += ((op[n].algLink_ & 0x1) != 0) ? op[0].Out() : 0;
                        v += ((op[n].algLink_ & 0x2) != 0) ? op[1].Out() : 0;
                        v += ((op[n].algLink_ & 0x4) != 0) ? op[2].Out() : 0;
                        v += ((op[n].algLink_ & 0x8) != 0) ? op[3].Out() : 0;

                        rn = op[n].CalcL(ch, v);
                        r += ((carrier & (0x1 << n)) != 0) ? rn : 0;
                    }
                }

                return r;
            }

            //  合成
            int CalcN(uint32_t noise)
            {
                buf[1] = buf[2] = buf[3] = 0;

                buf[0] = op[0].out_; op[0].CalcFB(ch, fb);
                Out[0] += op[1].Calc(ch, buf[In[0]]);
                Out[1] += op[2].Calc(ch, buf[In[1]]);
                int o = op[3].out_;
                op[3].CalcN(noise);
                return buf[Out[2]] + o;
            }

            //  合成
            int CalcLN(uint32_t noise)
            {
                chip_.SetPMV(pms[chip_.GetPML()]);
                buf[1] = buf[2] = buf[3] = 0;

                buf[0] = op[0].out_; op[0].CalcFBL(ch, fb);
                Out[0] += op[1].CalcL(ch, buf[In[0]]);
                Out[1] += op[2].CalcL(ch, buf[In[1]]);
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

            void SetAC(bool sw)
            {
                ac = sw;
                buildAlg();
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

            void buildAlg()
            {
                uint8_t mask = 0xf;
                uint8_t omask = 0;
                bool use[4] = { false, false, false, false };

                carrier = 0xf;

                for (int j = 0; j < 4; j++)
                {
                    omask = (uint8_t)(~mask);
                    for (int i = 0; i < 4; i++)
                    {
                        if (!use[i] && ( ((op[i].algLink_ & mask) == 0) || i == 0 ))
                        {
                            use[i] = true;
                            omask |= (uint8_t)(1 << i);
                            carrier &= (uint8_t)(~op[i].algLink_);
                        }
                    }
                    mask = (uint8_t)(~omask);
                }
            }

        private:
            //static bool tablehasmade;
            int kftable[64];
            uint32_t fb;
            int buf[4];
            int In[3];          // 各 OP の入力ポインタ
            int Out[3];         // 各 OP の出力ポインタ
            int* pms;
            int algo_;
            Chip chip_;
            bool ac = false;
            uint8_t carrier;
            int oAlg[4] = { 2, 1, 3, 0 };//オペレータの計算順序
            int ch = 0;
    };
};
