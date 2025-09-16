//
//  math_support.h
//  holdem
//
//  Created by Joseph Huang on 11/27/2013.
//  Copyright (c) 2013 Joseph Huang. All rights reserved.
//

#ifndef holdem_math_support_h
#define holdem_math_support_h

#include "portability.h"
#include <math.h>

#ifndef log1p
inline constexpr float64 log1p(float64 x)
{
    return log(1+x);
}
#endif // log1p


inline constexpr bool is_nan(float64 x)
{
    return (!(x == x));
}

#include <limits>
//#ifndef nan
//inline float64 nan()
//{
//    return std::numeric_limits<float64>::signaling_NaN();
//}
//#endif // nan

static_assert(std::numeric_limits<double>::has_signaling_NaN, "We use signaling_NaN everywhere. Sorry!");

#endif
