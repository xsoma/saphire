

#include <string>
#include <locale>

namespace conv {

    namespace multibyte {
        std::string  convert(const wchar_t* s);
        std::wstring convert(const char* s);
        std::string  convert(const std::wstring& s);
        std::wstring convert(const std::string& s);
    }// namespace multibyte 

    namespace utf8 {
        std::string  convert(const std::wstring& s);
        std::wstring convert(const std::string& s);
    }// namespace utf8 

    namespace stdlocal {

        std::string convert(
            const wchar_t* s,
            const size_t len,
            const std::locale& loc = std::locale(),
            const char default_char = '?'
        );

        std::string convert(
            const wchar_t* s,
            const std::locale& loc = std::locale(""),
            const char default_char = '?'
        );
        std::string convert(
            const std::wstring& s,
            const std::locale& loc = std::locale(""),
            const char default_char = '?'
        );

        std::wstring convert(
            const char* s,
            const std::locale& loc = std::locale("")
        );
        std::wstring convert(
            const std::string& s,
            const std::locale& loc = std::locale("")
        );

        std::wstring convert(
            const char* s,
            const size_t len,
            const std::locale& loc = std::locale()
        );

    } //namespace stdlocal 

}//namespace conv

#pragma once
