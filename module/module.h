#pragma once
#include <string>
#include <utility>

class module {
protected:
    std::string name;

public:
    bool status = false;
    int minCPS = 10;
    int maxCPS = 15;
    int toggleKey = VK_F6;

    explicit module(std::string n)
        : name(std::move(n)) {}

    virtual ~module() = default;

    [[nodiscard]] bool getStatus() const {
        return status;
    }

    void setStatus(bool v) {
        status = v;
    }
};