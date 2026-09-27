#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "cli/ArgParser.h"
#include "core/Equalization.h"
#include "core/Histogram.h"
#include "core/Image.h"

inline void printUsage(const char *prog) {
    std::cout << "Usage: " << prog << " <command> [arguments] [options]\n"
              << "\n"
              << "Histogram toolkit for images.\n"
              << "\n"
              << "Commands:\n"
              << "  hist <input> [-o <file>] [-l]  Export the histogram as CSV\n"
              << "  pdf  <input> [-o <file>] [-l]  Export the PDF as CSV\n"
              << "  cdf  <input> [-o <file>] [-l]  Export the CDF as CSV\n"
              << "  equalize <input> <output> [-m <mode>] [-w <n>] [-s <space>]  Histogram equalization (output: .png, .jpg)\n"
              << "\n"
              << "Options:\n"
              << "  -o, --output <file>       Output file (default: stdout)\n"
              << "  -l, --luminance           Export luminance instead of the channels\n"
              << "  -s, --space <ycbcr|lab>   Luminance space: Y of YCbCr or L* of Lab (default: ycbcr)\n"
              << "  -m, --mode <global|local> Equalization mode (default: global)\n"
              << "  -w, --window <n>          Window size in pixels for local mode, odd >= 3 (default: 1/8 of the shorter side)\n"
              << "  -h, --help                Show this help message\n"
              << "\n";
}

inline bool parseSpace(const ArgParser &args, LuminanceSpace &space) {
    std::string value = args.get("-s", "--space", "ycbcr");
    if (value == "ycbcr") {
        space = LuminanceSpace::YCbCr;
        return true;
    }
    if (value == "lab") {
        space = LuminanceSpace::Lab;
        return true;
    }
    std::cerr << "Error: unknown space: " << value << " (expected ycbcr or lab)\n";
    return false;
}

inline int cmdHist(const ArgParser &args) {
    const std::string &input = args.positional(0);
    if (input.empty()) {
        std::cerr << "Error: missing input image\n";
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    LuminanceSpace space;
    if (!parseSpace(args, space)) {
        return 1;
    }
    Histogram hist(image, space);
    bool luminance = args.has("-l", "--luminance");
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportHistogram(std::cout, luminance);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportHistogram(file, luminance);
    return 0;
}

inline int cmdPdf(const ArgParser &args) {
    const std::string &input = args.positional(0);
    if (input.empty()) {
        std::cerr << "Error: missing input image\n";
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    LuminanceSpace space;
    if (!parseSpace(args, space)) {
        return 1;
    }
    Histogram hist(image, space);
    bool luminance = args.has("-l", "--luminance");
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportPDF(std::cout, luminance);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportPDF(file, luminance);
    return 0;
}

inline int cmdCdf(const ArgParser &args) {
    const std::string &input = args.positional(0);
    if (input.empty()) {
        std::cerr << "Error: missing input image\n";
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    LuminanceSpace space;
    if (!parseSpace(args, space)) {
        return 1;
    }

    Histogram hist(image,space);
    bool luminance = args.has("-l", "--luminance");
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportCDF(std::cout, luminance);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportCDF(file, luminance);
    return 0;
}

inline int cmdEqualize(const ArgParser &args) {
    const std::string &input = args.positional(0);
    const std::string &output = args.positional(1);
    if (input.empty() || output.empty()) {
        std::cerr << "Error: equalize requires <input> and <output>\n";
        return 1;
    }
    bool png = output.ends_with(".png");
    bool jpg = output.ends_with(".jpg") || output.ends_with(".jpeg");
    if (!png && !jpg) {
        std::cerr << "Error: output must be .png, .jpg or .jpeg: " << output << "\n";
        return 1;
    }
    LuminanceSpace space;
    if (!parseSpace(args, space)) {
        return 1;
    }
    std::string mode = args.get("-m", "--mode", "global");
    if (mode != "global" && mode != "local") {
        std::cerr << "Error: unknown mode: " << mode << " (expected global or local)\n";
        return 1;
    }
    bool windowGiven = args.has("-w", "--window");
    int window = args.getInt("-w", "--window", 0);
    if (mode == "local" && windowGiven && (window < 3 || window % 2 == 0)) {
        std::cerr << "Error: window must be an odd number >= 3\n";
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    if (mode == "local" && !windowGiven) {
        window = std::min(image.getWidth(), image.getHeight()) / 8;
        window = std::max(3, window | 1);
    }
    if (mode == "local" && window > std::min(image.getWidth(), image.getHeight())) {
        std::cerr << "Error: window larger than the image (" << image.getWidth() << "x" << image.getHeight() << ")\n";
        return 1;
    }
    if (mode == "local") {
        equalizeLocal(image, window, space);
    } else {
        equalize(image, space);
    }
    if (png) {
        image.saveImagePNG(output.c_str());
    } else {
        image.saveImageJPG(output.c_str());
    }
    return 0;
}
