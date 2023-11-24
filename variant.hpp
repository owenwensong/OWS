/*!*****************************************************************************
 * @file    variant.hpp
 * @author  Owen Huang Wensong
 * @date    21 NOV 2023
 * @brief   STL include only variant designed for C++11.
 *
 * @par     Copyright (c) 2023 Owen Huang Wensong.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*******************************************************************************/

#ifndef HEADER_GUARD_OWS_VARIANT_HPP
#define HEADER_GUARD_OWS_VARIANT_HPP

// Some of these may be included in each other.
// However some nested includes are implementation dependent,
// so include headers which guarantee inclusion of dependencies.
#include <string> // exception str
#include <limits> // numeric_limits
#include <cstddef>// size_t
#include <utility>// std::forward
#include <exception>  // std::exception
#include <type_traits>// is_same, integral_constant, remove_reference, remove_cv

namespace OWS
{
  // ***************************************************************************
  // *************************************************************** detail ****

  namespace detail
  {
    namespace vrnt
    {
      // Compile Time Min Max
      template <typename T, T A, T B, T... Cs>
      struct CTMM
      {
        static constexpr T s_min{ CTMM<T, CTMM<T, A, B>::s_min, Cs...>::s_min };
        static constexpr T s_max{ CTMM<T, CTMM<T, A, B>::s_max, Cs...>::s_max };
      };

      // Compile Time Min Max
      template <typename T, T A, T B>
      struct CTMM<T, A, B>
      {
        static constexpr T s_min{ A > B ? B : A };
        static constexpr T s_max{ A > B ? A : B };
      };

      // pre C++17 disjunction
      template <typename R, typename T, typename... Ts>
      struct is_any : public std::integral_constant<bool, std::is_same<R, T>::value || is_any<R, Ts...>::value>{};

      template <typename R, typename T> // template specialization floor
      struct is_any<R, T> : public std::is_same<R, T>{};

      // pre C++17 conjunction
      template <typename R, typename T, typename... Ts>
      struct is_all : public std::integral_constant<bool, std::is_same<R, T>::value && is_all<R, Ts...>::value>{};

      template <typename R, typename T> // template specialization floor
      struct is_all<R, T> : public std::is_same<R, T>{};

      // no duplicate types in parameter pack
      template <typename T, typename... Ts>
      struct is_unique : public std::integral_constant<bool, !is_any<T, Ts...>::value && is_unique<Ts...>::value>{};

      template <typename T> // template specialization floor
      struct is_unique<T> : public std::integral_constant<bool, true>{};

      template <size_t I, typename T, typename... Ts>
      struct IthType{ using type = typename IthType<I - 1, Ts...>::type; };

      template <typename T, typename... Ts> // template specialization floor
      struct IthType<0, T, Ts...>{ using type = T; };

      template <size_t I, typename R, typename T, typename... Ts>
      struct IFromType : public std::integral_constant<typename std::enable_if<is_any<R, T, Ts...>::value, size_t>::type, IFromType<I + 1, R, Ts...>::value>{};

      template <size_t I, typename R, typename... Ts>
      struct IFromType<I, R, R, Ts...> : public std::integral_constant<size_t, I>{};

      template <typename T>
      struct remove_cvref : public std::remove_cv<typename std::remove_reference<T>::type> {};
    }
  }

#if OWS_SMOKE_TEST
  static_assert(-5 == OWS::detail::vrnt::CTMM<int, -5, 3, 6, 1, 5>::s_min, "detail: CTMM logic failure");
  static_assert(6  == OWS::detail::vrnt::CTMM<int, -5, 3, 6, 1, 5>::s_max, "detail: CTMM logic failure");
  static_assert(1  == OWS::detail::vrnt::CTMM<size_t,  3, 6, 1, 5>::s_min, "detail: CTMM logic failure");
  static_assert(6  == OWS::detail::vrnt::CTMM<size_t,  3, 6, 1, 5>::s_max, "detail: CTMM logic failure");

