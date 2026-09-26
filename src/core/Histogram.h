#pragma once

#include <array>
#include <iostream>

#include "Image.h"

enum class Channel { R, G, B, L };

class Histogram {
public:
    Histogram(const Image &image);
    ~Histogram() = default;
    const std::array<int, 256>& getHistogram(Channel channel) const;
    std::array<float, 256> getPDF(Channel channel) const;
    bool isGreyscale() const {
        return greyscale;
    }
    void exportHistogram(std::ostream &out = std::cout, bool luminance = false) const;
    void exportPDF(std::ostream &out = std::cout, bool luminance = false) const;

private:
    void compute(const Image &image);
    static size_t index(Channel channel) {
        return static_cast<size_t>(channel);
    }

    std::array<std::array<int, 256>, 4> histogram{};
    int res = 0;
    bool greyscale = false;
};