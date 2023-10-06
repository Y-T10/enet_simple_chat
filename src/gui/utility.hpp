#pragma once

#include <type_traits>
#include <cstddef>
#include <tuple>

template<class T, std::size_t N, class ...Params>
struct pop_front_impl : pop_front_impl<T, N - 1, std::tuple_element_t<N - 1, T>, Params...> {};

template<class T, class ...Params>
struct pop_front_impl<T, 1, Params...> {
    using type = std::tuple<Params...>;
};

template<class T, class ...Params>
struct pop_front_impl<T, 0, Params...> {
    using type = std::tuple<>;
};

template<class Tuple>
struct tuple_pop_front : public pop_front_impl<Tuple, std::tuple_size_v<Tuple>>{
};

static_assert(std::is_same_v<std::tuple<>, typename tuple_pop_front<std::tuple<>>::type>);
static_assert(std::is_same_v<std::tuple<>, typename tuple_pop_front<std::tuple<int>>::type>);
static_assert(std::is_same_v<std::tuple<char>, tuple_pop_front<std::tuple<int,char>>::type>);
static_assert(std::is_same_v<std::tuple<char, double>, tuple_pop_front<std::tuple<int,char,double>>::type>);
static_assert(std::is_same_v<std::tuple<char, double, int>, tuple_pop_front<std::tuple<int,char,double, int>>::type>);

template <class Tuple, class T>
struct check_tuple : 
    std::conditional_t<
        (std::tuple_size_v<Tuple> > 0),
        std::conditional_t<
            !std::is_same_v<std::tuple_element_t<0, Tuple>, T>,
            check_tuple<typename tuple_pop_front<Tuple>::type, T>,
            std::true_type>,
        std::false_type>{ };

template <class T>
struct check_tuple<std::tuple<>, T> : std::false_type { };

template <class Tuple, class T>
static constexpr auto check_tuple_v = check_tuple<Tuple, T>::value;

static_assert(!check_tuple_v<std::tuple<>, int>);
static_assert(check_tuple_v<std::tuple<int>, int>);
static_assert(check_tuple_v<std::tuple<char, double>, double>);
static_assert(!check_tuple_v<std::tuple<char, double, int>, float>);