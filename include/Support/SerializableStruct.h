#pragma once

#include <tuple>

template <typename ... T>
struct types {};

template <int i, typename T>
struct int_to_type {
    using type = T;
};

template <int N, int ... Rest>
struct type_sequence_gen {
    template <typename T>
    using type = typename type_sequence_gen<N - 1, N, Rest...>::template type<T>;
};

template <int ... Rest>
struct type_sequence_gen<0, Rest...> {
    template <typename T>
    using type = types<typename int_to_type<Rest, T>::type...>;
};

template <typename T, int N>
using type_sequence = typename type_sequence_gen<N>::template type<T>;

template <typename Tuple, typename ... Inputs>
struct struct_gen {};

// Next type to process is a types list. Unwrap.
template <typename ... Ts, typename ... Heads, typename ...Tail>
    struct struct_gen<std::tuple<Ts...>, types<Heads...>, Tail...> {
    using type = typename struct_gen<std::tuple<Ts...>, Heads..., Tail...>::type;
};

template <typename ... Ts, typename Head, typename ...Tail>
struct struct_gen<std::tuple<Ts...>, Head, Tail...> {
    using type = typename struct_gen<std::tuple<Ts..., Head>, Tail...>::type;
};

template <typename ...Ts>
struct struct_gen<std::tuple<Ts...>> {
    using type = std::tuple<Ts...>;
};

template <typename ... Ts>
using SerializableStruct = typename struct_gen<std::tuple<>, Ts...>::type;