#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_
#pragma once

#include <string>

namespace string_utils {

// Converts between wide and UTF-8 representations of a string. On error, the
// result is system-dependent.
std::string SysWideToUTF8(const std::wstring& wide);
std::wstring SysUTF8ToWide(const std::string& utf8);

// Converts between 8-bit and wide strings, using the given code page. The
// code page identifier is one accepted by the Windows function
// MultiByteToWideChar().
std::wstring SysMultiByteToWide(const std::string& mb, unsigned int code_page);
std::string SysWideToMultiByte(const std::wstring& wide,
                               unsigned int code_page);

}  // namespace string_utils

#endif  // STRING_UTILS_H_