#pragma once

#include <array>
#include <iostream>
#include <string>

#include "Image.h"

enum class Channel { R, G, B, L, C1, C2 };
enum class LuminanceSpace { YCbCr, Lab };
enum class ExportChannels { RGB, Space, Luminance };

struct Statistics {
    int min = 0;
    int max = 0;
    int range = 0;
    int p1 = 0;
    int p99 = 0;
    int percentileRange = 0;
    double mean = 0;
    int median = 0;
    int mode = 0;
    double stdDev = 0;
    int p25 = 0;
    int p75 = 0;
    int iqr = 0;
};

class Histogram {
public:
    Histogram(const Image &image, LuminanceSpace space = LuminanceSpace::YCbCr);
    static int luminanceBin(const Pixel& p, LuminanceSpace space);
    static std::array<int, 3> spaceBins(const Pixel& p, LuminanceSpace space);
    ~Histogram() = default;
    const std::array<int, 256>& getHistogram(Channel channel) const;
    std::array<float, 256> getPDF(Channel channel) const;
    std::array<float, 256> getCDF(Channel channel) const;
    Statistics getStatistics(Channel channel) const;
    LuminanceSpace getLuminanceSpace() const {
        return space;
    };

    bool isGreyscale() const {
        return greyscale;
    }
    void exportHistogram(std::ostream &out = std::cout, ExportChannels channels = ExportChannels::RGB) const;
    void exportPDF(std::ostream &out = std::cout, ExportChannels channels = ExportChannels::RGB) const;
    void exportCDF(std::ostream &out = std::cout, ExportChannels channels = ExportChannels::RGB) const;
    void exportStatistics(std::ostream &out = std::cout, ExportChannels channels = ExportChannels::RGB) const;

private:
    void compute(const Image &image);
    static size_t index(Channel channel) {
        return static_cast<size_t>(channel);
    }
    std::string columnNames(ExportChannels channels) const;
    int percentile(const std::array<int, 256> &h, double q) const;

    std::array<std::array<int, 256>, 6> histogram{};
    int res = 0;
    bool greyscale = false;
    LuminanceSpace space = LuminanceSpace::YCbCr;
};