#pragma once

#include "payment.h"

class DemoPayment : public Payment {
public:
    bool process(long long amountVnd,
                 const std::string& bookingCode) override;
};
