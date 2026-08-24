#pragma once

#include <iostream>
#include <string>

namespace tradeflow {

class Logger {
public:
    virtual ~Logger() = default;

    virtual void info(const std::string& message) const {
        std::cout << "[INFO] " << message << std::endl;
    }

    virtual void warn(const std::string& message) const {
        std::cout << "[WARN] " << message << std::endl;
    }

    virtual void error(const std::string& message) const {
        std::cerr << "[ERROR] " << message << std::endl;
    }
};

}  // namespace tradeflow
