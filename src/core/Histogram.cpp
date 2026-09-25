#include "Histogram.h"

#include <iostream>

Histogram::Histogram(const Image &image) {
    if (!image.isValid()) {
        std::cerr << "Histogram: Error, invalid image\n";
        return;
    }
    greyscale = image.getChannels() == 1;
    if (greyscale) {
        computeGreyscaleHistogram(image);
    } else {
        computeRGBHistogram(image);
    }
}


void Histogram::computeGreyscaleHistogram(const Image &image) {
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < image.getWidth(); j++) {
            for (int k = 0; k < image.getHeight(); k++) {
                Pixel p = image.getPixel(j, k);
                if (p.R == i) {
                    histogram[0][i]++;
                }
            }
        }
    }
}

void Histogram::computeRGBHistogram(const Image &image) {
    for (int i = 0; i < image.getChannels(); i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < image.getWidth(); k++) {
                for (int l = 0; l < image.getHeight(); l++) {
                    Pixel p = image.getPixel(k, l);
                    switch(i) {
                        case 0:
                        if (p.R == j) histogram[0][j]++;
                        break;
                        case 1:
                        if (p.G == j) histogram[1][j]++;
                        break;
                        case 2:
                        if (p.B == j) histogram[2][j]++;
                        break;
                    default: ;
                    }
                }
            }
        }
    }
}

const std::array<int, 256> &Histogram::getHistogram(int channel) const {
    return histogram[channel];
}

void Histogram::printHistogram(int channel) const {
    std::cout << "Histogram for channel " << channel << ":\n";
    for (int i = 0; i < 256; i++) {
        std::cout << i << ": " << histogram[channel][i] << "\n";
    }
}

void Histogram::exportHistogram(std::ostream &out) const {
    out << (greyscale ? "value,count\n" : "value,r,g,b\n");
    for (int i = 0; i < 256; i++) {
        out << i << "," << histogram[0][i];
        if (!greyscale) {
            out << "," << histogram[1][i] << "," << histogram[2][i];
        }
        out << "\n";
    }
}
