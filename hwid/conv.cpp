// --- conv.cpp

#include <string>
#include <cstring>
#include <locale>
#include <codecvt>

namespace conv {

    namespace multibyte {

        std::string convert(const std::wstring& s)
        {
            return convert(s.c_str());
        }
        std::wstring convert(const std::string& s)
        {
            return convert(s.c_str());
        }

        std::string convert(const wchar_t* s)
        {
            const size_t len = wcslen(s);
            std::string result(len, '\0');

#ifdef _MSC_VER
            size_t count = 0;
            const auto size_ = result.size();
            wcstombs_s(&count, &result[0], size_, s, size_);
#else
            std::mbstate_t state = std::mbstate_t();
            std::wcsrtombs(&result[0], &s, result.size(), &state);
#endif

            return result;
        }

        std::wstring convert(const char* s)
        {
            std::mbstate_t state = std::mbstate_t();

            size_t len;
            wchar_t wc[4] = {};
            std::wstring result;
            const char* end = s + std::strlen(s);
            while ((len = std::mbrtowc(wc, s, end - s, &state)) > 0)
            {
                result += wc; s += len;
                for (auto& ch : wc)  ch = L'\0';
            }
            return result;
        }

    }//namespace multibyte

    namespace utf8 {

        std::string  convert(const std::wstring& s)
        {
            typedef std::codecvt_utf8<wchar_t>
                convert_typeX;
            std::wstring_convert<convert_typeX, wchar_t>
                converterX;
            return converterX.to_bytes(s);
        }
        std::wstring convert(const std::string& s)
        {
            typedef std::codecvt_utf8<wchar_t>
                convert_typeX;
            std::wstring_convert<convert_typeX, wchar_t>
                converterX;
            return converterX.from_bytes(s);
        }

    }//namespace utf8 

    namespace stdlocal {

        std::wstring convert(const char* first, const size_t len, const std::locale& loc)
        {
            if (len == 0)
                return std::wstring();
            const std::ctype<wchar_t>& facet =
                std::use_facet< std::ctype<wchar_t> >(loc);
            const char* last = first + len;
            std::wstring result(len, L'\0');
            facet.widen(first, last, &result[0]);
            return result;
        }

        std::string convert(
            const wchar_t* first,
            const size_t len,
            const std::locale& loc,
            const char default_char
        )
        {
            if (len == 0)
                return std::string();
            const std::ctype<wchar_t>& facet =
                std::use_facet<std::ctype<wchar_t> >(loc);
            const wchar_t* last = first + len;
            std::string result(len, default_char);
            facet.narrow(first, last, default_char, &result[0]);
            return result;
        }

        std::string  convert(const wchar_t* s, const std::locale& loc, const char default_char)
        {
            return convert(s, std::wcslen(s), loc, default_char);
        }
        std::string  convert(const std::wstring& s, const std::locale& loc, const char default_char)
        {
            return convert(s.c_str(), s.length(), loc, default_char);
        }
        std::wstring convert(const char* s, const std::locale& loc)
        {
            return convert(s, std::strlen(s), loc);
        }
        std::wstring convert(const std::string& s, const std::locale& loc)
        {
            return convert(s.c_str(), s.length(), loc);
        }

    }//namespace stdlocal
}//namespace conv


