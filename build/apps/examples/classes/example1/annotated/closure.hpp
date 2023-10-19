#ifndef __CLOSURE__
#define __CLOSURE__

#include <cstdio>

namespace cle
{

    template <size_t len>
    struct fixed_string
    {
        template <size_t N>
        constexpr fixed_string(const char (&arr)[N]) {}
    };

    template <size_t N>
    fixed_string(const char (&arr)[N]) -> fixed_string<N + 1>;

    template <typename T, fixed_string f>
    using annotate = T;

}

#endif /* __CLOSURE__ */