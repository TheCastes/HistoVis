#pragma once

#include "Image.h"

class Histogram {
public:
    Histogram(const Image &image);
    ~Histogram() = default;
    int getHistogram(int channel) const;
    void printHistogram(int channel) const;
private:
    void computeGreyscaleHistogram(const Image &image);
    void computeRGBHistogram(const Image &image);
    int histogram[256][3]{};
};