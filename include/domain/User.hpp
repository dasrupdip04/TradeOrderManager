#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace tradeflow {

struct User {
    std::int64_t id = -1;
    std::string username;
    std::string email;
    std::chrono::system_clock::time_point createdAt{};
};

}  // namespace tradeflow
