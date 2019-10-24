// Copyright © 2019 Aleksey Nikolaev
// License: http://opensource.org/licenses/MIT

#ifndef __ENDIANNESS__
#define __ENDIANNESS__

#include <stdint.h>
#include <type_traits>
#include <utility>
//#include <bit> //stl c++20

namespace endianness {
namespace stl20 {
#ifdef _MSC_VER
constexpr int __LITTLE_ENDIAN = 1234;
constexpr int __BIG_ENDIAN = 4321;
constexpr int __PDP_ENDIAN = 3412;
constexpr int __BYTE_ORDER = __LITTLE_ENDIAN;
#else
#    include "endian.h"
#endif
#if __BYTE_ORDER != __LITTLE_ENDIAN && __BYTE_ORDER != __BIG_ENDIAN
#    error Only little/big endian are implemented. __PDP_ENDIAN and other are not implemented
#endif //__BYTE_ORDER
enum class endian
{
    little = __LITTLE_ENDIAN,
    big = __BIG_ENDIAN,
    native = __BYTE_ORDER
};
} // namespace stl20

#pragma pack(push, 1)
template <typename T, stl20::endian order = stl20::endian::native>
struct Endian
{
    static_assert(order == stl20::endian::little || order == stl20::endian::big,
                  "Only little/big endian are implemented. __PDP_ENDIAN and other are not implemented");
    union
    {
        T value;
        unsigned char bytes[sizeof(T)];
    };

    Endian() : value(T()) {}
    ~Endian() = default;
    Endian(T t)
    {
        if constexpr (order == stl20::endian::native || sizeof(T) == 1) {
            value = t;
        } else {
            *this = Endian<T, stl20::endian::native>(t);
        }
    }
    //copy
    Endian(const Endian &other) : value(other.value) {}

    template <typename U, stl20::endian orderU>
    Endian(const Endian<U, orderU> &other)
    {
        *this = other;
    }

    Endian &operator=(const Endian &other)
    {
        if (this != &other)
            value = other.value;
        return *this;
    }

    template <typename U, stl20::endian orderU>
    Endian &operator=(const Endian<U, orderU> &other)
    {
        if constexpr (order == orderU || sizeof(T) == 1) {
            value = other.value;
        } else if constexpr (!std::is_same_v<T, U>) {
            *this = Endian<T, orderU>(other);
        } else {
            value = other.value;
            for (auto i = sizeof(T) / 2; i-- > 0;)
                std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
        }
        return *this;
    }
    //move
    Endian(Endian &&other) : value(other.value) {}
    Endian &operator=(Endian &&other)
    {
        std::swap(value, other.value);
        return *this;
    }
    template <typename U, stl20::endian orderU>
    Endian &operator=(Endian<U, orderU> &&other)
    {
        *this = other;
        return *this;
    }
    template <typename U, stl20::endian orderU>
    Endian(Endian<U, orderU> &&other)
    {
        *this = other;
    }

    //type convertion
    operator T() const
    {
        if constexpr (order == stl20::endian::native || sizeof(T) == 1) {
            return value;
        } else {
            Endian<T, stl20::endian::native> t(*this);
            return t.value;
        }
    }

    // operators
    Endian operator|=(const Endian &other)
    {
        value |= other.value;
        return *this;
    }
    Endian operator&=(const Endian &other)
    {
        value &= other.value;
        return *this;
    }
    Endian operator^=(const Endian &other)
    {
        value ^= other.value;
        return *this;
    }
    Endian operator~() const
    {
        Endian tmp;
        tmp.value = ~value;
        return tmp;
    }
    Endian operator+=(const Endian &other) { return (*this = *this + other); }
    Endian operator-=(const Endian &other) { return (*this = *this - other); }
    Endian operator*=(const Endian &other) { return (*this = *this * other); }
    Endian operator/=(const Endian &other) { return (*this = *this / other); }
    Endian operator%=(const Endian &other) { return (*this = *this % other); }
    // postfix
    Endian operator++(int)
    {
        Endian tmp(*this);
        operator++();
        return tmp;
    }
    // prefix
    Endian &operator++()
    {
        if constexpr (order == stl20::endian::native || sizeof(T) == 1) {
            ++value;
        } else if constexpr (order == stl20::endian::big) {
            for (auto i = sizeof(T); i-- > 0;) {
                ++bytes[i];
                if (bytes[i] != 0)
                    break;
            }
        } else if constexpr (order == stl20::endian::little) {
            for (unsigned i = 0; i < sizeof(T); i++) {
                ++bytes[i];
                if (bytes[i] != 0)
                    break;
            }
        }
        return *this;
    }

    Endian operator--(int)
    {
        Endian tmp(*this);
        operator--();
        return tmp;
    }

    Endian &operator--()
    {
        if constexpr (order == stl20::endian::native || sizeof(T) == 1) {
            --value;
        } else if constexpr (order == stl20::endian::big) {
            for (auto i = sizeof(T); i-- > 0;) {
                --bytes[i];
                if (bytes[i] != (unsigned char)(-1))
                    break;
            }
        } else if constexpr (order == stl20::endian::little) {
            for (unsigned i = 0; i < sizeof(T); i++) {
                --bytes[i];
                if (bytes[i] != (unsigned char)(-1))
                    break;
            }
        }
        return *this;
    }
};
#pragma pack(pop)

template <typename T>
using LittleEndian = Endian<T, stl20::endian::little>;
template <typename T>
using BigEndian = Endian<T, stl20::endian::big>;

using int16le = LittleEndian<int16_t>;
using uint16le = LittleEndian<uint16_t>;
using int32le = LittleEndian<int32_t>;
using uint32le = LittleEndian<uint32_t>;
using int64le = LittleEndian<int64_t>;
using uint64le = LittleEndian<uint64_t>;

using int16be = BigEndian<int16_t>;
using uint16be = BigEndian<uint16_t>;
using int32be = BigEndian<int32_t>;
using uint32be = BigEndian<uint32_t>;
using int64be = BigEndian<int64_t>;
using uint64be = BigEndian<uint64_t>;
} // namespace endianness
#endif //__ENDIANNESS__
