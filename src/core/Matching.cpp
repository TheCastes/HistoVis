#include "Matching.h"

#include "LUT.h"

std::array<float, 256> buildMatchingLUT(const std::array<float, 256>& cdfI, const std::array<float, 256>& cdfJ) {
    std::array<float, 256> lut{};
    int z = 0;
    for (int g = 0; g < 256; g++) {
        while (z < 255 && cdfJ[z] < cdfI[g]) {
            z++;
        }
        int lower = z - 1;
        if (lower >= 0 && cdfJ[lower] > 0 && cdfI[g] - cdfJ[lower] <= cdfJ[z] - cdfI[g]) {
            while (lower > 0 && cdfJ[lower - 1] == cdfJ[lower]) {
                lower--;
            }
            lut[g] = lower;
        } else {
            lut[g] = z;
        }
    }
    return lut;
}

void match(Image& image, const Image& reference, LuminanceSpace space) {
    Histogram histI(image, space);
    Histogram histJ(reference, space);
    std::array<float, 256> cdfI;
    std::array<float, 256> cdfJ;
    if (histI.isGreyscale()) {
        cdfI = histI.getCDF(Channel::R);
    } else {
        cdfI = histI.getCDF(Channel::L);
    }
    if (histJ.isGreyscale()) {
        cdfJ = histJ.getCDF(Channel::R);
    } else {
        cdfJ = histJ.getCDF(Channel::L);
    }
    applyLUT(image, buildMatchingLUT(cdfI, cdfJ), space);
}