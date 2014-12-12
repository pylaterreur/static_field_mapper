#include <exception>
#include <iostream>
#include <utility>
#include <tuple>
#include <cassert>

// TODO: change the return type for get_new (something that could tell if there is a non-valid index, instead of -1)

template <unsigned ...Args>
struct Mapper
{
  template <unsigned Value, bool Dummy = false>
  struct get
  {
    static_assert(Dummy == true, "List overflow, mate");
  };

  static constexpr unsigned get_new(unsigned)
  {
    return (-1);
  }
};

template <unsigned First, unsigned ...Others>
struct Mapper<First, Others...> : Mapper<Others...>
{
  template <unsigned Value, bool Dummy = false>
  struct get;
  static constexpr unsigned get_new(unsigned index_to_map)
  {
    return (index_to_map == First) ? 0 : Mapper<Others...>::get_new(index_to_map) + 1;
  }
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

template <unsigned ...Args>
struct UniqueList;

template <unsigned First, unsigned Second, unsigned ...Args>
struct UniqueList<First, Second, Args...>
{
  static const bool value = First != Second && UniqueList<First, Args...>::value && UniqueList<Second, Args...>::value;
};

template <unsigned Last>
struct UniqueList<Last>
{
  static const bool value = true;
};

template <unsigned ...Args>
struct unique_sequence
{
  static_assert(UniqueList<Args...>::value, "Sequence should be composed of unique elements.");
};

template <typename T, unsigned ...Args>
class translated_array : public unique_sequence<Args...>, private std::array<T, sizeof...(Args)>
{
  typedef std::array<T, sizeof...(Args)> Dad;
public:
  const T& operator[](unsigned index_to_map) const
  {
    return Dad::operator[](Mapper<Args...>::get_new(index_to_map));
  }

  T& operator[](unsigned index_to_map)
  {
    return const_cast<T&>(const_cast<const translated_array&>(*this)[index_to_map]);
  }

  static constexpr bool outofbound(unsigned index_to_map)
  {
    return (-1 == Mapper<Args...>::get_new(index_to_map));
  }

  using Dad::begin;
  using Dad::end;
  using Dad::size;
};

template <typename T>
struct ZeroInitialized
{
  ZeroInitialized() = default;
  ZeroInitialized(const ZeroInitialized&) = delete;
  ZeroInitialized(const T& t) : t_(t)
  {}

  operator const T&() const
  {
    return t_;
  }

  operator T&()
  {
    return const_cast<const ZeroInitialized&>(this);
  }

private:
  T t_{};
};

int main()
{
    std::cout << Mapper<8, 41>::get<41>() << std::endl;
    std::cout << Mapper<8, 41>::get_new(41) << std::endl;
    translated_array<
      ZeroInitialized<
	const char*>
      , 1, 2, 3> arr;

    const char *message =
      "8=foo\x01"
      "41=baz\x01"
      "23=bar\x01";

    {
      enum class State
      {
	Key,
	Value
      };

      State s = State::Key;
      while (*message)
	{
	  if (*message != '=' && *message != '\x01' && *message != '\0')
	    {
	      unsigned key = 0;
	      while ('0' <= *message && *message <= '9')
		{
		  key = key * 10 + *message - '0';
		  ++message;
		}
	      if (*message != '=' || !key)
		{
		  throw std::runtime_error("invalid value for key");
		}
	      if (!arr.outofbound(key))
		{
		  arr[key] = message + 1;
		}
	      while (*message != '\x01' && *message != '\0')
		{
		  ++message;
		}
	      if (*message == '\x01')
		{
		  ++message;
		}
	    }
	  else
	    {
	      throw std::runtime_error("parse key error");
	    }
	}
    }
    std::cout << arr.size() << std::endl;
    for (const auto &field : arr)
      {
	if (field)
	  {
	    std::cout << "field inserted:" << field << std::endl;
	  }
      }
}