  static_assert(true  == OWS::detail::vrnt::is_any<bool, char, int, float, bool>::value,  "detail: is_any compile time logic failure");
  static_assert(false == OWS::detail::vrnt::is_any<bool, char, int, float>::value,        "detail: is_any compile time logic failure");
  static_assert(false == OWS::detail::vrnt::is_all<bool, char, int, float>::value,        "detail: is_all compile time logic failure");
  static_assert(true  == OWS::detail::vrnt::is_all<bool, bool>::value,                    "detail: is_all compile time logic failure");
  static_assert(true  == OWS::detail::vrnt::is_all<bool, bool, bool, bool>::value,        "detail: is_all compile time logic failure");
  static_assert(false == OWS::detail::vrnt::is_all<bool, bool, bool, bool, float>::value, "detail: is_all compile time logic failure");

  static_assert(false == OWS::detail::vrnt::is_unique<bool, bool, int, float, double>::value,   "detail: is_unique compile time logic failure");
  static_assert(false == OWS::detail::vrnt::is_unique<bool, float, int, float, double>::value,  "detail: is_unique compile time logic failure");
  static_assert(true  == OWS::detail::vrnt::is_unique<bool, char, int, float, double>::value,   "detail: is_unique compile time logic failure");
  static_assert(true  == OWS::detail::vrnt::is_unique<bool, char, int, float, double>::value,   "detail: is_unique compile time logic failure");

