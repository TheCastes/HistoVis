#pragma once

#include "Histogram.h"

void equalize(Image& image, LuminanceSpace space = LuminanceSpace::YCbCr);
//clipFactor 0: local equalization, clipFactor >= 1: CLAHE
void equalizeLocal(Image& image, int window, LuminanceSpace space = LuminanceSpace::YCbCr, float clipFactor = 0);