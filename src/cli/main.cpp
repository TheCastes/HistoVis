#include <iostream>
#include <string>

#include "cli/ArgParser.h"
#include "cli/Commands.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return 0;
    }

    ArgParser args(argc, argv, 2, {"-l", "--luminance", "-h", "--help"});
    if (args.has("-h", "--help")) {
        printUsage(argv[0]);
        return 0;
    }

    if (command == "hist") {
        return cmdHist(args);
    }
    if (command == "pdf") {
        return cmdPdf(args);
    }

    std::cerr << "Error: unknown command: " << command << "\n";
    printUsage(argv[0]);
    return 1;
}