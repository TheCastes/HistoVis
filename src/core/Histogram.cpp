#include "Histogram.h"

#include <cmath>
#include <iostream>

Histogram::Histogram(const Image &image) {
    if (!image.isValid()) {
        std::cerr << "Histogram: Error, invalid image\n";
        return;
    }
    greyscale = image.getChannels() < 3;
    res = image.getRes();
    compute(image);
}

void Histogram::compute(const Image &image) {
    for (int y = 0; y < image.getHeight(); y++) {
        for (int x = 0; x < image.getWidth(); x++) {
            Pixel p = image.getPixel(x, y);
            histogram[index(Channel::R)][p.R]++;
            histogram[index(Channel::G)][p.G]++;
            histogram[index(Channel::B)][p.B]++;
            histogram[index(Channel::L)][std::lround(0.2126 * p.R + 0.7152 * p.G + 0.0722 * p.B)]++; // Y with ITU-R BT.709 constants
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



void Histogram::exportHistogram(std::ostream &out, bool luminance) const {
    if (luminance || greyscale) {
        const std::array<int, 256> &h = histogram[index(luminance ? Channel::L : Channel::R)];
        out << "value,count\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << h[i] << "\n";
        }
        return;
    }
    const std::array<int, 256> &r = histogram[index(Channel::R)];
    const std::array<int, 256> &g = histogram[index(Channel::G)];
    const std::array<int, 256> &b = histogram[index(Channel::B)];
    out << "value,r,g,b\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << r[i] << "," << g[i] << "," << b[i] << "\n";
    }
}

void Histogram::exportPDF(std::ostream &out, bool luminance) const {
    if (luminance || greyscale) {
        std::array<float, 256> p = getPDF(luminance ? Channel::L : Channel::R);
        out << "value,probability\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << p[i] << "\n";
        }
        return;
    }
    std::array<float, 256> r = getPDF(Channel::R);
    std::array<float, 256> g = getPDF(Channel::G);
    std::array<float, 256> b = getPDF(Channel::B);
    out << "value,rprob,gprob,bprob\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << r[i] << "," << g[i] << "," << b[i] << "\n";
    }
}

void Histogram::exportCDF(std::ostream &out, bool luminance) const {
    if (luminance || greyscale) {
        std::array<float, 256> p = getCDF(luminance ? Channel::L : Channel::R);
        out << "value,cumulative\n";
        for (int i = 0; i < 256; i++) {
            out << i << "," << p[i] << "\n";
        }
        return;
    }
    std::array<float, 256> r = getCDF(Channel::R);
    std::array<float, 256> g = getCDF(Channel::G);
    std::array<float, 256> b = getCDF(Channel::B);
    out << "value,rprob,gprob,bprob\n";
    for (int i = 0; i < 256; i++) {
        out << i << "," << r[i] << "," << g[i] << "," << b[i] << "\n";
    }
}
