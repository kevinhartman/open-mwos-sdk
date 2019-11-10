#pragma once

#include "Endian.h"

#include <tuple>
#include <iostream>

namespace serializer_internal {
// TODO: rewrite this if possible to avoid duplicating everything for const overloads
template<support::Endian Endian, typename Visitor, typename Stream, bool ShouldSwap = Endian != support::Endian::ignore && Endian != support::HostEndian>
struct TupleTraverser {
    // TODO: add specializations for single byte scalars
    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Write(T& field, Stream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Write(const T& field, Stream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Write(T& field, Stream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Write(const T& field, Stream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<size_t I>
    static void Write(std::array<char, I>& field, Stream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<size_t I>
    static void Write(const std::array<char, I>& field, Stream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<typename T, size_t I>
    static void Write(std::array<T, I>& field, Stream& output) {
        printf("array\n");

        for (auto& element : field) {
            Write(element, output);
        }

        printf("end array\n");
    }

    template<typename T, size_t I>
    static void Write(const std::array<T, I>& field, Stream& output) {
        printf("array\n");

        for (auto& element : field) {
            Write(element, output);
        }

        printf("end array\n");
    }

    template<typename T, typename std::enable_if<!std::is_scalar<T>::value, int>::type = 0>
    static void Write(const T& field, Stream& output) {
        static_assert(std::is_scalar<T>::value, "Serializer only supports std::tuple and std::array containers and scalar elements.");
        printf("non scalar;\n");
    }

    template<typename ... Ts, size_t ... Is>
    static void Write(std::tuple<Ts...>& structure, Stream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{ 1,
                       (Write(std::get<Is>(structure), output), void(), int{})...
        };
    }

    template<typename ... Ts, size_t ... Is>
    static void Write(const std::tuple<Ts...>& structure, Stream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{ 1,
                       (Write(std::get<Is>(structure), output), void(), int{})...
        };
    }

    template<typename ... Ts>
    static void Write(std::tuple<Ts...>& structure, Stream& output) {
        Write(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<typename ... Ts>
    static void Write(const std::tuple<Ts...>& structure, Stream& output) {
        Write(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }
};

struct TupleElementWriter {
    template <typename T>
    static void VisitSwappedScalar(const T& field, std::ostream& stream) {
        T copy = field;
        support::EndianSwap(&copy);
        stream.write(reinterpret_cast<const char *>(&copy), sizeof(T));
        printf("Write SwappedScalar]\n");
    }

    template <typename T>
    static void VisitScalar(const T& field, std::ostream& stream) {
        stream.write(reinterpret_cast<const char *>(&field), sizeof(T));
        printf("Write Scalar\n");
    }

    template<size_t I>
    static void VisitCharArray(const std::array<char, I>& field, std::ostream& stream) {
        stream.write(field.data(), field.size());
        printf("Write CharArray\n");
    }
};

struct TupleElementReader {
    template <typename T>
    static void VisitSwappedScalar(T& field, std::istream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        support::EndianSwap(&field);
        printf("Read SwappedScalar]\n");
    }

    template <typename T>
    static void VisitScalar(T& field, std::istream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        printf("Read Scalar\n");
    }

    template<size_t I>
    static void VisitCharArray(std::array<char, I>& field, std::istream& stream) {
        stream.read(field.data(), field.size());
        printf("Read CharArray\n");
    }
};

}

namespace serializer {

template<support::Endian Endian, typename Tuple, typename OStream, typename ...Ts>
void SerializeTuple(const Tuple& structure, OStream& output) {
    using namespace serializer_internal;
    TupleTraverser<Endian, TupleElementWriter, OStream>::Write(structure, output);
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
    TupleTraverser<Endian, TupleElementReader, IStream>::Write(structure, input);
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