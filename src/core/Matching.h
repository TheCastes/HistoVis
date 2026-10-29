#pragma once

#include <array>

#include "Histogram.h"

std::array<float, 256> buildMatchingLUT(const std::array<float, 256>& cdfI, const std::array<float, 256>& cdfJ);
void match(Image& image, const Image& reference, LuminanceSpace space = LuminanceSpace::YCbCr);