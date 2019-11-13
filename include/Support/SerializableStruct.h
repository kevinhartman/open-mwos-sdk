#pragma once

#include <array>
#include <tuple>
#include <string>

namespace serializer_internal {
template<typename ... T>
struct types {
};

template<size_t i, typename T>
struct int_to_type {
    using type = T;
};

template<size_t N, size_t ... Rest>
struct type_sequence_gen {
    template<typename T>
    using type = typename type_sequence_gen<N - 1, N, Rest...>::template type<T>;
};

template<size_t ... Rest>
struct type_sequence_gen<0, Rest...> {
    template<typename T>
    using type = types<typename int_to_type<Rest, T>::type...>;
};

template<typename Tuple, typename ... Inputs>
struct struct_gen {
};

// Next type to process is a types list. Unwrap.
template<typename ... Ts, typename ... Heads, typename ...Tail>
struct struct_gen<std::tuple<Ts...>, types<Heads...>, Tail...> {
    using type = typename struct_gen<std::tuple<Ts...>, Heads..., Tail...>::type;
};

template<typename ... Ts, typename Head, typename ...Tail>
struct struct_gen<std::tuple<Ts...>, Head, Tail...> {
    using type = typename struct_gen<std::tuple<Ts..., Head>, Tail...>::type;
};

template<typename ...Ts>
struct struct_gen<std::tuple<Ts...>> {
    using type = std::tuple<Ts...>;
};
}

template<typename ... Ts>
using SerializableStruct = typename ::serializer_internal::struct_gen<std::tuple<>, Ts...>::type;

template<typename T, size_t N>
using SerializableArray = std::array<T, N>;

using SerializableString = std::basic_string<char>;

template<typename T, size_t N>
using SequenceOfType = typename ::serializer_internal::type_sequence_gen<N>::template type<T>;

template<size_t N>
using PadBytes = std::array<char, N>;