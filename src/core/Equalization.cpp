#include "Equalization.h"

#include <cmath>
#include <vector>

#include "ColorSpace.h"

namespace {
    std::vector<uint8_t> computeLuminanceBins(const Image& image, bool greyscale, LuminanceSpace space) {
        int width = image.getWidth();
        int height = image.getHeight();
        std::vector<uint8_t> luminance(width * height, 0);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                Pixel pixel = image.getPixel(j, i);
                int bin;
                if (greyscale) {
                    bin = pixel.R;
                } else {
                    bin = Histogram::luminanceBin(pixel, space);
                }
                luminance[i * width + j] = static_cast<uint8_t>(bin);
            }
        }
        return luminance;
    }

    int mirror(int i, int size) {
        if (i < 0) {
            return -i;
        }
        if (i >= size) {
            return 2 * size - i - 2;
        }
        return i;
    }
} //namespace

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

void equalizeLocal(Image& image, int window, LuminanceSpace space) {
    int width = image.getWidth();
    int height = image.getHeight();
    std::vector<uint8_t> oldLuminance(width * height, 0);
    std::vector<float> equalizedLuminance(width * height, 0);
    std::array<int, 256> tempHist;
    int halfWindow = window / 2;
    if (image.getChannels() < 3) {
        oldLuminance = computeLuminanceBins(image, true, space);
    } else {
        oldLuminance = computeLuminanceBins(image, false, space);
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
           if (j == 0) {
                tempHist = {};
               for (int k = i-halfWindow; k <= i+halfWindow; k++) {
                   for (int l = -halfWindow; l <= +halfWindow; l++) {
                       tempHist[oldLuminance[mirror(k, height) * width + mirror(l, width)]]++;
                   }
               }
           } else {
               int leaving = mirror(j - halfWindow -1, width);
               int entering = mirror(j + halfWindow, width);
               for (int k = i-halfWindow; k <= i+halfWindow; k++) {
                   tempHist[oldLuminance[mirror(k, height)*width + leaving]]--;
                   tempHist[oldLuminance[mirror(k, height)*width + entering]]++;
               }
           }
            int center = oldLuminance[i* width + j];
            int sum = 0;
            for (int k = 0; k <= center; k++) {
                sum += tempHist[k];
            }
            equalizedLuminance[i*width+j] = 255.0f * sum / (window*window);
        }
    }
    if (image.getChannels() < 3) {
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel pixel = image.getPixel(j, i);
                pixel.R = std::lround(equalizedLuminance[i*width+j]);
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
                ycbcrPixel.Y = equalizedLuminance[i*width+j];
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
                labPixel.L = equalizedLuminance[i*width+j] * 100.0f/255.0f;
                Pixel newPixel = ColorSpace::fromLab(labPixel);
                newPixel.A = pixel.A;
                image.setPixel(j, i, newPixel);
            }
        }
    }
}