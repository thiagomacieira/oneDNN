/*******************************************************************************
* Copyright 2020-2022 Intel Corporation
* Copyright 2020 Codeplay Software Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef GPU_NVIDIA_CUDNN_SOFTMAX_HPP
#define GPU_NVIDIA_CUDNN_SOFTMAX_HPP

#include "cudnn.h"

#include <CL/sycl.hpp>

#include "common/primitive.hpp"
#include "common/softmax_pd.hpp"
#include "gpu/nvidia/cudnn_softmax_impl.hpp"
#include "gpu/nvidia/sycl_cuda_engine.hpp"
#include "gpu/nvidia/sycl_cuda_utils.hpp"

namespace dnnl {
namespace impl {
namespace gpu {
namespace nvidia {

struct cudnn_softmax_fwd_t : public primitive_t {
    using primitive_t::primitive_t;

    struct pd_t : public softmax_fwd_pd_t {
        using softmax_fwd_pd_t::softmax_fwd_pd_t;

        DECLARE_COMMON_PD_T("cuda:cudnn:any", cudnn_softmax_fwd_t);

        status_t init(engine_t *) {
            const memory_desc_wrapper src_d(src_md());
            const memory_desc_wrapper dst_d(dst_md());

            bool ok = is_fwd()
                    && utils::one_of(
                            src_d.data_type(), data_type::f32, data_type::f16)
                    && src_d.is_plain() && dst_d.is_plain()
                    && attr()->has_default_values() && dst_d == src_d;
            if (!ok) return status::unimplemented;

            softmax_impl_.reset(new cudnn_softmax_fwd_impl_t());

            return softmax_impl_->init(this);
        }

        std::shared_ptr<cudnn_softmax_impl_base_t> softmax_impl_;
    };

    status_t execute(const exec_ctx_t &ctx) const override;

private:
    const pd_t *pd() const { return (const pd_t *)primitive_t::pd().get(); }
};

struct cudnn_softmax_bwd_t : public primitive_t {
    using primitive_t::primitive_t;

    struct pd_t : public softmax_bwd_pd_t {
        using softmax_bwd_pd_t::softmax_bwd_pd_t;

        DECLARE_COMMON_PD_T("cuda:cudnn:any", cudnn_softmax_bwd_t);

        status_t init(engine_t *) {
            const memory_desc_wrapper diff_src_d(diff_src_md());
            const memory_desc_wrapper diff_dst_d(diff_dst_md());
            const memory_desc_wrapper dst_d(dst_md());

            bool ok = !is_fwd()
                    && utils::one_of(
                            dst_d.data_type(), data_type::f32, data_type::f16)
                    && attr()->has_default_values()
                    && set_default_formats_common() && dst_d.is_plain()
                    && diff_dst_d.is_plain() && diff_src_d.is_plain()
                    && diff_src_d == diff_dst_d && diff_src_d == dst_d;
            if (!ok) return status::unimplemented;

            softmax_impl_.reset(new cudnn_softmax_bwd_impl_t());

            return softmax_impl_->init(this);
        }

        std::shared_ptr<cudnn_softmax_impl_base_t> softmax_impl_;
    };

    status_t execute(const exec_ctx_t &ctx) const override;

private:
    const pd_t *pd() const { return (const pd_t *)primitive_t::pd().get(); }
};

} // namespace nvidia
} // namespace gpu
} // namespace impl
} // namespace dnnl

#endif
