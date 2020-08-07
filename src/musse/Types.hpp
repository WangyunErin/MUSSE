#ifndef TYPES
#define TYPES

#include <array>
#include <vector>
#include <iostream>

// The main type for passing around raw file data
#define ID_SIZE 16

using byte_t = uint8_t;//one-byte char
using block = std::vector<byte_t>;//the the type of elements is one-byte char

template <size_t N>// the capacity of template is N
using bytes = std::array<byte_t, N>;//type is one-byte char, number is N

// A bucket contains a number of Blocks
constexpr int Z = 4;

enum Op {
    READ,
    WRITE
};    //define enumarate type Op;

template< typename T >
std::array< byte_t, sizeof (T) > to_bytes(const T& object) {
    std::array< byte_t, sizeof (T) > bytes;

    const byte_t* begin = reinterpret_cast<const byte_t*> (std::addressof(object));//reinterpret_cast standard transformation operator，used to transform any irrelevant types
    const byte_t* end = begin + sizeof (T);
    std::copy(begin, end, std::begin(bytes));//copy all elements of object to bytes

    return bytes;
}  //define a template function to_bytes to copy all elements of object to bytes

template< typename T >
T& from_bytes(const std::array< byte_t, sizeof (T) >& bytes, T& object) {
    byte_t* begin_object = reinterpret_cast<byte_t*> (std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);

    return object;
}  //define a template function from_bytes to copy all elements of bytes to object

#endif