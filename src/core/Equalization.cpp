#include "Equalization.h"

#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

#include "ColorSpace.h"
#include "LUT.h"

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

    void clipHistogram(std::array<int, 256>& histogram, int clipLimit) {
        int excess = 0;
        for (int i = 0; i < 256; i++) {
            if (histogram[i] > clipLimit) {
                excess += histogram[i] - clipLimit;
            }
        }
        int binIncr = excess / 256;
        int upper = clipLimit - binIncr;
        for (int i = 0; i < 256; i++) {
            if (histogram[i] > clipLimit) {
                histogram[i] = clipLimit;
            } else if (histogram[i] > upper) {
                excess -= clipLimit - histogram[i];
                histogram[i] = clipLimit;
            } else {
                excess -= binIncr;
                histogram[i] += binIncr;
            }
        }
        while (excess > 0) {
            for (int start = 0; start < 256 && excess > 0; start++) {
                int step = std::max(1, 256 / excess);
                for (int i = start; i < 256 && excess > 0; i += step) {
                    if (histogram[i] < clipLimit) {
                        histogram[i]++;
                        excess--;
                    }
                }
            }
        }
    }

   
    void equalizeRows(const std::vector<uint8_t>& oldLuminance, std::vector<float>& equalizedLuminance,
                      int width, int height, int window, float clipFactor, int clipLimit, int firstRow, int lastRow) {
        std::array<int, 256> tempHist;
        std::array<int, 256> clippedHist;
        int halfWindow = window / 2;
        for (int i = firstRow; i < lastRow; i++) {
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
                if (clipFactor > 0) {
                    clippedHist = tempHist;
                    clipHistogram(clippedHist, clipLimit);
                    for (int k = 0; k <= center; k++) {
                        sum += clippedHist[k];
                    }
                } else {
                    for (int k = 0; k <= center; k++) {
                        sum += tempHist[k];
                    }
                }
                equalizedLuminance[i*width+j] = 255.0f * sum / (window*window);
            }
        }
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
    applyLUT(image, T, space);
}

void equalizeLocal(Image& image, int window, LuminanceSpace space, float clipFactor) {
    int width = image.getWidth();
    int height = image.getHeight();
    std::vector<uint8_t> oldLuminance(width * height, 0);
    std::vector<float> equalizedLuminance(width * height, 0);
    int windowPixels = window * window;
    int clipLimit = std::max(static_cast<int>(clipFactor * windowPixels / 256), (windowPixels + 255) / 256);
    if (image.getChannels() < 3) {
        oldLuminance = computeLuminanceBins(image, true, space);
    } else {
        oldLuminance = computeLuminanceBins(image, false, space);
    }
    int threadCount = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    threadCount = std::min(threadCount, height);
    {
        std::vector<std::jthread> threads;
        for (int t = 0; t < threadCount; t++) {
            int firstRow = height * t / threadCount;
            int lastRow = height * (t + 1) / threadCount;
            threads.emplace_back([&oldLuminance, &equalizedLuminance, width, height, window, clipFactor, clipLimit,
                                  firstRow, lastRow]() {
                equalizeRows(oldLuminance, equalizedLuminance, width, height, window, clipFactor, clipLimit,
                             firstRow, lastRow);
            });
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