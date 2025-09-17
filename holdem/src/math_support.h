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
#include <cmath>
#include <limits>

struct ValueAndSlope {
  float64 v;
  float64 d_v;

  // Marked `…_unsafe` because you should be guarding against `pow(0.0, 0.0)` before you call this
  static constexpr ValueAndSlope exponentiate_unsafe(const ValueAndSlope &a, const ValueAndSlope &b) {
    // y = std::pow(base, exponent)
    float64 exponentiation_power_v = std::pow(a.v, b.v);
    // log(y) = exponent * log(base)
    // d log(y) = d exponent * log(base) + exponent * d log(base)
    // dy / y = d exponent * log(base) + exponent * (d base) / base
    // dy = y * (d exponent * log(base) + exponent * (d base) / base)
    float64 exponentiation_power_d_v = exponentiation_power_v * (b.d_v * std::log(a.v)  +  b.v * a.d_v / a.v);
    return ValueAndSlope{
      exponentiation_power_v, exponentiation_power_d_v
    };
  }

  static constexpr ValueAndSlope multiply2(const ValueAndSlope &a, const ValueAndSlope &b) {
    float64 multiplication_product_v = a.v * b.v;
    float64 multiplication_product_d_v = a.d_v * b.v + b.d_v * a.v;
    return ValueAndSlope{
      multiplication_product_v, multiplication_product_d_v
    };
  }

  static constexpr ValueAndSlope multiply3(const ValueAndSlope &a, const ValueAndSlope &b, const ValueAndSlope &c) {
    // multiplicand, multiplier, product
    float64 multiplication_product_v = a.v * b.v * c.v;
    // y = a * b * c
    // log(y) = log(a) + log(b) + log(c)
    // dy / y =  da / a + db / b + dc / c
    // dy = y * (da / a + db / b + dc / c)
    float64 multiplication_product_d_v = multiplication_product_v * ( a.d_v / a.v + b.d_v / b.v + c.d_v / c.v );
    return ValueAndSlope{
      multiplication_product_v, multiplication_product_d_v
    };
  }

  static constexpr ValueAndSlope sum3(const ValueAndSlope &a, const ValueAndSlope &b, const ValueAndSlope &c) {
    float64 addition_sum_v = a.v + b.v + c.v;
    float64 addition_sum_d_v = a.d_v + b.d_v + c.d_v;
    return ValueAndSlope{
      addition_sum_v, addition_sum_d_v
    };
  }

  static ValueAndSlope constexpr lesserOfTwo(const ValueAndSlope &a, const ValueAndSlope &b) {
    if (a.v < b.v) {
      return a;
    } else if (b.v < a.v) {
      return b;
    } else {
        if (a.d_v < b.d_v) {
          return a;
        } else {
          return b;
        }
    }
  }
  static ValueAndSlope constexpr greaterOfTwo(const ValueAndSlope &a, const ValueAndSlope &b) {
    if (a.v < b.v) {
      return b;
    } else if (b.v < a.v) {
      return a;
    } else {
        if (a.d_v < b.d_v) {
          return b;
        } else {
          return a;
        }
    }
  }
};

static_assert(std::numeric_limits<float64>::has_signaling_NaN, "We use signaling_NaN everywhere. Sorry!");

#endif
