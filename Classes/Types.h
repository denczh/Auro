//
//  Types.h
//  Vowels
//
//  Created by Pedro Pinto on 17/06/13.
//  Copyright (c) 2013 Pedro Pinto. All rights reserved.
//

#ifndef Vowels_Types_h
#define Vowels_Types_h

typedef int32_t DATATYPE;
#define NSAMPLES    1024            // Number of samples obtained in one iteration.
#define SAMPRATE    44100.0         // Sampling rate.
#define MAXFREQ     5500.0          // Frequency limit for highest formant.
#define SAFETY      50.0            // Safety margin of 50 Hz.
#define DFACTOR     4               // Decimation factor (downsampling).
#define NFORMANTS   5               // Number of formants.
#define DEG         2*NFORMANTS     // Degree of the polynomial.
#define FTAPS       15              // Filter taps.
#define PREEM       50.0            // Preemphasis.
#define NVOWELS     16              // Number of vowels.

typedef enum {
    F1,
    F2
} formant_t;

// 16 vowels
typedef enum {
    A1,                 // a	850		1610
    A2,                 // ɶ	820		1530
    A3,                 // ɑ	750		940
    A4,                 // ɒ	700		760
    A5,                 // ʌ	600		1170
    E1,                 // e	390		2300
    E2,                 // ø	370		1900
    E3,                 // ɛ	610		1900
    E4,                 // œ	585		1710
    I1,                 // i	240		2400
    I2,                 // y	235		2100
    O1,                 // ɔ	500		700
    O2,                 // ɤ	460		1310
    O3,                 // o	360		640
    U1,                 // ɯ	300		1390
    U2,                 // u	250		595
    X                   // No vowel
} vowel_t;

static const char *const vowels[] = {" ", "a", "ɶ", "ɑ", "ɒ", "ʌ", "e", "ø", "ɛ", "œ", "i", "y", "ɔ", "ɤ", "o", "ɯ", "u"};

static const int formant[16][2] = {
    {850, 1610},        // a
    {820, 1530},        // ɶ
    {750, 940},         // ɑ
    {700, 760},         // ɒ
    {600, 1170},        // ʌ
    {390, 2300},        // e
    {370, 1900},        // ø
    {610, 1900},        // ɛ
    {585, 1710},        // œ
    {240, 2400},        // i
    {235, 2100},        // y
    {500, 700},         // ɔ
    {460, 1310},        // ɤ
    {360, 640},         // o
    {300, 1390},        // ɯ
    {250, 595}          // u
};

static double filter[FTAPS] = {
    -0.016786917004987616, -0.04688118580012321, -0.066205455883452, -0.050683206629742246, 0.027263783225017137,
    0.14576014757303982, 0.2591178731820796, 0.30364832368945754, 0.2591178731820796, 0.14576014757303982,
    0.027263783225017137, -0.050683206629742246, -0.066205455883452, -0.04688118580012321, -0.016786917004987616};

#endif
