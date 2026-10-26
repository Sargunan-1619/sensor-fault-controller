/**
 * @file math.h
 * @brief Freestanding math declarations mapping IEEE-754 checks to Clang builtins for WebAssembly.
 */
#ifndef WASM_MATH_H
#define WASM_MATH_H

#define isnan(x) __builtin_isnan(x)
#define isinf(x) __builtin_isinf(x)

#endif /* WASM_MATH_H */
