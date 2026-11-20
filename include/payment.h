#pragma once

#include <string>

class Payment {
public:
    virtual ~Payment();
    virtual bool process(long long amountVnd,
                         const std::string& bookingCode) = 0;
};
