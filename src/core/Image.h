#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

class Pixel {
public:
    int R = 0;
    int G = 0;
    int B = 0;
    int A = 0;

    Pixel operator*(int s) const {
        return Pixel{R * s, G * s, B * s, A * s};
    }
    Pixel operator/(int s) const {
        return Pixel{R / s, G / s, B / s, A / s};
    }

    Pixel operator-(const Pixel & pixel) const {
        return Pixel{R - pixel.R, G - pixel.G, B - pixel.B, A - pixel.A};
    }

    Pixel operator+(int s) const {
        return Pixel{R + s, G + s, B + s, A + s};
    }
    Pixel operator+(const Pixel &p) const {
        return Pixel{R + p.R, G + p.G, B + p.B, A + p.A};
    }
};


class Image {
    public:

    explicit Image(const char *fileName);
    //black image in memory, channels from 1 to 4
    Image(int width, int height, int channels);
    ~Image();

    //explicit deep copy, the copy constructor stays deleted to avoid copying by mistake
    std::unique_ptr<Image> clone() const;

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    bool isValid() const {
        return data != nullptr;
    }

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }

    int getChannels() const {
        return channels;
    }

    int getRes() const {
        return width * height;
    }

    const uint8_t *getData() const {
        return data;
    }

    Pixel getPixel(int x, int y) const {
        Pixel p;
        size_t index = (y * width + x) * channels;
        p.R = data[index];
        if (channels >= 3) {
            p.G = data[index+1];
            p.B = data[index+2];
        } else {
            p.G = p.B = p.R;
        }
        if (channels == 2 || channels == 4) {
            p.A = data[index + channels - 1];
        }
        return p;
    }

    void setPixel(int x, int y, const Pixel &p) const {
        size_t index = (y * width + x) * channels;
        data[index] = std::clamp(p.R, 0, 255);
        if (channels >= 3) {
            data[index+1] = std::clamp(p.G, 0, 255);
            data[index+2] = std::clamp(p.B, 0, 255);
        }
        if (channels == 2 || channels == 4) {
            data[index + channels - 1] = std::clamp(p.A, 0, 255);
        }
    }

    bool saveImageJPG(const char *fileName) const;
    bool saveImagePNG(const char *fileName) const;

    void toGreyScale() const {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Pixel p = getPixel(x, y);
                int L = static_cast<int>(0.2126 * p.R + 0.7152 * p.G + 0.0722 * p.B);
                setPixel(x, y, Pixel{L, L, L, p.A});
            }
        }
    }

    private:
    int width{};
    int height{};
    int channels{};
    size_t size{};
    uint8_t *data = nullptr;
};