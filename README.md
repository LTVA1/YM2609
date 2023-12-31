# YM2609

An (unfinished) C++ port of the kuma4649's [YM2609 emulator](https://github.com/kuma4649/MDSound) (see the `fmvgen` folder for this particular chip, `fmgen.cs` for the code it is partially based on).

YM2609 (aka OPNA2) is a fantasy (= fictional) sound chip (Yamaha YM2608 (aka OPNA) but better) used by Ryu Umemoto for his PC-98 remixes. He used it by putting together various VSTs in a DAW; however, this implementation is a traditional sound chip emulator.

The chip features 12 FM (OPM + OPN + more functions) channels, 12 PSG (AY8930-like but with more functionality) channels, 6 rhythm sound source, 6 ADPCM-A and 3 ADPCM-B (better sound quality than ADPCM-A) channels. 1st ADPCM-B channel has 256 KiB sample buffer, 2nd and 3rd ones have 16 MiB sample buffers each. ADPCM-A buffer size seems to be custom. Rhythm sound source seems to be identical to YM2608 one, except they use DSP effects as well.

The chip also features DSP effects chain (equalizer, compressor, LP/HP filters, reverb, chorus, and left/right phase inversion) which can be applied to each channel independently, however, reverb has some global settings, and equalizer is global. There also is yet another compressor which is global and can be applied as the very last effect in the chain.

The full spec is [here](https://github.com/kuma4649/MDSound/blob/master/YM2609.txt)
