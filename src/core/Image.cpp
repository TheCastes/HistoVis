#include "core/Image.h"

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

void Image::saveImageJPG(const char *fileName) const {
    stbi_write_jpg(fileName, width, height, channels, data, 100);
}

void Image::saveImagePNG(const char *fileName) const {
    stbi_write_png(fileName, width, height, channels, data, width * channels);
}