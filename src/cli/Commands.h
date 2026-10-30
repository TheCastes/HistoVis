#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "cli/ArgParser.h"
#include "core/Equalization.h"
#include "core/Histogram.h"
#include "core/Image.h"
#include "core/Matching.h"

inline void printUsage(const char *prog) {
    std::cout << "Usage: " << prog << " <command> [arguments] [options]\n"
              << "\n"
              << "Histogram toolkit for images.\n"
              << "\n"
              << "Commands:\n"
              << "  hist <input> [-o <file>] [-s <space>] [-l]  Export the histogram as CSV\n"
              << "  pdf  <input> [-o <file>] [-s <space>] [-l]  Export the PDF as CSV\n"
              << "  cdf  <input> [-o <file>] [-s <space>] [-l]  Export the CDF as CSV\n"
              << "  stats <input> [-o <file>] [-s <space>] [-l]  Export histogram stats as CSV\n"
              << "  equalize <input> <output> [-m <mode>] [-w <n>] [-c <clip>] [-s <space>]  Histogram equalization (output: .png, .jpg)\n"
              << "  match <input> <output> -r <ref> [-s <space>]  Histogram matching against a reference image (output: .png, .jpg)\n"
              << "\n"
              << "Options:\n"
              << "  -o, --output <file>       Output file (default: stdout)\n"
              << "  -r, --reference <file>    Reference image for match\n"
              << "  -l, --luminance           Export only the luminance: Y of YCbCr, or L* with -s lab\n"
              << "  -s, --space <space>       hist/pdf/cdf/stats: channels to export, rgb|ycbcr|lab (default: rgb)\n"
              << "                            equalize/match: luminance space, ycbcr|lab (default: ycbcr)\n"
              << "  -m, --mode <mode>         Equalization mode: global, local or clahe (default: global)\n"
              << "  -w, --window <n>          Window size in pixels for local/clahe, odd >= 3 (default: 1/8 of the shorter side)\n"
              << "  -c, --clip <factor>       Clip limit for clahe, >= 1, 1 = no change (default: 2)\n"
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

inline bool parseExport(const ArgParser &args, LuminanceSpace &space, ExportChannels &channels) {
    std::string value = args.get("-s", "--space", "rgb");
    bool luminance = args.has("-l", "--luminance");
    if (value == "rgb") {
        space = LuminanceSpace::YCbCr;
        channels = luminance ? ExportChannels::Luminance : ExportChannels::RGB;
        return true;
    }
    if (value == "ycbcr") {
        space = LuminanceSpace::YCbCr;
        channels = luminance ? ExportChannels::Luminance : ExportChannels::Space;
        return true;
    }
    if (value == "lab") {
        space = LuminanceSpace::Lab;
        channels = luminance ? ExportChannels::Luminance : ExportChannels::Space;
        return true;
    }
    std::cerr << "Error: unknown space: " << value << " (expected rgb, ycbcr or lab)\n";
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
    ExportChannels channels;
    if (!parseExport(args, space, channels)) {
        return 1;
    }
    Histogram hist(image, space);
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportHistogram(std::cout, channels);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportHistogram(file, channels);
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
    ExportChannels channels;
    if (!parseExport(args, space, channels)) {
        return 1;
    }
    Histogram hist(image, space);
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportPDF(std::cout, channels);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportPDF(file, channels);
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
    ExportChannels channels;
    if (!parseExport(args, space, channels)) {
        return 1;
    }
    Histogram hist(image, space);
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportCDF(std::cout, channels);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportCDF(file, channels);
    return 0;
}

inline int cmdStats(const ArgParser &args) {
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
    ExportChannels channels;
    if (!parseExport(args, space, channels)) {
        return 1;
    }
    Histogram hist(image, space);
    std::string output = args.get("-o", "--output");
    if (output.empty()) {
        hist.exportStatistics(std::cout, channels);
        return 0;
    }
    std::ofstream file(output);
    if (!file) {
        std::cerr << "Error: cannot open output file: " << output << "\n";
        return 1;
    }
    hist.exportStatistics(file, channels);
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
    if (mode != "global" && mode != "local" && mode != "clahe") {
        std::cerr << "Error: unknown mode: " << mode << " (expected global, local or clahe)\n";
        return 1;
    }
    bool local = mode == "local" || mode == "clahe";
    float clip = 0;
    if (mode == "clahe") {
        clip = args.getFloat("-c", "--clip", 2.0f);
        if (clip < 1) {
            std::cerr << "Error: clip limit must be >= 1\n";
            return 1;
        }
        if (clip < 1.5f) {
            std::cerr << "Warning: clip limits below 1.5 can be very slow with large windows\n";
        }
    } else if (args.has("-c", "--clip")) {
        std::cerr << "Error: -c is only valid with -m clahe\n";
        return 1;
    }
    bool windowGiven = args.has("-w", "--window");
    int window = args.getInt("-w", "--window", 0);
    if (local && windowGiven && (window < 3 || window % 2 == 0)) {
        std::cerr << "Error: window must be an odd number >= 3\n";
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    if (local && !windowGiven) {
        window = std::min(image.getWidth(), image.getHeight()) / 8;
        window = std::max(3, window | 1);
    }
    if (local && window > std::min(image.getWidth(), image.getHeight())) {
        std::cerr << "Error: window larger than the image (" << image.getWidth() << "x" << image.getHeight() << ")\n";
        return 1;
    }
    if (local) {
        equalizeLocal(image, window, space, clip);
    } else {
        equalize(image, space);
    }
    bool saved = png ? image.saveImagePNG(output.c_str()) : image.saveImageJPG(output.c_str());
    if (!saved) {
        std::cerr << "Error: cannot write output file: " << output << "\n";
        return 1;
    }
    return 0;
}

inline int cmdMatch(const ArgParser &args) {
    const std::string &input = args.positional(0);
    const std::string &output = args.positional(1);
    if (input.empty() || output.empty()) {
        std::cerr << "Error: match requires <input> and <output>\n";
        return 1;
    }
    bool png = output.ends_with(".png");
    bool jpg = output.ends_with(".jpg") || output.ends_with(".jpeg");
    if (!png && !jpg) {
        std::cerr << "Error: output must be .png, .jpg or .jpeg: " << output << "\n";
        return 1;
    }
    std::string referencePath = args.get("-r", "--reference");
    if (referencePath.empty()) {
        std::cerr << "Error: match requires -r <reference>\n";
        return 1;
    }
    LuminanceSpace space;
    if (!parseSpace(args, space)) {
        return 1;
    }
    Image image(input.c_str());
    if (!image.isValid()) {
        return 1;
    }
    Image reference(referencePath.c_str());
    if (!reference.isValid()) {
        return 1;
    }
    match(image, reference, space);
    bool saved = png ? image.saveImagePNG(output.c_str()) : image.saveImageJPG(output.c_str());
    if (!saved) {
        std::cerr << "Error: cannot write output file: " << output << "\n";
        return 1;
    }
    return 0;
}
