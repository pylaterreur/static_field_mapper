#include <iostream>
#include <utility>
#include <tuple>

template <unsigned ...Args>
struct Mapper
{
  template <unsigned Value, bool Dummy = false>
  struct get
  {
    static_assert(Dummy == true, "List overflow, mate");
  };
};

template <unsigned First, unsigned ...Others>
struct Mapper<First, Others...> : Mapper<Others...>
{
  template <unsigned Value, bool Dummy = false>
  struct get;
};

template <unsigned First, unsigned ...Others>
template <unsigned Value, bool Dummy>
struct Mapper<First, Others...>::get
{
  constexpr get() = default;
  constexpr operator unsigned() const
  {
    return value;
  }
  static const unsigned value = Mapper<Others...>::template get<Value>::value + 1;
};

template <unsigned First, unsigned ...Others>
template <bool Dummy>
struct Mapper<First, Others...>::get<First, Dummy>
{
  constexpr get() = default;
  constexpr operator unsigned() const
  {
    return value;
  }
  static const unsigned value = 0;
};

int main()
{
    std::cout << Mapper<8, 41>::get<41>() << std::endl;
}
