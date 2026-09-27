#pragma once

#include "Histogram.h"

void equalize(Image& image, LuminanceSpace space = LuminanceSpace::YCbCr);
void equalizeLocal(Image& image, int window, LuminanceSpace space = LuminanceSpace::YCbCr);