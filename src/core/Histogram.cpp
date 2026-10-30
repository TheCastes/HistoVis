#include "Histogram.h"

#include <cmath>
#include <iomanip>
#include <iostream>

#include "ColorSpace.h"



Histogram::Histogram(const Image &image, LuminanceSpace space) {
    if (!image.isValid()) {
        std::cerr << "Histogram: Error, invalid image\n";
        return;
    }
    greyscale = image.getChannels() < 3;
    res = image.getRes();
    this->space = space;
    compute(image);
}

int Histogram::luminanceBin(const Pixel &p, LuminanceSpace space) {
    return spaceBins(p, space)[0];
}

std::array<int, 3> Histogram::spaceBins(const Pixel &p, LuminanceSpace space) {
    std::array<int, 3> bins{};
    if (space == LuminanceSpace::Lab) {
        ColorSpace::Lab lab = ColorSpace::toLab(p);
        bins[0] = std::lround(lab.L * 255.0f / 100.0f);
        bins[1] = std::lround(lab.a + 128.0f);
        bins[2] = std::lround(lab.b + 128.0f);
    } else {
        ColorSpace::YCbCr ycbcr = ColorSpace::toYCbCr(p);
        bins[0] = std::lround(ycbcr.Y);
        bins[1] = std::lround(ycbcr.Cb + 128.0f);
        bins[2] = std::lround(ycbcr.Cr + 128.0f);
    }
    for (int i = 0; i < 3; i++) {
        bins[i] = std::clamp(bins[i], 0, 255);
    }
    return bins;
}

void Histogram::compute(const Image &image) {
    for (int y = 0; y < image.getHeight(); y++) {
        for (int x = 0; x < image.getWidth(); x++) {
            Pixel p = image.getPixel(x, y);
            std::array<int, 3> bins = spaceBins(p, space);
            histogram[index(Channel::R)][p.R]++;
            histogram[index(Channel::G)][p.G]++;
            histogram[index(Channel::B)][p.B]++;
            histogram[index(Channel::L)][bins[0]]++;
            histogram[index(Channel::C1)][bins[1]]++;
            histogram[index(Channel::C2)][bins[2]]++;
        }
    }
}

const std::array<int, 256>& Histogram::getHistogram(Channel channel) const {
    return histogram[index(channel)];
}

std::array<float, 256> Histogram::getPDF(Channel channel) const {
    std::array<float, 256> pdf{};
    if (res == 0) {
        return pdf;
    }
    for (int i = 0; i < 256; i++) {
        pdf[i] = static_cast<float>(histogram[index(channel)][i]) / res;
    }
    return pdf;
}

std::array<float, 256> Histogram::getCDF(Channel channel) const {
    std::array<float, 256> cdf{};
    if (res == 0) {
        return cdf;
    }
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += histogram[index(channel)][i];
        cdf[i] = static_cast<float>(sum) / res;
    }
    return cdf;
}

Statistics Histogram::getStatistics(Channel channel) const {
    Statistics stats;
    if (res == 0) {
        return stats;
    }
    const std::array<int, 256> &h = histogram[index(channel)];

    stats.min = 0;
    while (h[stats.min] == 0) {
        stats.min++;
    }
    stats.max = 255;
    while (h[stats.max] == 0) {
        stats.max--;
    }
    stats.range = stats.max - stats.min;

    double sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += static_cast<double>(i) * h[i];
        if (h[i] > h[stats.mode]) {
            stats.mode = i;
        }
    }
    stats.mean = sum / res;
    double squares = 0;
    for (int i = 0; i < 256; i++) {
        squares += (i - stats.mean) * (i - stats.mean) * h[i];
    }
    stats.stdDev = std::sqrt(squares / res);

    stats.p1 = percentile(h, 0.01);
    stats.p25 = percentile(h, 0.25);
    stats.median = percentile(h, 0.5);
    stats.p75 = percentile(h, 0.75);
    stats.p99 = percentile(h, 0.99);
    stats.percentileRange = stats.p99 - stats.p1;
    stats.iqr = stats.p75 - stats.p25;
    return stats;
}

int Histogram::percentile(const std::array<int, 256> &h, double q) const {
    double target = q * res;
    int cumulative = 0;
    for (int i = 0; i < 256; i++) {
        cumulative += h[i];
        if (cumulative >= target) {
            return i;
        }
    }
    return 255;
}

