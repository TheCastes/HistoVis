#include "LUT.h"

#include <cmath>

#include "ColorSpace.h"

void applyLUT(Image& image, const std::array<float, 256>& lut, LuminanceSpace space) {
    if (image.getChannels() < 3) {
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel pixel = image.getPixel(j, i);
                pixel.R = std::lround(lut[pixel.R]);
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
                ycbcrPixel.Y = lut[Histogram::luminanceBin(pixel, space)];
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
                labPixel.L = lut[Histogram::luminanceBin(pixel, space)] * 100.0f/255.0f;
                Pixel newPixel = ColorSpace::fromLab(labPixel);
                newPixel.A = pixel.A;
                image.setPixel(j, i, newPixel);
            }
        }
    }
}