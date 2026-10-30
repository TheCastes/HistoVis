#include "core/Image.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "stb_image.h"
#include "stb_image_write.h"

Image::Image(const char *fileName) {
    if ((data = stbi_load(fileName, &width, &height, &channels, 0)) != nullptr) {
        size = width * height * channels;
    } else {
        std::cerr << "Error loading image: " << fileName << "\n";
    }
}

Image::Image(int width, int height, int channels) {
    if (width <= 0 || height <= 0 || channels < 1 || channels > 4) {
        std::cerr << "Error: invalid image size " << width << "x" << height << "x" << channels << "\n";
        return;
    }
    this->width = width;
    this->height = height;
    this->channels = channels;
    size = static_cast<size_t>(width) * height * channels;
    data = static_cast<uint8_t *>(std::calloc(size, 1));
}

std::unique_ptr<Image> Image::clone() const {
    std::unique_ptr<Image> copy = std::make_unique<Image>(width, height, channels);
    if (copy->data != nullptr && data != nullptr) {
        std::memcpy(copy->data, data, size);
    }
    return copy;
}

Image::~Image() {
    if (data != nullptr) {
        stbi_image_free(data);
    }
    data = nullptr;
    width = 0;
    height = 0;
    channels = 0;
    size = 0;
}

bool Image::saveImageJPG(const char *fileName) const {
    return stbi_write_jpg(fileName, width, height, channels, data, 100) != 0;
}

bool Image::saveImagePNG(const char *fileName) const {
    return stbi_write_png(fileName, width, height, channels, data, width * channels) != 0;
}