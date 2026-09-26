#include "Equalization.h"

#include <cmath>

#include "ColorSpace.h"

void equalize(Image& image, LuminanceSpace space) {
    Histogram hist(image, space);
    std::array<float, 256> CDF;
    if (hist.isGreyscale()) {
        CDF = hist.getCDF(Channel::R);
    } else {
        CDF = hist.getCDF(Channel::L);
    }
    std::array<float, 256> T;
    for (int i = 0; i < 256; i++) {
        T[i] = 255.0f * CDF[i];
    }
    if (hist.isGreyscale()) {
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel pixel = image.getPixel(j, i);
                pixel.R = std::lround(T[pixel.R]);
                pixel.G = pixel.R;
                pixel.B = pixel.R;
                image.setPixel(j, i, pixel);
            }
        }
    } else if (space == LuminanceSpace::YCbCr) {
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel pixel = image.getPixel(j, i);
                ColorSpace::YCbCr ycbcrPixel = ColorSpace::toYCbCr(pixel);
                ycbcrPixel.Y = T[Histogram::luminanceBin(pixel, space)];
                Pixel newPixel = ColorSpace::fromYCbCr(ycbcrPixel);
                newPixel.A = pixel.A;
                image.setPixel(j, i, newPixel);
            }
        }
    } else {
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel pixel = image.getPixel(j, i);
                ColorSpace::Lab labPixel = ColorSpace::toLab(pixel);
                labPixel.L = T[Histogram::luminanceBin(pixel, space)] * 100.0f/255.0f;
                Pixel newPixel = ColorSpace::fromLab(labPixel);
                newPixel.A = pixel.A;
                image.setPixel(j, i, newPixel);
            }
        }
    }
}
