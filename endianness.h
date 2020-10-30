// Copyright © 2019 Aleksey Nikolaev
// License MIT: http://opensource.org/licenses/MIT

#ifndef __ENDIANNESS__
#define __ENDIANNESS__

#include <climits>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

#if __cplusplus >= 202002L
#    include <bit> //stl c++20
namespace utils {
using endian = std::endian;

template <typename To, typename From>
using bit_cast = std::bit_cast<To, From>;
} // namespace utils

#else
#    ifdef _MSC_VER
constexpr int __LITTLE_ENDIAN = 1234;
constexpr int __BIG_ENDIAN = 4321;
constexpr int __PDP_ENDIAN = 3412;
constexpr int __BYTE_ORDER = __LITTLE_ENDIAN;
#    else
#        include "endian.h"
#    endif
#    if __BYTE_ORDER != __LITTLE_ENDIAN && __BYTE_ORDER != __BIG_ENDIAN
#        error Only little/big endian are implemented. __PDP_ENDIAN and other are not implemented
#    endif //__BYTE_ORDER
#    include <cstring>
namespace utils {
enum class endian
{
    little = __LITTLE_ENDIAN,
    big = __BIG_ENDIAN,
    native = __BYTE_ORDER
};
template <class To, class From>
std::enable_if_t<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable<From>::value && std::is_trivial<To>::value,
                 // this implementation requires that To is trivially default constructible
                 To>
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
} // namespace utils
#endif

namespace endianness {
// compile-time endianness swap based on http://stackoverflow.com/a/36937049
template <class T, class U, std::size_t... N>
static constexpr U byteSequenceSwap(U i, std::index_sequence<N...>) noexcept
{
    return (((i >> N * CHAR_BIT & std::uint8_t(-1)) << (sizeof(T) - 1 - N) * CHAR_BIT) | ...);
}
template <class T>
static constexpr std::enable_if_t<!std::is_floating_point_v<T>, T> byteSwap(T i) noexcept
{
    return static_cast<T>(byteSequenceSwap<std::make_unsigned_t<T>>(i, std::make_index_sequence<sizeof(T)>{}));
}

template <class T>
std::enable_if_t<std::is_floating_point_v<T>, T> byteSwap(T i) noexcept
{
    return utils::bit_cast<T>(byteSwap(utils::bit_cast<std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>>(i)));
}

#pragma pack(push, 1)
template <typename T, utils::endian order = utils::endian::native>
struct Endian
{
    static_assert(order == utils::endian::little || order == utils::endian::big,
                  "Only little/big endian are implemented. __PDP_ENDIAN and other are not implemented");

    static constexpr bool isNative() noexcept { return order == utils::endian::native || sizeof(T) == 1; }
    using value_type = T;
    T value;

    Endian() noexcept : value(T()) {}
    ~Endian() = default;
    constexpr Endian(T t) noexcept : value(convertFrom<T, utils::endian::native>(t)) {}
    //copy
    constexpr Endian(const Endian &other) noexcept : value(other.value) {}
    template <typename U, utils::endian orderU>
    constexpr Endian(const Endian<U, orderU> &other) noexcept : value(convertFrom<U, orderU>(other.value))
    {}
    Endian &operator=(const Endian &other) noexcept
    {
        if (this != &other)
            value = other.value;
        return *this;
    }
    template <typename U, utils::endian orderU>
    Endian &operator=(const Endian<U, orderU> &other) noexcept
    {
        value = convertFrom<U, orderU>(other);
        return *this;
    }
    //move
    constexpr Endian(Endian &&other) noexcept : value(other.value) {}
    template <typename U, utils::endian orderU>
    constexpr Endian(Endian<U, orderU> &&other) noexcept : value(convertFrom<U, orderU>(other.value))
    {}
    Endian &operator=(Endian &&other) noexcept
    {
        std::swap(value, other.value);
        return *this;
    }
    template <typename U, utils::endian orderU>
    Endian &operator=(Endian<U, orderU> &&other) noexcept
    {
        value = convertFrom<U, orderU>(other);
        return *this;
    }

    //type conversion
    operator T() const noexcept
    {
        if constexpr (isNative()) {
            return value;
        } else {
            return Endian<T, utils::endian::native>(*this).value;
        }
    }

    // operators
    bool operator==(const Endian &other) const noexcept { return (value == other.value); }
    bool operator!=(const Endian &other) const noexcept { return (value != other.value); }
    Endian operator|=(const Endian &other) noexcept
    {
        value |= other.value;
        return *this;
    }
    Endian operator&=(const Endian &other) noexcept
    {
        value &= other.value;
        return *this;
    }
    Endian operator^=(const Endian &other) noexcept
    {
        value ^= other.value;
        return *this;
    }
    Endian operator~() const noexcept
    {
        Endian tmp;
        tmp.value = ~value;
        return tmp;
    }
    Endian operator+=(const Endian &other) noexcept { return (*this = *this + other); }
    Endian operator-=(const Endian &other) noexcept { return (*this = *this - other); }
    Endian operator*=(const Endian &other) noexcept { return (*this = *this * other); }
    Endian operator/=(const Endian &other) noexcept { return (*this = *this / other); }
    Endian operator%=(const Endian &other) noexcept { return (*this = *this % other); }

    // postfix
    Endian operator++(int) noexcept
    {
        Endian tmp(*this);
        operator++();
        return tmp;
    }
    // prefix
    Endian &operator++() noexcept
    {
        if constexpr (isNative()) {
            ++value;
        } else {
            *this += Endian(1);
        }
        return *this;
    }

    Endian operator--(int) noexcept
    {
        Endian tmp(*this);
        operator--();
        return tmp;
    }

    Endian &operator--() noexcept
    {
        if constexpr (isNative()) {
            --value;
        } else {
            *this -= Endian(1);
        }
        return *this;
    }

private:
    template <typename U, utils::endian orderU>
    static constexpr T convertFrom(const U &otherValue) noexcept
    {
        if constexpr (std::is_same_v<T, U>) {
            if constexpr (order == orderU || sizeof(T) == 1)
                return otherValue;
            else
                return byteSwap(otherValue);
        } else if constexpr (sizeof(U) == 1 && sizeof(T) == 1) {
            return static_cast<T>(otherValue);
        } else {
            if constexpr (orderU == utils::endian::native || sizeof(U) == 1)
                return convertFrom<T, utils::endian::native>(static_cast<T>(otherValue));
            else
                return convertFrom<T, utils::endian::native>(static_cast<T>(byteSwap(otherValue)));
        }
    }
};
#pragma pack(pop)

template <typename T>
using LittleEndian = Endian<T, utils::endian::little>;
template <typename T>
using BigEndian = Endian<T, utils::endian::big>;
} // namespace endianness

template <typename T, utils::endian order>
struct std::hash<endianness::Endian<T, order>>
{
    std::size_t operator()(const endianness::Endian<T, order> &number) const noexcept
    {
        return std::hash<T>{}(number.value);
    }
};

#endif //__ENDIANNESS__
