#pragma once

#include "Endian.h"
#include "SerializableStruct.h"

#include <tuple>
#include <string>
#include <vector>
#include <iostream>

#define LOG_STEP(f_) (std::cout << (f_) << std::endl)

namespace serializer_internal {

template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename> struct IsSerializableStruct : std::false_type {};
template <typename ...T> struct IsSerializableStruct<std::tuple<T...>> : std::true_type {};

template <typename> struct IsSerializableArray : std::false_type {};
template <typename T, size_t I> struct IsSerializableArray<std::array<T, I>> : std::true_type {};
template <typename T> struct IsSerializableArray<std::vector<T>> : std::true_type {};

template <typename T>
using SerializableArrayElement = decltype(std::declval<T>()[0]);

template<support::Endian Endian, typename Visitor, bool ShouldSwap = Endian != support::Endian::ignore && Endian != support::HostEndian>
struct StructVisitor {
    // TODO: add specializations for single byte scalars
    template<typename T, typename Stream>
    static void VisitElement(T&& field, Stream&& output) {
        if constexpr (IsSerializableArray<remove_cvref_t<T>>::value) {
            LOG_STEP("Detected array.");
            if constexpr (std::is_same_v<remove_cvref_t<SerializableArrayElement<T>>, char>) {
                // Special treatment of char arrays: visit as element
                LOG_STEP("VisitElement: detected special case char array.");
                Visitor::template VisitElement<ShouldSwap>(std::forward<T>(field), std::forward<Stream>(output));
            } else {
                // Array of some other type.
                LOG_STEP("VisitElement: detected array. unpacking...");
                VisitArray(std::forward<T>(field), std::forward<Stream>(output));
            }
        } else if constexpr (IsSerializableStruct<remove_cvref_t<T>>::value) {
            LOG_STEP("VisitElement: detected tuple. unpacking...");
            VisitTuple(std::forward<T>(field), std::forward<Stream>(output));
        } else {
            // We don't special case this type. Allow the Visitor to handle it.
            LOG_STEP("VisitElement: detected unknown element type. forwarding to Visitor...");
            Visitor::template VisitElement<ShouldSwap>(std::forward<T>(field), std::forward<Stream>(output));
        }
    }

    template<typename T, typename Stream>
    static void VisitArray(T&& field, Stream&& output) {
        LOG_STEP("Begin array.");
        for (auto&& element : field) {
            VisitElement(std::forward<decltype(element)>(element), std::forward<Stream>(output));
        }
        LOG_STEP("End array.");
    }

    template<typename Tuple, typename Stream, size_t ... Is>
    static void VisitTuple(Tuple&& structure, Stream&& output, std::index_sequence<Is...>) {
        using swallow = int[];
        (void)swallow{(VisitElement(std::get<Is>(std::forward<Tuple>(structure)), std::forward<Stream>(output)), int{})...};
    }

    template<typename Tuple, typename Stream>
    static void VisitTuple(Tuple&& structure, Stream&& output) {
        LOG_STEP("Begin tuple.");
        VisitTuple(std::forward<Tuple>(structure), std::forward<Stream>(output), std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
        LOG_STEP("End tuple.");
    }
};

struct StructElementWriter {
    template <bool ShouldSwap, typename T, typename = typename std::enable_if<std::is_scalar_v<T>>::type>
    static void VisitElement(const T& field, std::ostream& stream) {
        if constexpr (ShouldSwap) {
            LOG_STEP("Writing swapped scalar type.");
            T copy = field;
            support::EndianSwap(&copy);
            stream.write(reinterpret_cast<const char *>(&copy), sizeof(T));
        } else {
            LOG_STEP("Writing non-swapped scalar type.");
            stream.write(reinterpret_cast<const char *>(&field), sizeof(T));
        }
    }

    template<bool, size_t I>
    static void VisitElement(const std::array<char, I>& field, std::ostream& stream) {
        LOG_STEP("Writing char array.");
        stream.write(field.data(), field.size());
    }

    template<bool>
    static void VisitElement(const std::vector<char>& field, std::ostream& stream) {
        LOG_STEP("Writing char vector.");
        stream.write(field.data(), field.size());
    }

    template <bool>
    static void VisitElement(const std::string& field, std::ostream& stream) {
        LOG_STEP("Writing string as null-terminated c string.");
        stream.write(field.c_str(), field.size() + 1);
    }
};

struct StructElementReader {
    template <bool ShouldSwap, typename T, typename = typename std::enable_if<std::is_scalar_v<T>>::type>
    static void VisitElement(T& field, std::istream& stream) {
        if constexpr (ShouldSwap) {
            LOG_STEP("Reading swapped scalar type.");
            stream.read(reinterpret_cast<char*>(&field), sizeof(T));
            support::EndianSwap(&field);
        } else {
            LOG_STEP("Reading non-swapped scalar type.");
            stream.read(reinterpret_cast<char *>(&field), sizeof(T));
        }
    }

    template<bool, size_t I>
    static void VisitElement(std::array<char, I>& field, std::istream& stream) {
        LOG_STEP("Reading char array.");
        stream.read(field.data(), field.size());
    }

    template <bool>
    static void VisitElement(std::string& field, std::istream& stream) {
        LOG_STEP("Reading null-terminated c string as string.");
        std::getline(stream, field, '\0');
    }
};

}

namespace serializer {

template<support::Endian Endian, typename T, typename OStream>
void Serialize(const T& in, OStream&& out) {
    using namespace serializer_internal;
    LOG_STEP("Serialize begin.");
    StructVisitor<Endian, StructElementWriter>::VisitElement(in, std::forward<OStream>(out));
    LOG_STEP("Serialize end.");
}

template <typename T, typename OStream>
static void Serialize(const T& in, OStream&& out, support::Endian endianness) {
    switch (endianness) {
        case support::Endian::big:
            Serialize<support::Endian::big>(in, std::forward<OStream>(out));
            break;
        case support::Endian::little:
            Serialize<support::Endian::little>(in, std::forward<OStream>(out));
            break;
        case support::Endian::ignore:
            Serialize<support::Endian::ignore>(in, std::forward<OStream>(out));
            break;
    }
}

template<support::Endian Endian, typename T, typename IStream>
void Deserialize(T& out, IStream&& in) {
    using namespace serializer_internal;
    LOG_STEP("Deserialize begin.");
    StructVisitor<Endian, StructElementReader>::VisitElement(out, std::forward<IStream>(in));
    LOG_STEP("Deserialize end.");
}

template <typename T, typename IStream>
static void Deserialize(T& out, IStream&& in, support::Endian endianness) {
    switch (endianness) {
        case support::Endian::big:
            Deserialize<support::Endian::big>(out, std::forward<IStream>(in));
            break;
        case support::Endian::little:
            Deserialize<support::Endian::little>(out, std::forward<IStream>(in));
            break;
        case support::Endian::ignore:
            Deserialize<support::Endian::ignore>(out, std::forward<IStream>(in));
            break;
    }
}

}