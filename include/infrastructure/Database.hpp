#pragma once

#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>

#include <pqxx/pqxx>

namespace tradeflow {

class Database {
public:
    static std::string buildConnectionString() {
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");
        const char* dbname = std::getenv("DB_NAME");
        const char* user = std::getenv("DB_USER");
        const char* password = std::getenv("DB_PASSWORD");

        std::string hostValue = host ? host : "localhost";
        std::string portValue = port ? port : "5432";
        std::string dbnameValue = dbname ? dbname : "tradeflow";
        std::string userValue = user ? user : "tradeflow";
        std::string passwordValue = password ? password : "tradeflow";

        return "host=" + hostValue + " port=" + portValue + " dbname=" + dbnameValue +
               " user=" + userValue + " password=" + passwordValue;
    }

    static pqxx::connection connect() {
        auto connectionString = buildConnectionString();
        pqxx::connection connection(connectionString);
        if (!connection.is_open()) {
            throw std::runtime_error("Unable to open PostgreSQL connection");
        }
        return connection;
    }
};

}  // namespace tradeflow
