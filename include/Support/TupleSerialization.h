#pragma once

#include "Endian.h"

#include <tuple>
#include <iostream>

namespace serializer_internal {
// TODO: rewrite this if possible to avoid duplicating everything for const overloads
template<support::Endian Endian, typename Visitor, bool ShouldSwap = Endian != support::Endian::ignore && Endian != support::HostEndian>
struct TupleTraverser {
    // TODO: add specializations for single byte scalars
    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Write(T& field, std::iostream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Write(const T& field, std::iostream& output) {
        Visitor::VisitScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Write(T& field, std::iostream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Write(const T& field, std::iostream& output) {
        Visitor::VisitSwappedScalar(field, output);
    }

    template<size_t I>
    static void Write(std::array<char, I>& field, std::iostream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<size_t I>
    static void Write(const std::array<char, I>& field, std::iostream& output) {
        Visitor::VisitCharArray(field, output);
    }

    template<typename T, size_t I>
    static void Write(std::array<T, I>& field, std::iostream& output) {
        printf("array\n");

        for (auto& element : field) {
            Write(element, output);
        }

        printf("end array\n");
    }

    template<typename T, size_t I>
    static void Write(const std::array<T, I>& field, std::iostream& output) {
        printf("array\n");

        for (auto& element : field) {
            Write(element, output);
        }

        printf("end array\n");
    }

    template<typename T, typename std::enable_if<!std::is_scalar<T>::value, int>::type = 0>
    static void Write(const T& field, std::iostream& output) {
        static_assert(std::is_scalar<T>::value, "Serializer only supports std::tuple and std::array containers and scalar elements.");
        printf("non scalar;\n");
    }

    template<typename ... Ts, size_t ... Is>
    static void Write(std::tuple<Ts...>& structure, std::iostream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{ 1,
                       (Write(std::get<Is>(structure), output), void(), int{})...
        };
    }

    template<typename ... Ts, size_t ... Is>
    static void Write(const std::tuple<Ts...>& structure, std::iostream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{ 1,
                       (Write(std::get<Is>(structure), output), void(), int{})...
        };
    }

    template<typename ... Ts>
    static void Write(std::tuple<Ts...>& structure, std::iostream& output) {
        Write(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<typename ... Ts>
    static void Write(const std::tuple<Ts...>& structure, std::iostream& output) {
        Write(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }
};

struct TupleElementWriter {
    template <typename T>
    static void VisitSwappedScalar(const T& field, std::iostream& stream) {
        T copy = field;
        support::EndianSwap(&copy);
        stream.write(reinterpret_cast<const char *>(&copy), sizeof(T));
        printf("Write SwappedScalar]\n");
    }

    template <typename T>
    static void VisitScalar(const T& field, std::iostream& stream) {
        stream.write(reinterpret_cast<const char *>(&field), sizeof(T));
        printf("Write Scalar\n");
    }

    template<size_t I>
    static void VisitCharArray(const std::array<char, I>& field, std::iostream& stream) {
        stream.write(field.data(), field.size());
        printf("Write CharArray\n");
    }
};

struct TupleElementReader {
    template <typename T>
    static void VisitSwappedScalar(T& field, std::iostream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        support::EndianSwap(&field);
        printf("Read SwappedScalar]\n");
    }

    template <typename T>
    static void VisitScalar(T& field, std::iostream& stream) {
        stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        printf("Read Scalar\n");
    }

    template<size_t I>
    static void VisitCharArray(std::array<char, I>& field, std::iostream& stream) {
        stream.read(field.data(), field.size());
        printf("Read CharArray\n");
    }
};

}

namespace serializer {

template<support::Endian Endian, typename ...Ts>
void SerializeTuple(const std::tuple<Ts...>& structure, std::iostream& output) {
    using namespace serializer_internal;
    TupleTraverser<Endian, TupleElementWriter>::Write(structure, output);
}

template<support::Endian Endian, typename ...Ts>
void DeserializeTuple(std::tuple<Ts...>& structure, std::iostream& input) {
    using namespace serializer_internal;
    TupleTraverser<Endian, TupleElementReader>::Write(structure, input);
}

}