#pragma once

#include <iostream>
#include <string>

#include "cli/ArgParser.h"
#include "core/Histogram.h"
#include "core/Image.h"

inline void printUsage(const char *prog) {
    std::cout << "Usage: " << prog << " <command> [arguments] [options]\n"
              << "\n"
              << "Histogram toolkit for images.\n"
              << "\n"
              << "Commands:\n"
              << "  hist     <input>                     Compute the histogram\n"
              << "\n"
              << "Options:\n"
              << "  -h, --help               Show this help message\n"
              << "\n";
}

inline bool loadOrFail(const std::string &path, Image &image) {
    if (path.empty()) {
        std::cerr << "Error: missing input image\n";
        return false;
    }
    if (!image.isValid()) {
        return false;
    }
    return true;
}

inline int cmdHist(const ArgParser &args) {
    Image image(args.positional(0).c_str());
    if (!loadOrFail(args.positional(0), image)) {
        return 1;
    }
    std::string channel = args.get("-c", "--channel", "r");
    Histogram hist(image);
    for (int i = 0; i < image.getChannels(); i++) {
        hist.printHistogram(i);
    }
    return 0;
}