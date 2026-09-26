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
              << "  hist <input> [-o <file>] [-l]  Export the histogram as CSV\n"
              << "  pdf  <input> [-o <file>] [-l]  Export the PDF as CSV\n"
              << "  cdf  <input> [-o <file>] [-l]  Export the CDF as CSV\n"
              << "\n"
              << "Options:\n"
              << "  -o, --output <file>       Output file (default: stdout)\n"
              << "  -l, --luminance           Export luminance instead of the channels\n"
              << "  -h, --help                Show this help message\n"
              << "\n";
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
    Histogram hist(image);
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
    Histogram hist(image);
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

    Histogram hist(image);
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