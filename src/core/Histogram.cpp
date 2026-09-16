#include "Histogram.h"

#include <iostream>

Histogram::Histogram(const Image &image) {
    if (!image.isValid()) {
        std::cerr << "Histogram: Error, invalid image\n";
        return;
    }
    if (image.getChannels() == 1) {
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
                    histogram[i][0]++;
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
                        if (p.R == j) histogram[j][0]++;
                        break;
                        case 1:
                        if (p.G == j) histogram[j][1]++;
                        break;
                        case 2:
                        if (p.B == j) histogram[j][2]++;
                        break;
                    default: ;
                    }
                }
            }
        }
    }
}

int Histogram::getHistogram(int channel) const {
    return histogram[0][channel];
}

void Histogram::printHistogram(int channel) const {
    std::cout << "Histogram for channel " << channel << ":\n";
    for (int i = 0; i < 256; i++) {
        std::cout << i << ": " << histogram[i][channel] << "\n";
    }
}