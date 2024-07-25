// SPDX-License-Identifier: LGPL-2.0-or-later
// Copyright (C) 2024 Matti Tiainen <mvtiaine@cc.hut.fi>

#pragma once

// XXX __cpp_constexpr and __cplusplus checks are not reliable here, so just check compiler version

#if defined(__clang__)

#if __clang_major__ >= 18
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 constexpr
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 constexpr
#define _CONSTEXPR_L constexpr
#elif __clang_major__ >= 10
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#elif __clang_major__ >= 7
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#else
#define _CONSTEXPR_F inline
#define _CONSTEXPR_F1 inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V const
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L 
#endif

#elif defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)

#if __GNUC__ >= 13
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 constexpr
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 constexpr
#define _CONSTEXPR_L constexpr
#elif __GNUC__ >= 10
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#elif __GNUC__ >= 7
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#else
#define _CONSTEXPR_F inline
#define _CONSTEXPR_F1 inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V const
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L 
#endif

// just guessing
#elif __cplusplus >= 202302L
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 constexpr
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 constexpr
#define _CONSTEXPR_L constexpr
#elif __cplusplus >= 202002L
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F1 constexpr
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 constexpr
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#elif __cplusplus >= 201703L
#define _CONSTEXPR_F constexpr
#define _CONSTEXPR_F inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V constexpr
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L constexpr
#else
#define _CONSTEXPR_F inline
#define _CONSTEXPR_F1 inline
#define _CONSTEXPR_F2 inline
#define _CONSTEXPR_V const
#define _CONSTEXPR_V1 const
#define _CONSTEXPR_V2 const
#define _CONSTEXPR_L 
#endif
