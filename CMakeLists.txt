cmake_minimum_required(VERSION 3.5)

project(Scratch CXX)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.59 REQUIRED system regex date_time)

set(scratch_obj_sources
    src/byte_buffer_view.cpp
)

set(scratch_sources
    main.cpp
)

add_library(ScratchIncludes INTERFACE)

target_include_directories(ScratchIncludes
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(ScratchIncludes
    INTERFACE
        cxx_std_17
)

add_library(ScratchObjects
    OBJECT
        ${scratch_obj_sources}
)

target_include_directories(ScratchObjects
    PUBLIC
        $<TARGET_PROPERTY:ScratchIncludes,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_options(ScratchObjects
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4 /WX /Zi /Od
            /Zc:strictStrings
            /Zc:referenceBinding-
            /Zc:auto,forScope,rvalueCast
            /Zc:throwingNew,wchar_t
        >
)

add_executable(Scratch 
    ${scratch_sources}
    $<TARGET_OBJECTS:ScratchObjects>
)

target_link_libraries(Scratch
    PRIVATE
        ScratchIncludes
        Boost::boost
        Boost::date_time
)
