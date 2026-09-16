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
        std::cerr << "Histogram: TODO RGB Images\n";
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

int Histogram::getHistogram(int channel) const {
    return histogram[0][channel];
}

void Histogram::printHistogram(int channel) const {
    std::cout << "Histogram for channel " << channel << ":\n";
    for (int i = 0; i < 256; i++) {
        std::cout << i << ": " << histogram[i][channel] << "\n";
    }
}