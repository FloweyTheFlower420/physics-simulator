#ifndef __PHY_VEC_H__
#define __PHY_VEC_H__
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <bit>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <numeric>
#include <cmath>

namespace phy
{
    constexpr float invsqrt(float number) noexcept
    {
        static_assert(std::numeric_limits<float>::is_iec559);

        float const y = std::bit_cast<float>(0x5f3759df - (std::bit_cast<std::uint32_t>(number) >> 1));
        return y * (1.5f - (number * 0.5f * y * y));
    }

    constexpr double invsqrt(double number)
    {
        float const y = std::bit_cast<double>(0x5fe6eb50c7b537a9 - (std::bit_cast<std::uint64_t>(number) >> 1));
        return y * (1.5f - (number * 0.5f * y * y));
    }

    template <typename T, std::size_t n>
    class vec
    {
        T v[n];
    public:
        constexpr vec(std::initializer_list<T> il)
        {
            auto b = il.begin();
            for (std::size_t i = 0; i < n; i++)
                v[i] = *b++;
        }

        constexpr vec() : v()
        {
            for (auto& i : v)
                i = 0;
        }

        constexpr vec(const vec& rhs) = default;

        template <typename V>
        constexpr vec(const vec<V, n>& rhs)
        {
            for (std::size_t i = 0; i < n; i++)
                v[i] = rhs[i];
        }

        constexpr vec(vec&& rhs) = default;

        constexpr T& operator[](std::size_t i) { return v[i]; }
        constexpr T operator[](std::size_t i) const { return v[i]; }

        template <std::convertible_to<T> V>
        constexpr const vec operator=(const vec<V, n>& vec)
        {
            for (std::size_t i = 0; i < n; i++)
                v[i] = vec[i];
            return *this;
        }

        constexpr const vec& operator=(const vec& vec)
        {
            for (std::size_t i = 0; i < n; i++)
                v[i] = vec[i];
            return *this;
        }

        constexpr auto operator<=>(const vec& rhs)
        {
            std::strong_ordering o = std::strong_ordering::equal;
            for (std::size_t i = 0; i < n; i++)
            {
                o = v[i] <=> rhs[i];
                if (v[i] != rhs[i])
                    return o;
            }

            return o;
        }

        constexpr vec normalize() const 
        {
            vec ret;
            T sum = 0;
            for(auto i : v)
                sum += i * i;
            sum = invsqrt(sum);
            for(std::size_t i = 0; i < n; i++)
                ret[i] = sum * v[i];
            return ret;
        }

        constexpr T dot(const vec& rhs) const
        {
            T ret = 0;
            for(std::size_t i = 0; i < n; i++)
                ret += v[i] * rhs[i];
            return ret;
        }

        constexpr double magnitude() const
        {
            double d = dot(*this);
            if(d < 0)
                while(1) {}
            return std::sqrt(dot(*this));
        }

        constexpr T angle() const requires(n == 2)
        {
            return std::atan2(v[0], v[1]) * 57.2958;
        }

        constexpr void magnitude(T t)
        {
            *this = this->normalize() * t;
        }

        constexpr T* begin() { return v; }
        constexpr T* end() { return v + n; }

        constexpr const T* begin() const { return v; }
        constexpr const T* end() const { return v + n; }

        constexpr const T* cbegin() { return v; }
        constexpr const T* cend() { return v + n; }


        constexpr vec operator-()
        {
            vec<T, n> ret;
            for (std::size_t i = 0; i < n; i++)
                ret[i] = -(T)(v[i]);
            return ret;
        }
    };

    namespace detail
    {
        template <typename T, typename V>
        concept addable = requires(T t, V v)
        {
            { t + v } -> std::convertible_to<T>;
        };

        template <typename T, typename V>
        concept subtractable = requires(T t, V v)
        {
            { t - v } -> std::convertible_to<T>;
        };

        template <typename T, typename V>
        concept multipliable = requires(T t, V v)
        {
            { t * v } -> std::convertible_to<T>;
        };

        template <typename T, typename V>
        concept divisible = requires(T t, V v)
        {
            { t / v } -> std::convertible_to<T>;
        };
    } // namespace detail

    template <typename T, typename V, std::size_t n>
    requires(detail::addable<T, V>) constexpr vec<T, n> operator+(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] + rhs[i]);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::subtractable<T, V>) constexpr vec<T, n> operator-(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] - rhs[i]);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::multipliable<T, V>) constexpr vec<T, n> operator*(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] * rhs[i]);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::divisible<T, V>) constexpr vec<T, n> operator/(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] / rhs[i]);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::addable<T, V>) constexpr const vec<T, n>& operator+=(vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] + rhs[i]);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::subtractable<T, V>) constexpr const vec<T, n>& operator-=(vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] - rhs[i]);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::multipliable<T, V>) constexpr const vec<T, n>& operator*=(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] * rhs[i]);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::divisible<T, V>) constexpr const vec<T, n>& operator/=(const vec<T, n>& lhs, const vec<V, n>& rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] / rhs[i]);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::multipliable<T, V>) constexpr vec<T, n> operator*(const vec<T, n>& lhs, V rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] * rhs);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::divisible<T, V>) constexpr vec<T, n> operator/(const vec<T, n>& lhs, V rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(lhs[i] / rhs);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::multipliable<T, V>) constexpr const vec<T, n>& operator*=(const vec<T, n>& lhs, V rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] * rhs);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::divisible<T, V>) constexpr const vec<T, n>& operator/=(const vec<T, n>& lhs, V rhs)
    {
        for (std::size_t i = 0; i < n; i++)
            lhs[i] = (T)(lhs[i] / rhs);
        return lhs;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::multipliable<T, V>) constexpr vec<T, n> operator*(V lhs, const vec<T, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(rhs[i] * lhs);
        return v;
    }

    template <typename T, typename V, std::size_t n>
    requires(detail::divisible<T, V>) constexpr vec<T, n> operator/(V lhs, const vec<T, n>& rhs)
    {
        vec<T, n> v;
        for (std::size_t i = 0; i < n; i++)
            v[i] = (T)(rhs[i] / lhs);
        return v;
    }

    template <typename T>
    using vec2 = vec<T, 2>;
    template <typename T>
    using vec3 = vec<T, 3>;

    using vec2i = vec2<int>;
    using vec2f = vec2<float>;
    using vec2d = vec2<double>;

    using vec3i = vec3<int>;
    using vec3f = vec3<float>;
    using vec3d = vec3<double>;

    template<typename U, typename T>
    constexpr sf::Vector2<U> vector_cast(const vec<T, 2>& rhs)
    {
        return sf::Vector2<U>((U)rhs[0], (U)rhs[1]);
    }

    template<typename U, typename T>
    constexpr sf::Vector3<T> vector_cast(const vec<T, 3>& rhs)
    {
        return sf::Vector3<T>((U)rhs[0], (U)rhs[1], (U)rhs[2]);
    }

    template<typename U, typename T>
    constexpr vec2<U> vector_cast(const sf::Vector2<T>& rhs)
    {
        return vec2<U>{(U)rhs.x, (U)rhs.y};
    }

    template<typename U, typename T>
    constexpr vec3<U> vector_cast(const sf::Vector3<T>& rhs)
    {
        return vec3<U>{(U)rhs.x, (U)rhs.y, (U)rhs.z};
    }
} // namespace phy
#endif
