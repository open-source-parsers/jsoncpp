// Copyright 2015 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at https://raw.githubusercontent.com/open-source-parsers/jsoncpp/master/LICENSE
#ifndef CPPTL_JSON_ALLOCATOR_H_INCLUDED
#define CPPTL_JSON_ALLOCATOR_H_INCLUDED

#include <cstring> // std::memset
#include <cstddef> // std::size_t, std::ptrdiff_t
#include <utility> // std::forward

#if !defined(JSON_IS_AMALGAMATION)
#include "config.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

/**
 * This class is used for de-allocating memory securely, allocation happens
 * in the normal manner however it's important that de-allocation explicitly
 * overwrites the memory with 0s to ensure the memory cannot be "sniffed"
 */
template<typename T>
class JSON_API SecureAllocator {
  public:
    // Type definitions
    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    /**
     * Release memory which was allocated for N items at pointer P.
     *
     * The memory block is filled with zeroes before being released.
     * The pointer argument is tagged as "volatile" to prevent the
     * compiler optimizing out this critical step.
     */
    void deallocate(volatile pointer p, size_type n) {
      std::fill_n((volatile char*)p, n * sizeof(value_type), 0);
      std::allocator<T>::deallocate(p, n);
    }
};

template<typename T, typename U>
bool operator==(const SecureAllocator<T>&, const SecureAllocator<U>&) {
  return true;
}

template<typename T, typename U>
bool operator!=(const SecureAllocator<T>&, const SecureAllocator<U>&) {
  return false;
}

} //end namespace Json

#endif // CPPTL_JSON_ALLOCATOR_H_INCLUDED
