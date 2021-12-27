#pragma once
#ifndef Strutils_hpp
#define Strutils_hpp

#include <algorithm>
#include <cstring>
#include <limits>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

class Strutils {
  public:
    static inline void to_upper(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) -> unsigned char {
                           return static_cast<unsigned char>(std::toupper(c));
                       });
    }

    static inline void to_lower(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) -> unsigned char {
                           return static_cast<unsigned char>(std::tolower(c));
                       });
    }

    static inline void replace_chr(std::string &str, char chin, char chout) {
        for (auto &ch : str) {
            if (ch == chin) {
                ch = chout;
            }
        }
    }

    static inline constexpr auto constexpr_strlen(const char *str) -> size_t {
        return std::string_view(str).size();
    }

    static inline constexpr auto constexpr_strncpy(char *dest, const char *src,
                                                   const size_t n) -> char * {
        for (size_t i = 0; i < n; i++) {
            dest[i] = src[i];
        }
        dest[n] = 0;
        return dest;
    }

    static inline constexpr auto constexpr_strncat(char *dest,
                                                   const size_t start,
                                                   const char *src,
                                                   const size_t n) -> char * {
        for (size_t i = start; i < n; i++) {
            dest[i] = src[i];
        }
        dest[n] = 0;
        return dest;
    }

    static inline auto getCNPJNumbers(const std::string &cnpj) -> std::string {
        return std::regex_replace(cnpj, std::regex("[^0-9]*"),
                                  std::string("$1"));
    }

    static auto explode(const std::string &s, char delim)
        -> std::vector<std::string> {
        std::vector<std::string> result;
        std::istringstream iss(s);

        for (std::string token; std::getline(iss, token, delim);) {
            result.push_back(std::move(token));
        }

        return result;
    }

    static auto url_decode(const std::string &str) -> std::string {
        std::string result;
        result.reserve(str.size());

        std::unordered_map<char, int> arrmap = {
            std::pair<char, int>('0', 0),   std::pair<char, int>('1', 1),
            std::pair<char, int>('2', 2),   std::pair<char, int>('3', 3),
            std::pair<char, int>('4', 4),   std::pair<char, int>('5', 5),
            std::pair<char, int>('6', 6),   std::pair<char, int>('7', 7),
            std::pair<char, int>('8', 8),   std::pair<char, int>('9', 9),
            std::pair<char, int>('A', 0xA), std::pair<char, int>('B', 0xB),
            std::pair<char, int>('C', 0xC), std::pair<char, int>('D', 0xD),
            std::pair<char, int>('E', 0xE), std::pair<char, int>('F', 0xF),
        };

        int chrsize = 0;
        int tempchchr = 0;
        bool inchr = false;

        for (auto &c : str) {
            if (chrsize == 2) {
                result.append(1u, static_cast<char>(tempchchr));
                inchr = false;
                chrsize = 0;
                tempchchr = 0;
            }

            if (c == '%') {
                inchr = true;
                chrsize = 0;
                tempchchr = 0;
                continue;
            }

            if (inchr) {
                if (chrsize == 0) {
                    tempchchr = 0;
                    tempchchr |= arrmap[c] << 4;
                    ++chrsize;
                } else {
                    tempchchr |= arrmap[c];
                    ++chrsize;
                }
            } else {
                result.append(1u, c);
            }
        }

        if (chrsize == 2) {
            result.append(1u, static_cast<char>(tempchchr));
        }

        return result;
    }
};

#endif
