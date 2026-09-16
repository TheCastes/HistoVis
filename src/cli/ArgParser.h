#pragma once

#include <map>
#include <string>
#include <vector>

class ArgParser {
public:
    ArgParser(int argc, char *argv[], int start = 1) {
        for (int i = start; i < argc; i++) {
            std::string argument = argv[i];
            if (argument.size() > 1 && argument[0] == '-') {
                std::string key = argument;
                std::string value;
                size_t eq = argument.find('=');
                if (eq != std::string::npos) {
                    key = argument.substr(0, eq);
                    value = argument.substr(eq + 1);
                } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                    value = argv[++i];
                }
                options[key] = value;
            } else {
                positionals.push_back(argument);
            }
        }
    }

    bool has(const std::string &key) const {
        return options.count(key) > 0;
    }

    bool has(const std::string &shortKey, const std::string &longKey) const {
        return has(shortKey) || has(longKey);
    }

    std::string get(const std::string &shortKey, const std::string &longKey, const std::string &fallback = "") const {
        if (has(shortKey)) {
            return value(shortKey, fallback);
        }
        return value(longKey, fallback);
    }

    int getInt(const std::string &shortKey, const std::string &longKey, int fallback) const {
        std::string value = get(shortKey, longKey);
        if (value.empty()) {
            return fallback;
        }
        try {
            return std::stoi(value);
        } catch (...) {
            return fallback;
        }
    }

    const std::string &positional(size_t index) const {
        static const std::string empty;
        return index < positionals.size() ? positionals[index] : empty;
    }

    size_t positionalCount() const {
        return positionals.size();
    }

private:
    std::string value(const std::string &key, const std::string &fallback) const {
        auto it = options.find(key);
        return it != options.end() && !it->second.empty() ? it->second : fallback;
    }

    std::map<std::string, std::string> options;
    std::vector<std::string> positionals;
};