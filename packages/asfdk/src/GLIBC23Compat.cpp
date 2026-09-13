#include <cstdlib>
#include <cerrno>

extern "C" {
    unsigned long long __isoc23_strtoull(const char* nptr, char** endptr, int base) {
        return std::strtoull(nptr, endptr, base);
    }
    long long __isoc23_strtoll(const char* nptr, char** endptr, int base) {
        return std::strtoll(nptr, endptr, base);
    }
    unsigned long long __isoc23_strtoull_l(const char* nptr, char** endptr, int base, locale_t loc) {
        return std::strtoull(nptr, endptr, base);
    }
    long long __isoc23_strtoll_l(const char* nptr, char** endptr, int base, locale_t loc) {
        return std::strtoll(nptr, endptr, base);
    }
}