  static_assert(true  == std::is_same<bool, typename OWS::detail::vrnt::IthType<0, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(true  == std::is_same<char, typename OWS::detail::vrnt::IthType<1, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(true  == std::is_same<int,  typename OWS::detail::vrnt::IthType<2, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<bool, typename OWS::detail::vrnt::IthType<1, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<bool, typename OWS::detail::vrnt::IthType<2, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<char, typename OWS::detail::vrnt::IthType<0, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<char, typename OWS::detail::vrnt::IthType<2, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<int,  typename OWS::detail::vrnt::IthType<0, bool, char, int>::type>::value, "detail: IthType compile time logic failure");
  static_assert(false == std::is_same<int,  typename OWS::detail::vrnt::IthType<1, bool, char, int>::type>::value, "detail: IthType compile time logic failure");

  static_assert(0 == OWS::detail::vrnt::IFromType<0, bool,  bool, char, int, float>::value, "detail: IFromType compile time logic failure");
  static_assert(1 == OWS::detail::vrnt::IFromType<0, char,  bool, char, int, float>::value, "detail: IFromType compile time logic failure");
  static_assert(2 == OWS::detail::vrnt::IFromType<0, int,   bool, char, int, float>::value, "detail: IFromType compile time logic failure");
  static_assert(3 == OWS::detail::vrnt::IFromType<0, float, bool, char, int, float>::value, "detail: IFromType compile time logic failure");
#endif // OWS_SMOKE_TEST

  // *************************************************************** detail ****
  // ***************************************************************************

  class bad_variant_access : public std::exception
  {
  public:

    ~bad_variant_access() = default;
    bad_variant_access() = default;
    template <typename... Args> // args supporting string operator+=
    explicit bad_variant_access(Args&&... args) : m_msg{ "bad_variant_access: " }
    {
      int unpack[]{ 0, (m_msg += std::forward<Args>(args), 0)... };
      static_cast<void>(unpack);
    }
    bad_variant_access(bad_variant_access const&) = default;
    bad_variant_access(bad_variant_access&&) = default;
    bad_variant_access& operator=(bad_variant_access const&) = default;
    bad_variant_access& operator=(bad_variant_access&&) = default;

    const char* what() const noexcept override { return m_msg.c_str(); }

  private:
    std::string m_msg;
  };

  template <typename... Ts>
  class alignas(Ts...) Variant
  {
  public:

    static_assert(true == detail::vrnt::is_unique<Ts...>::value, "variant should have unique parameter list");

    unsigned index() const noexcept { return m_Idx; }
    bool valueless() const noexcept { return m_Idx == s_Valueless; }

    template <typename T>
    bool holds_alternative() const noexcept
    {
      constexpr auto typeIdx{ detail::vrnt::IFromType<0, T, Ts...>::value };
      return typeIdx == m_Idx;
    }

    template <typename T>
    typename std::enable_if<detail::vrnt::is_any<T, Ts...>::value, T*>::type get_if() noexcept
    {
      constexpr auto typeIdx{ detail::vrnt::IFromType<0, T, Ts...>::value };
      return typeIdx == m_Idx ? reinterpret_cast<T*>(m_Raw) : nullptr;
    }

    template <typename T>
    typename std::enable_if<detail::vrnt::is_any<T, Ts...>::value, T const*>::type get_if() const noexcept
    {
      constexpr auto typeIdx{ detail::vrnt::IFromType<0, T, Ts...>::value };
      return typeIdx == m_Idx ? reinterpret_cast<T const*>(m_Raw) : nullptr;
    }

    template <size_t I, typename T = typename std::enable_if<I < sizeof...(Ts), typename detail::vrnt::IthType<I, Ts...>::type>::type>
    T* get_if() noexcept
    {
      return I == m_Idx ? reinterpret_cast<T*>(m_Raw) : nullptr;
    }

    template <size_t I, typename T = typename std::enable_if<I < sizeof...(Ts), typename detail::vrnt::IthType<I, Ts...>::type>::type>
    T const* get_if() const noexcept
    {
      return I == m_Idx ? reinterpret_cast<T const*>(m_Raw) : nullptr;
    }

    template <typename T>
    typename std::enable_if<detail::vrnt::is_any<T, Ts...>::value, T&>::type get() 
    {
      constexpr auto typeIdx{ detail::vrnt::IFromType<0, T, Ts...>::value };
      if (typeIdx != m_Idx)throw bad_variant_access{ "get<", std::to_string(typeIdx), "> on variant ", std::to_string(m_Idx) };
      return reinterpret_cast<T&>(m_Raw);
    }
    
    template <typename T>
    typename std::enable_if<detail::vrnt::is_any<T, Ts...>::value, T const&>::type get() const
    {
      constexpr auto typeIdx{ detail::vrnt::IFromType<0, T, Ts...>::value };
      if (typeIdx != m_Idx)throw bad_variant_access{ "get<", std::to_string(typeIdx), "> on variant ", std::to_string(m_Idx) };
      return reinterpret_cast<T const&>(m_Raw);
    }

    template <size_t I, typename T = typename std::enable_if<I < sizeof...(Ts), typename detail::vrnt::IthType<I, Ts...>::type>::type>
    T& get()
    {
      if (I != m_Idx)throw bad_variant_access{ "get<", std::to_string(I), "> on variant ", std::to_string(m_Idx) };
      return reinterpret_cast<T&>(m_Raw);
    }

    template <size_t I, typename T = typename std::enable_if<I < sizeof...(Ts), typename detail::vrnt::IthType<I, Ts...>::type>::type>
    T const& get() const
    {
      if (I != m_Idx)throw bad_variant_access{ "get<", std::to_string(I), "> on variant ", std::to_string(m_Idx) };
      return reinterpret_cast<T const&>(m_Raw);
    }

    ~Variant() noexcept
    {
      if (s_Valueless == m_Idx)return;
      s_Destructors[m_Idx](this); // no need to clear index
    }

    template <typename T, typename... Args>
    typename std::enable_if<detail::vrnt::is_any<T, Ts...>::value, T>::type& emplace(Args&&... args)
    {
      if (s_Valueless != m_Idx)s_Destructors[m_Idx](this);
      m_Idx = detail::vrnt::IFromType<0, T, Ts...>::value;
      return *::new (reinterpret_cast<T*>(m_Raw)) T{ std::forward<Args>(args)... };
    }

    // valueless constructor
    Variant() = default;
    
    explicit Variant(Variant const& other) : Variant{ /* idx initialized in emplace called in TAssign from s_CopyAssigns */ }
    {
      if (s_Valueless == other.m_Idx)return;
      s_CopyAssigns[other.m_Idx](this, const_cast<Variant*>(&other));// internal guaranteed not to modify other
    }

    explicit Variant(Variant&& other) noexcept : Variant{ /* idx initialized in emplace called in TAssign from s_MoveAssigns */ }
    {
      if (s_Valueless == other.m_Idx)return;
      s_MoveAssigns[other.m_Idx](this, &other);// internal moves other contents
      other.m_Idx = s_Valueless;
    }

    // Value initializer
    template <typename T, typename U = typename std::enable_if<detail::vrnt::is_any<typename detail::vrnt::remove_cvref<T>::type, Ts...>::value, T>::type>
    explicit Variant(T&& variant) : Variant{ /* idx initialized in emplace */ }
    {
      emplace<typename detail::vrnt::remove_cvref<U>::type>(std::forward<U>(variant));
    }

    Variant& operator=(Variant const& other)
    {
      if (s_Valueless == other.m_Idx)
      {
        if (s_Valueless != m_Idx)s_Destructors[m_Idx](this);
        m_Idx = s_Valueless;
        return *this;
      }
      s_CopyAssigns[other.m_Idx](this, const_cast<Variant*>(&other));// internal guaranteed not to modify other, this idx set as side effect
      return *this;
    }

    Variant& operator=(Variant&& other) noexcept
    {
      if (s_Valueless == other.m_Idx)
      {
        if (s_Valueless != m_Idx)s_Destructors[m_Idx](this);
        m_Idx = s_Valueless;
        return *this;
      }
      s_MoveAssigns[other.m_Idx](this, &other);// internal moves other contents, this idx set as side effect
      other.m_Idx = s_Valueless;
      return *this;
    }

    // variant type combined copy and move assignment operator requires respective type constructor to be available
    template <typename T, typename U = typename std::enable_if<detail::vrnt::is_any<typename detail::vrnt::remove_cvref<T>::type, Ts...>::value, T>::type>
    Variant& operator=(T&& rhs) noexcept
    {
      emplace<typename detail::vrnt::remove_cvref<T>::type>(std::forward<T>(rhs));
      return *this;
    }

  private:

    template <typename T>
    static inline constexpr void TDestructor(Variant* thisPtr) noexcept
    {
      thisPtr->get<T>().~T();
    }

    template <typename T>
    static inline constexpr void TAssign(Variant* lhsPtr, Variant* rhsPtr)
    { // internal call assumed, no const checking
      lhsPtr->emplace<typename detail::vrnt::remove_cvref<T>::type>(reinterpret_cast<T>(rhsPtr->m_Raw));
    }

    static constexpr size_t s_RawSize{ detail::vrnt::CTMM<size_t, sizeof(Ts)...>::s_max };
    static constexpr unsigned s_Valueless{ std::numeric_limits<unsigned>::max() };

    // static fnptr storage = 3 * sizeof(void*) * sizeof...(Ts)
    static constexpr void (*const s_Destructors[])(Variant*){ TDestructor<Ts>... };
    static constexpr void (*const s_CopyAssigns[])(Variant*, Variant*){ TAssign<Ts const&>... };
    static constexpr void (*const s_MoveAssigns[])(Variant*, Variant*){ TAssign<Ts&&>... };

    char m_Raw[s_RawSize]{};
    unsigned m_Idx{ s_Valueless }; // current variant index

  };

  // linkage for pre C++17 struct static inline constexpr
  template <typename... Ts>
  constexpr void (*const Variant<Ts...>::s_Destructors[])(Variant*);

  // linkage for pre C++17 struct static inline constexpr
  template <typename... Ts>
  constexpr void (*const Variant<Ts...>::s_CopyAssigns[])(Variant*, Variant*);

  // linkage for pre C++17 struct static inline constexpr
  template <typename... Ts>
  constexpr void (*const Variant<Ts...>::s_MoveAssigns[])(Variant*, Variant*);
}

#endif // !HEADER_GUARD_OWS_VARIANT_HPP
