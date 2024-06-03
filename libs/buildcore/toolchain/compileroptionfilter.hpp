#ifndef COMPILEROPTIONFILTER_HPP
#define COMPILEROPTIONFILTER_HPP

#include <tuple>

class CompilerInfoFamilyGcc;
class CompilerInfoFamilySdcc;
class CompilerInfoFamilyMsvc;

namespace CompilerOptionFilter {
    template <typename... Functors>
    struct And : private std::tuple<Functors...> {
        using std::tuple<Functors...>::tuple;

        template <typename T>
        bool operator()(const T &compilerInfo) const {
            return std::apply([&compilerInfo](const Functors... functors) {
                return (functors(compilerInfo) && ...);
            }, this->tuple);
        }
    };

    template <typename... Functors>
    struct Or : private std::tuple<Functors...> {
        using std::tuple<Functors...>::tuple;

        template <typename T>
        bool operator()(const T &compilerInfo) const {
            return std::apply([&compilerInfo](const Functors... functors) {
                return (functors(compilerInfo) || ...);
            }, this->tuple);
        }
    };

    template <typename Functor>
    struct Not : private Functor {
        using Functor::Functor;

        template <typename T>
        bool operator()(const T &compilerInfo) const {
            return !Functor::operator()(compilerInfo);
        }
    };

    struct Falsy {
        template <typename T>
        bool operator()(const T &) const { return false; }
    };

    template <int Major, int Minor = 0, int Patch = 0>
    struct GccAtLeast : Falsy {
        bool operator()(const CompilerInfoFamilyGcc &) const {
            return true;
        }
    };

    template <int Major, int Minor = 0, int Patch = 0>
    struct GccLessThan : Falsy {
        bool operator()(const CompilerInfoFamilyGcc &) const {
            return true;
        }
    };

    struct Clang : Falsy {
        bool operator()(const CompilerInfoFamilyGcc &) const {
            return true;
        }

        bool operator()(const CompilerInfoFamilyMsvc &) const {
            return true;
        }
    };

    template <int Major, int Minor = 0, int Patch = 0>
    struct ClangAtLeast : Falsy {
        bool operator()(const CompilerInfoFamilyGcc &) const {
            return true;
        }

        bool operator()(const CompilerInfoFamilyMsvc &) const {
            return true;
        }
    };

    template <int Major, int Minor = 0, int Patch = 0>
    struct ClangLessThan : Falsy {
        bool operator()(const CompilerInfoFamilyGcc &) const {
            return true;
        }

        bool operator()(const CompilerInfoFamilyMsvc &) const {
            return true;
        }
    };

    template <int Major, int Minor = 0>
    struct MsvcAtLeast : Falsy {
        bool operator()(const CompilerInfoFamilyMsvc &) const {
            return true;
        }
    };

    template <int Major, int Minor = 0>
    struct MsvcLessThan : Falsy {
        bool operator()(const CompilerInfoFamilyMsvc &) const {
            return true;
        }
    };
}

#endif // COMPILEROPTIONFILTER_HPP
