// Copyright 2022 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "yacl/crypto/primitives/ot/ferret_ote.h"
#include "yacl/crypto/primitives/ot/gywz_ote.h"

namespace yacl::crypto {

uint64_t MpCotRNHelper(uint64_t idx_num, uint64_t idx_range) {
  const auto batch_size = (idx_range + idx_num - 1) / idx_num;
  const auto last_size = idx_range - batch_size * (idx_num - 1);
  return Log2Ceil(batch_size) * (idx_num - 1) + Log2Ceil(last_size);
}

void MpCotRNSend(const std::shared_ptr<link::Context>& ctx,
                 const OtSendStore& cot, uint64_t idx_range, uint64_t idx_num,
                 absl::Span<uint128_t> out) {
  const auto full_size = idx_range;
  const auto batch_num = idx_num;
  const auto batch_size = full_size / batch_num;

  // for each bin, call single-point cot
  for (uint64_t i = 0; i < batch_num; ++i) {
    const uint64_t this_size =
        (i == batch_size - 1) ? full_size - i * batch_size : batch_size;
    const auto& cot_slice =
        cot.Slice(i * Log2Ceil(this_size), (i + 1) * Log2Ceil(this_size));

    FerretGywzOtExtSend(ctx, cot_slice, this_size,
                        out.subspan(i * batch_size, this_size));
  }
}

void MpCotRNRecv(const std::shared_ptr<link::Context>& ctx,
                 const OtRecvStore& cot, uint64_t idx_range, uint64_t idx_num,
                 absl::Span<uint128_t> out) {
  const auto full_size = idx_range;
  const auto batch_num = idx_num;
  const auto batch_size = full_size / batch_num;

  // for each bin, call single-point cot
  for (uint64_t i = 0; i < batch_num; ++i) {
    const uint64_t this_size =
        (i == batch_size - 1) ? full_size - i * batch_size : batch_size;
    const auto cot_slice =
        cot.Slice(i * Log2Ceil(this_size), (i + 1) * Log2Ceil(this_size));
    FerretGywzOtExtRecv(ctx, cot_slice, this_size,
                        out.subspan(i * batch_size, this_size));
  }
}

}  // namespace yacl::crypto
