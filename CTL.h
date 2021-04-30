#pragma once

#include "logic.h"

template <typename F> struct AX : detail::UnaryOp<F> {};
template <typename F> struct EX : detail::UnaryOp<F> {};
template <typename F> struct AF : detail::UnaryOp<F> {};
template <typename F> struct EF : detail::UnaryOp<F> {};
template <typename F> struct AG : detail::UnaryOp<F> {};
template <typename F> struct EG : detail::UnaryOp<F> {};

template <typename L, typename R> struct AU : detail::BinaryOp<L, R> {};
template <typename L, typename R> struct EU : detail::BinaryOp<L, R> {};
