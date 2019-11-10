#pragma once

#include "Endian.h"

#include <tuple>
#include <iostream>

namespace serializer_internal {
// TODO: rewrite this if possible to avoid duplicating everything for const overloads
template<support::Endian Endian, typename Visitor, typename Stream, bool ShouldSwap = Endian != support::Endian::ignore && Endian != support::HostEndian>
struct TupleVisitor {
    // TODO: add specializations for single byte scalars
    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Visit(T& field, Stream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Visit(const T& field, Stream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Visit(T& field, Stream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Visit(const T& field, Stream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<size_t I>
    static void Visit(std::array<char, I>& field, Stream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<size_t I>
    static void Visit(const std::array<char, I>& field, Stream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<typename T, size_t I>
    static void Visit(std::array<T, I>& field, Stream& output) {
        for (auto& element : field) {
            Visit(element, output);
        }
    }

    template<typename T, size_t I>
    static void Visit(const std::array<T, I>& field, Stream& output) {
        for (auto& element : field) {
            Visit(element, output);
        }
    }

    template<typename T, typename std::enable_if<!std::is_scalar<T>::value, int>::type = 0>
    static void Visit(const T& field, Stream& output) {
        static_assert(std::is_scalar<T>::value, "Serializer only supports std::tuple and std::array containers and scalar elements.");
    }

    template<typename ... Ts, size_t ... Is>
    static void Visit(std::tuple<Ts...>& structure, Stream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{(Visit(std::get<Is>(structure), output), int{})...};
    }

    template<typename ... Ts, size_t ... Is>
    static void Visit(const std::tuple<Ts...>& structure, Stream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{(Visit(std::get<Is>(structure), output), int{})...};
    }

    template<typename ... Ts>
    static void Visit(std::tuple<Ts...>& structure, Stream& output) {
        Visit(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<typename ... Ts>
    static void Visit(const std::tuple<Ts...>& structure, Stream& output) {
        Visit(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }
};

struct TupleElementWriter {
    template <typename T>
    static void VisitSwappedScalar(const T& field, std::ostream& stream) {
        T copy = field;
        support::EndianSwap(&copy);
        stream.write(reinterpret_cast<const char *>(&copy), sizeof(T));
    }

    template <typename T>
    static void VisitScalar(const T& field, std::ostream& stream) {
        stream.write(reinterpret_cast<const char *>(&field), sizeof(T));
    }

    template<size_t I>
    static void VisitCharArray(const std::array<char, I>& field, std::ostream& stream) {
        stream.write(field.data(), field.size());
    }
};

struct TupleElementReader {
    template <typename T>
    static void VisitSwappedScalar(T& field, std::istream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        support::EndianSwap(&field);
    }

    template <typename T>
    static void VisitScalar(T& field, std::istream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
    }

    template<size_t I>
    static void VisitCharArray(std::array<char, I>& field, std::istream& stream) {
        stream.read(field.data(), field.size());
    }
};

}

namespace serializer {

template<support::Endian Endian, typename Tuple, typename OStream, typename ...Ts>
void SerializeTuple(const Tuple& structure, OStream& output) {
    using namespace serializer_internal;
    TupleVisitor<Endian, TupleElementWriter, OStream>::Visit(structure, output);
}

template <typename Tuple, typename OStream>
static void SerializeTuple(const Tuple& tuple, OStream& file, support::Endian endianness) {
    switch (endianness) {
        case support::Endian::big:
            SerializeTuple<support::Endian::big>(tuple, file);
            break;
        case support::Endian::little:
            SerializeTuple<support::Endian::big>(tuple, file);
            break;
        case support::Endian::ignore:
            SerializeTuple<support::Endian::ignore>(tuple, file);
            break;
    }
}

template<support::Endian Endian, typename Tuple, typename IStream, typename ...Ts>
void DeserializeTuple(Tuple& structure, IStream& input) {
    using namespace serializer_internal;
    TupleVisitor<Endian, TupleElementReader, IStream>::Visit(structure, input);
}

template <typename Tuple, typename IStream>
static void DeserializeTuple(const Tuple& tuple, IStream& file, support::Endian endianness) {
    switch (endianness) {
        case support::Endian::big:
            DeserializeTuple<support::Endian::big>(tuple, file);
            break;
        case support::Endian::little:
            DeserializeTuple<support::Endian::big>(tuple, file);
            break;
        case support::Endian::ignore:
            DeserializeTuple<support::Endian::ignore>(tuple, file);
            break;
    }
}

}