std::string Histogram::columnNames(ExportChannels channels) const {
    if (channels == ExportChannels::RGB) {
        return "r,g,b";
    }
    if (space == LuminanceSpace::Lab) {
        return "l,a,b";
    }
    return "y,cb,cr";
}

void Histogram::exportHistogram(std::ostream &out, ExportChannels channels) const {
    if (channels == ExportChannels::Luminance || greyscale) {
        const std::array<int, 256> &h = histogram[index(channels == ExportChannels::Luminance ? Channel::L : Channel::R)];
        out << "value,count\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << h[i] << "\n";
        }
        return;
    }
    bool rgb = channels == ExportChannels::RGB;
    const std::array<int, 256> &c1 = histogram[index(rgb ? Channel::R : Channel::L)];
    const std::array<int, 256> &c2 = histogram[index(rgb ? Channel::G : Channel::C1)];
    const std::array<int, 256> &c3 = histogram[index(rgb ? Channel::B : Channel::C2)];
    out << "value," << columnNames(channels) << "\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << c1[i] << "," << c2[i] << "," << c3[i] << "\n";
    }
}

void Histogram::exportPDF(std::ostream &out, ExportChannels channels) const {
    if (channels == ExportChannels::Luminance || greyscale) {
        std::array<float, 256> p = getPDF(channels == ExportChannels::Luminance ? Channel::L : Channel::R);
        out << "value,probability\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << p[i] << "\n";
        }
        return;
    }
    bool rgb = channels == ExportChannels::RGB;
    std::array<float, 256> c1 = getPDF(rgb ? Channel::R : Channel::L);
    std::array<float, 256> c2 = getPDF(rgb ? Channel::G : Channel::C1);
    std::array<float, 256> c3 = getPDF(rgb ? Channel::B : Channel::C2);
    out << "value," << columnNames(channels) << "\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << c1[i] << "," << c2[i] << "," << c3[i] << "\n";
    }
}

void Histogram::exportCDF(std::ostream &out, ExportChannels channels) const {
    if (channels == ExportChannels::Luminance || greyscale) {
        std::array<float, 256> p = getCDF(channels == ExportChannels::Luminance ? Channel::L : Channel::R);
        out << "value,cumulative\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << p[i] << "\n";
        }
        return;
    }
    bool rgb = channels == ExportChannels::RGB;
    std::array<float, 256> c1 = getCDF(rgb ? Channel::R : Channel::L);
    std::array<float, 256> c2 = getCDF(rgb ? Channel::G : Channel::C1);
    std::array<float, 256> c3 = getCDF(rgb ? Channel::B : Channel::C2);
    out << "value," << columnNames(channels) << "\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << c1[i] << "," << c2[i] << "," << c3[i] << "\n";
    }
}

void Histogram::exportStatistics(std::ostream &out, ExportChannels channels) const {
    std::array<std::string, 3> names = {"r", "g", "b"};
    std::array<Channel, 3> rows = {Channel::R, Channel::G, Channel::B};
    int rowCount = 3;
    if (channels == ExportChannels::Luminance) {
        names[0] = space == LuminanceSpace::Lab ? "l" : "y";
        rows[0] = Channel::L;
        rowCount = 1;
    } else if (greyscale) {
        names[0] = "grey";
        rows[0] = Channel::R;
        rowCount = 1;
    } else if (channels == ExportChannels::Space) {
        names = space == LuminanceSpace::Lab ? std::array<std::string, 3>{"l", "a", "b"}
                                             : std::array<std::string, 3>{"y", "cb", "cr"};
        rows = {Channel::L, Channel::C1, Channel::C2};
    }

    std::ios::fmtflags flags = out.flags();
    std::streamsize precision = out.precision();
    out << std::fixed << std::setprecision(2);
    out << "channel,min,max,range,p1,p99,range_p1_p99,mean,median,mode,std,p25,p75,iqr\n";
    for (int r = 0; r < rowCount; r++) {
        Statistics s = getStatistics(rows[r]);
        out << names[r] << "," << s.min << "," << s.max << "," << s.range << "," << s.p1 << "," << s.p99 << ","
            << s.percentileRange << "," << s.mean << "," << s.median << "," << s.mode << "," << s.stdDev << ","
            << s.p25 << "," << s.p75 << "," << s.iqr << "\n";
    }
    out.flags(flags);
    out.precision(precision);
}