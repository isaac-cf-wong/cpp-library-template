# Sanitizer support, driven by CPP_LIBRARY_TEMPLATE_SANITIZERS.
#
# Sanitizers must instrument every translation unit in the process, so the flags
# are applied directory-wide rather than through a link-time interface target.
# Use the "asan-ubsan" or "tsan" presets in CMakePresets.json.

if(NOT CPP_LIBRARY_TEMPLATE_SANITIZERS)
    return()
endif()

if(MSVC)
    if(NOT "address" IN_LIST CPP_LIBRARY_TEMPLATE_SANITIZERS)
        message(FATAL_ERROR "MSVC only supports the 'address' sanitizer, got '${CPP_LIBRARY_TEMPLATE_SANITIZERS}'.")
    endif()
    add_compile_options(/fsanitize=address /Zi)
    return()
endif()

# thread is mutually exclusive with address and memory.
if("thread" IN_LIST CPP_LIBRARY_TEMPLATE_SANITIZERS)
    foreach(_conflict address memory leak)
        if("${_conflict}" IN_LIST CPP_LIBRARY_TEMPLATE_SANITIZERS)
            message(FATAL_ERROR "ThreadSanitizer cannot be combined with '${_conflict}'.")
        endif()
    endforeach()
endif()

list(JOIN CPP_LIBRARY_TEMPLATE_SANITIZERS "," _sanitizer_list)
message(STATUS "cpp_library_template: enabling sanitizers -fsanitize=${_sanitizer_list}")

add_compile_options(-fsanitize=${_sanitizer_list} -fno-omit-frame-pointer -fno-sanitize-recover=all -g)
add_link_options(-fsanitize=${_sanitizer_list})
