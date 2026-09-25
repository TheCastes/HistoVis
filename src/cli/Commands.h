#pragma once

#include <fstream>
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
              << "  hist <input> [-o <file>]  Export the histogram as CSV\n"
              << "\n"
              << "Options:\n"
              << "  -o, --output <file>     Output file (default: stdout)\n"
              << "  -h, --help              Show this help message\n"
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
    Histogram hist(image);
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportHistogram();
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportHistogram(file);
    return 0;
}