find_package(Git)
if (NOT GIT_FOUND)
    message(WARNING "Could not find git executable, build will not include git information!")
endif()

function(runGit)
    set(options "")
    set(singleValueArgs "OUTPUT_VARIABLE")
    set(multiValueArgs "COMMAND")
    cmake_parse_arguments(ARG "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    if ("${ARG_OUTPUT_VARIABLE}" STREQUAL "")
        message(FATAL_ERROR "OUTPUT_VARIABLE is required")
    endif()
    if ("${ARG_COMMAND}" STREQUAL "")
        message(FATAL_ERROR "COMMAND is required")
    endif()

    if (NOT GIT_FOUND)
        set(${ARG_OUTPUT_VARIABLE} "" PARENT_SCOPE)
        return()
    endif()

    execute_process(
            COMMAND ${GIT_EXECUTABLE} ${ARG_COMMAND}
            OUTPUT_VARIABLE LOCAL_OUTPUT_VARIABLE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${ARG_OUTPUT_VARIABLE} "${LOCAL_OUTPUT_VARIABLE}" PARENT_SCOPE)
endfunction()
