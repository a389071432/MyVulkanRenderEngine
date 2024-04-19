#pragma once
#include<assert.h>
#include <iostream>
#include <cassert>

#define ASSERT(condition, message)\
        if(!(condition)){ \
           std::cerr << "Assertion failed: " << message << std::endl;\
           std::abort();\
        }