#include "ColorSpace.h"

#include <algorithm>
#include <cmath>

namespace {

    //D65 Standard Illuminant
    const float Xn = 0.95047f;
    const float Yn = 1.0f;
    const float Zn = 1.088840f;

    float Labft(float t) {
        float sigma = 6.0f / 29.0f;
        if (t > std::pow(sigma,3)) {
            return std::cbrt(t);
        }
        return (1.0f/3.0f)*std::pow(sigma, -2) * t + 4.0f / 29.0f;
    }

    float inverseLabft(float t) {
        float sigma = 6.0f / 29.0f;
        if (t > sigma) {
            return std::pow(t, 3);
        }
        return (3.0f*sigma*sigma) * (t - 4.0f / 29.0f);
    }

    float srgbToLinear(float srgb) {
        if (srgb <= 0.04045f) {
            return srgb / 12.92f;
        }
        return std::pow((srgb + 0.055f) / 1.055f, 2.4f);
    }

    float linearToSRGB(float linear) {
        if (linear <= 0.0031308f) {
            return 12.92f * linear;
        }
        return 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
    }
} //namespace

//values took from standard ITU-R BT.709
ColorSpace::YCbCr ColorSpace::toYCbCr(const Pixel& pixel) {
    YCbCr ycbcrPixel{};
    ycbcrPixel.Y  =  0.2126f * pixel.R + 0.7152f * pixel.G + 0.0722f * pixel.B;
    ycbcrPixel.Cb = -0.1146f * pixel.R - 0.3854f * pixel.G + 0.5f    * pixel.B;
    ycbcrPixel.Cr =  0.5f    * pixel.R - 0.4542f * pixel.G - 0.0458f * pixel.B;
    return ycbcrPixel;
}

Pixel ColorSpace::fromYCbCr(const YCbCr& ycbcr) {
    Pixel pixel;
    pixel.R = std::clamp(static_cast<int>(std::lround(ycbcr.Y + 1.5748f * ycbcr.Cr)), 0, 255);
    pixel.G = std::clamp(static_cast<int>(std::lround(ycbcr.Y - 0.1873f * ycbcr.Cb - 0.4681f * ycbcr.Cr)), 0, 255);
    pixel.B = std::clamp(static_cast<int>(std::lround(ycbcr.Y + 1.8556f * ycbcr.Cb)), 0, 255);
    return pixel;
}

ColorSpace::Lab ColorSpace::toLab(const Pixel& pixel) {
    Lab labPixel{};

    float R = srgbToLinear(pixel.R / 255.0f);
    float G = srgbToLinear(pixel.G / 255.0f);
    float B = srgbToLinear(pixel.B / 255.0f);


    float X = 0.4124f * R + 0.3576f * G + 0.1805f * B;
    float Y = 0.2126f * R + 0.7152f * G + 0.0722f * B;
    float Z = 0.0193f * R + 0.1192f * G + 0.9505f * B;

    labPixel.L = 116.0f * Labft(Y/Yn) - 16.0f;
    labPixel.a = 500.0f * (Labft(X/Xn) - Labft(Y/Yn));
    labPixel.b = 200.0f * (Labft(Y/Yn) - Labft(Z/Zn));

    return labPixel;

}

Pixel ColorSpace::fromLab(const Lab& lab) {
    Pixel pixel;

    float X = Xn*inverseLabft(((lab.L + 16.0f) / 116.0f)+lab.a/500.0f);
    float Y = Yn*inverseLabft((lab.L + 16.0f) / 116.0f);
    float Z = Zn*inverseLabft(((lab.L + 16.0f) / 116.0f)-lab.b/200.0f);

    float R = 3.2406f * X - 1.5372f * Y - 0.4986f * Z;
    float G = -0.9689f * X + 1.8758f * Y + 0.0415f * Z;
    float B = 0.0557f * X - 0.2040f * Y + 1.0570f * Z;

    pixel.R = std::clamp(static_cast<int>(std::lround(linearToSRGB(R)*255.0f)),0,255);
    pixel.G = std::clamp(static_cast<int>(std::lround(linearToSRGB(G)*255.0f)),0,255);
    pixel.B = std::clamp(static_cast<int>(std::lround(linearToSRGB(B)*255.0f)),0,255);

    return pixel;
}