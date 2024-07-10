#pragma once
#include <iostream>
//#include <cstdlib>

#define ASSERT(condition, message)\
        if(!(condition)){ \
           std::cerr << "Assertion failed: " << message << std::endl;\
           std::abort();\
        }

//#define ASSERT(condition, message,value)\
//        if(!(condition)){ \
//           std::cerr << "Assertion failed: " << message <<": "<<value<< std::endl;\
//           std::abort();\
//        }

//// TODO: study this (variadic macro)
//#define ASSERT_IMPL_2(condition, message) \
//    do { \
//        if (!(condition)) { \
//            std::cerr << "Assertion failed: " << message << std::endl; \
//            std::abort(); \
//        } \
//    } while (false)
//
//#define ASSERT_IMPL_3(condition, message, value) \
//    do { \
//        if (!(condition)) { \
//            std::cerr << "Assertion failed: " << message << ": " << value << std::endl; \
//            std::abort(); \
//        } \
//    } while (false)
//
//#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
//#define ASSERT(...) GET_MACRO(__VA_ARGS__, ASSERT_IMPL_3, ASSERT_IMPL_2)(__VA_ARGS__)