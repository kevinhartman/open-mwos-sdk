#pragma once

#include "Endian.h"

#include <tuple>
#include <ostream>

namespace serializer {
template<support::Endian Endian, bool ShouldSwap = Endian != support::Endian::ignore && Endian != support::HostEndian>
struct ElementWriter {
    // TODO: add specializations for single byte scalars
    template<typename T, typename std::enable_if<std::is_scalar<T>::value && !ShouldSwap, int>::type = 0>
    static void Write(const T& field, std::ostream& output) {
        output.write(reinterpret_cast<const char *>(&field), sizeof(T));
        printf("don't swap;\n");
    }

    template<typename T, typename std::enable_if<std::is_scalar<T>::value && ShouldSwap, int>::type = 0>
    static void Write(const T& field, std::ostream& output) {
        T copy = field;
        support::EndianSwap(&copy);
        output.write(reinterpret_cast<const char *>(&copy), sizeof(T));
        printf("swap;\n");
    }

    template<size_t I>
    static void Write(const std::array<char, I>& field, std::ostream& output) {
        printf("pad;\n");
        output.write(field.data(), field.size());
    }
    
    template<typename T, size_t I>
    static void Write(const std::array<T, I>& field, std::ostream& output) {
        printf("array\n");

        for (auto& element : field) {
            Write(element, output);
        }

        printf("end array\n");
    }

    template<typename T, typename std::enable_if<!std::is_scalar<T>::value, int>::type = 0>
    static void Write(const T& field, std::ostream& output) {
        static_assert(std::is_scalar<T>::value, "Serializer only supports std::tuple and std::array containers and scalar elements.");
        printf("non scalar;\n");
    }

    template<typename ... Ts, size_t ... Is>
    static void Write(const std::tuple<Ts...>& structure, std::ostream& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{ 1,
                       (Write(std::get<Is>(structure), output), void(), int{})...
        };
    }

    template<typename ... Ts>
    static void Write(const std::tuple<Ts...>& structure, std::ostream& output) {
        Write(structure, output, std::make_index_sequence<sizeof...(Ts)>{});
    }
};

template<support::Endian Endian, typename ...Ts>
void SerializeTuple(const std::tuple<Ts...>& structure, std::ostream& output) {
    ElementWriter<Endian>::Write(structure, output);
}

}