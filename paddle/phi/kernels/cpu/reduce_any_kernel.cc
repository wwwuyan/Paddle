// Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "paddle/phi/kernels/reduce_any_kernel.h"

#include <type_traits>

#include "paddle/phi/backends/cpu/cpu_context.h"
#include "paddle/phi/common/complex.h"
#include "paddle/phi/core/kernel_registry.h"
#include "paddle/phi/kernels/cpu/reduce.h"
#include "paddle/phi/kernels/funcs/reduce_functor.h"

using complex64 = ::phi::dtype::complex<float>;
using complex128 = ::phi::dtype::complex<double>;

namespace phi {

template <typename T, typename Context>
void AnyRawKernel(const Context& dev_ctx,
                  const DenseTensor& x,
                  const std::vector<int64_t>& dims,
                  bool keep_dim,
                  bool reduce_all,
                  DenseTensor* out) {
  reduce_all = recompute_reduce_all(x, dims, reduce_all);
  if (std::is_same<T, complex64>::value) {
    DenseTensor bool_tensor;
    bool_tensor.Resize(x.dims());

    bool* bool_data = dev_ctx.template Alloc<bool>(&bool_tensor);
    const complex64* data = x.data<complex64>();

    int64_t numel = x.numel();
    for (int64_t i = 0; i < numel; ++i) {
      bool_data[i] = (data[i].real != 0 || data[i].imag != 0);
    }
    phi::BoolReduceKernel<CPUContext, T, phi::funcs::AnyFunctor>(
        dev_ctx, bool_tensor, dims, keep_dim, reduce_all, out);
  } else if (std::is_same<T, complex128>::value) {
    DenseTensor bool_tensor;
    bool_tensor.Resize(x.dims());

    bool* bool_data = dev_ctx.template Alloc<bool>(&bool_tensor);
    const complex128* data = x.data<complex128>();

    int64_t numel = x.numel();
    for (int64_t i = 0; i < numel; ++i) {
      bool_data[i] = (data[i].real != 0 || data[i].imag != 0);
    }
    phi::BoolReduceKernel<CPUContext, T, phi::funcs::AnyFunctor>(
        dev_ctx, bool_tensor, dims, keep_dim, reduce_all, out);
  } else {
    phi::BoolReduceKernel<CPUContext, T, phi::funcs::AnyFunctor>(
        dev_ctx, x, dims, keep_dim, reduce_all, out);
  }
}

}  // namespace phi

PD_REGISTER_KERNEL(any_raw,
                   CPU,
                   ALL_LAYOUT,
                   phi::AnyRawKernel,
                   float,
                   double,
                   int,
                   int64_t,
                   bool,
                   complex64,
                   complex128) {
  kernel->OutputAt(0).SetDataType(phi::DataType::BOOL);
}
