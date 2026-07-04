#ifndef CM_TYPES_HH_
#define CM_TYPES_HH_

// #include <map>
// #include <deque>
// #include <fstream>
#include <vector>
#include <string>
// #include "cm_thread.hh"

// #ifdef DEBUG_ALLOCATIONS

// Jen pro C++11

// extern CMSMutex allocator_mutex;
// extern std::map<void *, size_t> allocations;
// extern std::map<char, size_t> allocated_total;
// extern std::ofstream allocation_debug_output;

// void print_allocation_message();

// template <char Name>
// struct named_allocator
// {
//     template <typename T>
//     class allocator
//     {
//     public:
//         using value_type = T;

//         allocator() noexcept {}

//         template <class U>
//         allocator(allocator<U> const &) noexcept
//         {}

//         value_type* allocate(std::size_t n)
//         {
//             allocator_mutex.Lock();
//             size_t size = n*sizeof(value_type);
//             allocated_total[Name] += size;
//             void *mem = ::operator new(size);
//             allocations.insert(std::make_pair(mem, size));
//             if (allocations.size() % 1000 == 0)
//                 print_allocation_message();
//             allocator_mutex.Unlock();
//             return static_cast<value_type*>(mem);
//         }

//         void deallocate(value_type* p, std::size_t) noexcept
//         {
//             allocator_mutex.Lock();
//             size_t size = allocations.at(p);
//             allocations.erase(p);
//             if (allocations.size() % 1000 == 0)
//                 print_allocation_message();
//             allocated_total[Name] -= size;
//             ::operator delete(p);
//             allocator_mutex.Unlock();
//         }

//         template <typename U>
//         bool operator==(allocator<U> const &)
//         {
//             return true;
//         }

//         template <typename U>
//         bool operator!=(allocator<U> const &)
//         {
//             return false;
//         }
//     };
// };

// typedef std::vector<std::string, named_allocator<'s'>::allocator<std::string>> StringList;
// typedef std::vector<double, named_allocator<'d'>::allocator<double>> DoubleList;
// typedef std::vector<uint64_t, named_allocator<'i'>::allocator<uint64_t>> UIntList;

// #else // DEBUG ALLOCATIONS

// class libcm_integer
// {
//     int64_t value;

// public:
//     template <typename T>
//     libcm_integer(T const &v)
// 	: value(v)
//     {
// 	static_assert(std::is_same<T, int64_t>::value ||
// 		      std::is_same<T, int>::value ||
// 		      std::is_same<T, int16_t>::value ||
// 		      std::is_same<T, int8_t>::value
// 		      , "bad integer");
//     }

//     libcm_integer() : value(0) {}

//     friend bool operator==(libcm_integer a, libcm_integer b) { return a.value == b.value; }
//     friend bool  operator<(libcm_integer a, libcm_integer b) { return a.value  < b.value; }
//     friend bool  operator>(libcm_integer a, libcm_integer b) { return a.value  > b.value; }
//     friend bool operator<=(libcm_integer a, libcm_integer b) { return a.value <= b.value; }
//     friend bool operator>=(libcm_integer a, libcm_integer b) { return a.value >= b.value; }
//     friend bool operator!=(libcm_integer a, libcm_integer b) { return a.value != b.value; }

//     friend bool operator==(libcm_integer a, int64_t value) { return a.value == value; }
//     friend bool  operator<(libcm_integer a, int64_t value) { return a.value  < value; }
//     friend bool  operator>(libcm_integer a, int64_t value) { return a.value  > value; }
//     friend bool operator<=(libcm_integer a, int64_t value) { return a.value <= value; }
//     friend bool operator>=(libcm_integer a, int64_t value) { return a.value >= value; }
//     friend bool operator!=(libcm_integer a, int64_t value) { return a.value != value; }

//     friend std::ostream &operator<<(std::ostream &o, libcm_integer a)
//     { return o << a.value; }
    
//     operator int64_t() const { return value; }
// }
// ;

// #define CM_GET(x) x.get()
// #define CM_INT(x) libcm_integer(libcm_integer::private_type(), x)

#if __cplusplus < 201103L
#define __STDC_FORMAT_MACROS 1
#define __STDC_LIMIT_MACROS 1
#endif

#include <stdint.h>
#include <inttypes.h>

typedef int64_t libcm_integer;

typedef std::vector<std::string> StringList;
typedef std::vector<double> DoubleList;
typedef std::vector<libcm_integer> UIntList;

// #endif // DEBUG ALLOCATIONS


#endif // CM_TYPES_HH_
