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

// @return ln(a/b) = ln(a) - ln(b)
constexpr float64 stable_ln_div(float64 a, float64 b) {
  const float64 delta = (a - b) / b;                     // = r−1
  if (std::fabs(delta) <= std::sqrt(std::numeric_limits<float64>::epsilon())) {
    // at roughly √ε at least half the mantissa is retained after subtraction, so the log1p approach is 5~6 decimal digits more accurate
    return std::log1p(delta);                   // best accuracy
  } else {
    return std::log(a) - std::log(b);
  }
}

struct ValueAndSlope {
  float64 v;
  float64 D_v;

  void clearToZero() {
    this->v = 0.0;
    this->D_v = 0.0;
  }

  void set_value_and_slope(float64 new_v, float64 new_dv) {
    this->v = new_v;
    this->D_v = new_dv;
  }

  void rescale(float64 mag) {
    this->v *= mag;
    this->D_v *= mag;
  }

  constexpr bool any_nan() const {
    return (std::isnan(this->v) || std::isnan(this->D_v));
  }

  // Marked `…_unsafe` because you should be guarding against `pow(0.0, 0.0)` before you call this
  static constexpr ValueAndSlope exponentiate_unsafe(const ValueAndSlope &a, const ValueAndSlope &b) {
    // y = std::pow(base, exponent)
    const float64 exponentiation_power_v = std::pow(a.v, b.v);
    // log(y) = exponent * log(base)
    // d log(y) = d exponent * log(base) + exponent * d log(base)
    // dy / y = d exponent * log(base) + exponent * (d base) / base
    // dy = y * (d exponent * log(base) + exponent * (d base) / base)
    const float64 exponentiation_power_d_v = exponentiation_power_v * (b.D_v * std::log(a.v)  +  b.v * a.D_v / a.v);
    return ValueAndSlope{
      exponentiation_power_v, exponentiation_power_d_v
    };
  }

  static constexpr ValueAndSlope multiply2(const ValueAndSlope &a, const ValueAndSlope &b) {
    const float64 multiplication_product_v = a.v * b.v;
    const float64 multiplication_product_d_v = a.D_v * b.v + b.D_v * a.v;
    return ValueAndSlope{
      multiplication_product_v, multiplication_product_d_v
    };
  }

  static constexpr ValueAndSlope multiply3(const ValueAndSlope &a, const ValueAndSlope &b, const ValueAndSlope &c) {
    // multiplicand, multiplier, product
    const float64 multiplication_product_v = a.v * b.v * c.v;
    // y = a * b * c
    // log(y) = log(a) + log(b) + log(c)
    // dy / y =  da / a + db / b + dc / c
    // dy = y * (da / a + db / b + dc / c)
    const float64 multiplication_product_d_v = a.D_v * b.v * c.v + a.v * b.D_v * c.v + a.v * b.v * c.D_v;
    // ^^^ More stable in case any of (a.v, b.v, c.v) are close to 0.0
    // const float64 multiplication_product_d_v = multiplication_product_v * ( a.D_v / a.v + b.D_v / b.v + c.D_v / c.v );

    return ValueAndSlope{
      multiplication_product_v, multiplication_product_d_v
    };
  }

  constexpr ValueAndSlope operator+(const ValueAndSlope &a) const {
    float64 addition_sum_v = this->v + a.v;
    float64 addition_sum_d_v = this->D_v + a.D_v;
    return ValueAndSlope{
      addition_sum_v, addition_sum_d_v
    };
  }

  static constexpr ValueAndSlope sum3(const ValueAndSlope &a, const ValueAndSlope &b, const ValueAndSlope &c) {
    float64 addition_sum_v = a.v + b.v + c.v;
    float64 addition_sum_d_v = a.D_v + b.D_v + c.D_v;
    return ValueAndSlope{
      addition_sum_v, addition_sum_d_v
    };
  }

  constexpr ValueAndSlope operator-(const ValueAndSlope &a) const {
    float64 subtraction_diff_v = this->v - a.v;
    float64 subtraction_diff_d_v = this->D_v - a.D_v;
    return ValueAndSlope{
      subtraction_diff_v, subtraction_diff_d_v
    };
  }

  static ValueAndSlope constexpr lesserOfTwo(const ValueAndSlope &a, const ValueAndSlope &b) {
    if (a.v < b.v) {
      return a;
    } else if (b.v < a.v) {
      return b;
    } else {
        if (a.D_v < b.D_v) {
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
        if (a.D_v < b.D_v) {
          return b;
        } else {
          return a;
        }
    }
  }
};

static_assert(std::numeric_limits<float64>::has_signaling_NaN, "We use signaling_NaN everywhere. Sorry!");

#endif
