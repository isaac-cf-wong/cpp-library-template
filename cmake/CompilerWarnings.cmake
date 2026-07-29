# A single interface target carrying the project warning set.
#
# Link it PRIVATE from every target we own. It is deliberately not part of the
# installed interface -- downstream consumers should not inherit our warnings.

add_library(cpp_library_template_warnings INTERFACE)

set(_msvc_warnings
    /W4 # baseline
    /permissive- # standards conformance
    /w14242 # possible loss of data on implicit conversion
    /w14254 # larger bit field assigned to smaller
    /w14263 # member function does not override anything
    /w14265 # class has virtual functions but non-virtual destructor
    /w14287 # unsigned/negative constant mismatch
    /we4289 # loop control variable used outside the loop
    /w14296 # expression is always true/false
    /w14311 # pointer truncation
    /w14545 # expression before comma has no effect
    /w14546 # expression before comma has no effect
    /w14547 # operator before comma has no effect
    /w14549 # operator before comma has no effect
    /w14555 # expression has no effect
    /w14619 # no warning number exists
    /w14640 # thread-unsafe static member initialisation
    /w14826 # sign-inconsistent conversion
    /w14905 # wide string literal cast to LPSTR
    /w14906 # string literal cast to LPWSTR
    /w14928 # illegal copy-initialisation
)

set(_clang_warnings
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
)

set(_gcc_warnings
    ${_clang_warnings}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
    -Wsuggest-override
)

if(MSVC)
    set(_warnings ${_msvc_warnings})
    set(_werror /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(_warnings ${_clang_warnings})
    set(_werror -Werror)
else()
    set(_warnings ${_gcc_warnings})
    set(_werror -Werror)
endif()

if(CPP_LIBRARY_TEMPLATE_WARNINGS_AS_ERRORS)
    list(APPEND _warnings ${_werror})
endif()

target_compile_options(cpp_library_template_warnings INTERFACE ${_warnings})
