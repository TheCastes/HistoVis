#pragma once

#include <array>

#include "Histogram.h"

void applyLUT(Image& image, const std::array<float, 256>& lut, LuminanceSpace space = LuminanceSpace::YCbCr);