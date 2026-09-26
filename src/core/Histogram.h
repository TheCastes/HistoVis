#pragma once

#include <array>
#include <iostream>

#include "Image.h"

enum class Channel { R, G, B, L };
enum class LuminanceSpace { YCbCr, Lab };

class Histogram {
public:
    Histogram(const Image &image, LuminanceSpace space = LuminanceSpace::YCbCr);
    static int luminanceBin(const Pixel& p, LuminanceSpace space);
    ~Histogram() = default;
    const std::array<int, 256>& getHistogram(Channel channel) const;
    std::array<float, 256> getPDF(Channel channel) const;
    std::array<float, 256> getCDF(Channel channel) const;
    LuminanceSpace getLuminanceSpace() const {
        return space;
    };

    bool isGreyscale() const {
        return greyscale;
    }
    void exportHistogram(std::ostream &out = std::cout, bool luminance = false) const;
    void exportPDF(std::ostream &out = std::cout, bool luminance = false) const;
    void exportCDF(std::ostream &out = std::cout, bool luminance = false) const;

private:
    void compute(const Image &image);
    static size_t index(Channel channel) {
        return static_cast<size_t>(channel);
    }

    std::array<std::array<int, 256>, 4> histogram{};
    int res = 0;
    bool greyscale = false;
    LuminanceSpace space = LuminanceSpace::YCbCr;
};