#pragma once

#include <cmath>
#include <stdint.h>
#include <stdbool.h>

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

float panTable_opna[4] = { 1.0f, 0.7512f, 0.4512f, 0.0500f };
uint32_t sinetable_opna[12][4][1024];

int amtable_opna[FM_LFOENTS];
int pmtable_opna[FM_LFOENTS];
int tltable_opna[FM_TLENTS + FM_TLPOS];