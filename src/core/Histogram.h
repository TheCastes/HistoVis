#pragma once

#include <array>
#include <iostream>

#include "Image.h"

class Histogram {
public:
    Histogram(const Image &image);
    ~Histogram() = default;
    const std::array<int, 256> &getHistogram(int channel) const;
    void printHistogram(int channel) const;
    void exportHistogram(std::ostream &out = std::cout) const;
private:
    void computeGreyscaleHistogram(const Image &image);
    void computeRGBHistogram(const Image &image);
    std::array<std::array<int, 256>, 3> histogram{};
    bool greyscale = false;
};