// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

// XXX __cpp_constexpr and __cplusplus checks are not reliable here, so just check compiler version

#if defined(__clang__)

#if __clang_major__ >= 18
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 constexpr
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 constexpr
#define constexpr_l constexpr
#elif __clang_major__ >= 10
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 const
#define constexpr_l constexpr
#elif __clang_major__ >= 7
#define constexpr_f constexpr
#define constexpr_f1 inline
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l constexpr
#else
#define constexpr_f inline
#define constexpr_f1 inline
#define constexpr_f2 inline
#define constexpr_v const
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l 
#endif

#elif defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)

#if __GNUC__ >= 13
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 constexpr
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 constexpr
#define constexpr_l constexpr
#elif __GNUC__ >= 10
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 const
#define constexpr_l constexpr
#elif __GNUC__ >= 7
#define constexpr_f constexpr
#define constexpr_f1 inline
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l constexpr
#else
#define constexpr_f inline
#define constexpr_f1 inline
#define constexpr_f2 inline
#define constexpr_v const
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l 
#endif

// just guessing
#elif __cplusplus >= 202302L
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 constexpr
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 constexpr
#define constexpr_l constexpr
#elif __cplusplus >= 202002L
#define constexpr_f constexpr
#define constexpr_f1 constexpr
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 constexpr
#define constexpr_v2 const
#define constexpr_l constexpr
#elif __cplusplus >= 201703L
#define constexpr_f constexpr
#define constexpr_f inline
#define constexpr_f2 inline
#define constexpr_v constexpr
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l constexpr
#else
#define constexpr_f inline
#define constexpr_f1 inline
#define constexpr_f2 inline
#define constexpr_v const
#define constexpr_v1 const
#define constexpr_v2 const
#define constexpr_l 
#endif
