/*
 * Copyright (c) 2015 - 2023, Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOCKDENSELAYER_HPP_INCLUDE
#define MOCKDENSELAYER_HPP_INCLUDE

#include "gmock/gmock.h"
#include "DenseLayer.hpp"
#include "TensorOneD.hpp"

namespace geopm {

class MockDenseLayer : public DenseLayer {
 public:
  MOCK_METHOD(TensorOneD, forward, (const TensorOneD &input), (const override));
  MOCK_METHOD(size_t, get_input_dim, (), (const override));
  MOCK_METHOD(size_t, get_output_dim, (), (const override));
};

}  // namespace geopm

#endif //MOCKDenseLayer_HPP_INCLUDE