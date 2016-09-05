#ifndef UTILITY_H
#define UTILITY_H

#include "utility_global.h"
#include <cmath>
class UTILITYSHARED_EXPORT Utility
{

public:
    Utility();
};

namespace procon {
    template<typename X,typename Y>
    X floor(X var,Y power)
    {
        return std::floor(var * std::pow(10,power)) / std::pow(10,power);
    }

    template<typename X,typename Y>
    X round(X var,Y power)
    {
        return std::round(var * std::pow(10,power)) / std::pow(10,power);
    }

    template<typename X,typename Y>
    bool nearlyEqual(X var1,X var2,Y allowance)
    {

        return std::abs(var1 - var2) < allowance ? true : false;
    }
}

#endif // UTILITY_H