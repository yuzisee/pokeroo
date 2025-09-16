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

struct ValueAndSlope {
  float64 v;
  float64 d_v;

  static const ValueAndSlope constexpr lesserOfTwo(const ValueAndSlope &a, const ValueAndSlope &b) {
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
  static const ValueAndSlope constexpr greaterOfTwo(const ValueAndSlope &a, const ValueAndSlope &b) {
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

static_assert(std::numeric_limits<double>::has_signaling_NaN, "We use signaling_NaN everywhere. Sorry!");

#endif
