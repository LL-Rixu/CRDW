#pragma once

#include <string>

template<size_t N>
struct FixedString
{
    char data[N] = {};
    consteval FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }

    constexpr operator char*() const { return data; }
    constexpr operator const char*() const { return data; }
    constexpr operator std::string_view() const { return {data, N - 1}; }

    constexpr const char* c_str() const { return data; }
};