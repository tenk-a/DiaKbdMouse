cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0136)
    cmake_policy(SET CMP0136 NEW)
endif()

project(DiaKbdMouse LANGUAGES CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "DiaKbdMouse is a Windows application.")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DIA_PLATFORM_NAME x64)
    set(DIA_PACKAGE_PLATFORM win64)
else()
    set(DIA_PLATFORM_NAME Win32)
    set(DIA_PACKAGE_PLATFORM win32)
endif()

set(DIA_PACKAGE_SUFFIX "" CACHE STRING "Optional package platform suffix, for example -xp.")
set(DIA_PACKAGE_FLAVOR "" CACHE STRING "Package platform/flavor suffix override.")
if(NOT DIA_PACKAGE_FLAVOR)
    set(DIA_PACKAGE_FLAVOR "${DIA_PACKAGE_PLATFORM}${DIA_PACKAGE_SUFFIX}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/exe_${DIA_PLATFORM_NAME}_$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/exe_${DIA_PLATFORM_NAME}_$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib_${DIA_PLATFORM_NAME}_$<CONFIG>")

if(MSVC)
    set(DIA_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "${DIA_MSVC_RUNTIME_LIBRARY}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "DiaKbdMouse")
    set(DIA_WINDOWS_SUBSYSTEM_VERSION "" CACHE STRING "Optional Windows subsystem version, for example 5.01 for Windows XP.")
    if(DIA_WINDOWS_SUBSYSTEM_VERSION)
        set(DIA_WINDOWS_SUBSYSTEM_OPTION "/SUBSYSTEM:WINDOWS,${DIA_WINDOWS_SUBSYSTEM_VERSION}")
    else()
        set(DIA_WINDOWS_SUBSYSTEM_OPTION "/SUBSYSTEM:WINDOWS")
    endif()
endif()

if(WATCOM)
    set(CMAKE_WATCOM_RUNTIME_LIBRARY "MultiThreaded")
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE
            CMAKE_SHARED_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS_DEBUG CMAKE_SHARED_LINKER_FLAGS_RELEASE)
        if(DEFINED ${flag_var})
            string(REGEX REPLACE "(^| )-br([ =]|$)" "\\1" ${flag_var} "${${flag_var}}")
            string(REGEX REPLACE "(^| )/br([ =]|$)" "\\1" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

add_library(DiaKbdMouseHook SHARED
    src/dll/DiaKbdMouseHook.cpp
    src/dll/DiaKbdMouseHook_ConvKey.cpp
    src/dll/DiaKbdMouseHook_Impl.cpp
    src/dll/stdafx.cpp
    src/dll/DiaKbdMouseHook.h
    src/dll/DiaKbdMouseHook_Impl.h
    src/dll/stdafx.h
)

set(DIA_RESOURCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/main")
set(DIA_CANONICAL_RESOURCES
    "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouse.rc"
    "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouseManifest.rc"
    "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouseWatcom.rc"
)

if(WATCOM)
    set(DIA_MAIN_RESOURCES)
else()
    set(DIA_MAIN_RESOURCES
        "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouse.rc"
        "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouseManifest.rc"
    )
endif()

target_compile_definitions(DiaKbdMouseHook PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    HOOKDLL_EXPORTS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

target_include_directories(DiaKbdMouseHook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dll
)

add_executable(DiaKbdMouse WIN32
    src/cmn/misc.cpp
    src/cmn/misc.h
    src/main/ConfigFileReader.cpp
    src/main/DiaKbdMouse.cpp
    src/main/KbdMouseCtrl.cpp
    src/main/stdafx.cpp
    src/main/TrayIcon.cpp
    ${DIA_MAIN_RESOURCES}
    src/main/ConfigFileReader.h
    src/main/DiaKbdMouse.h
    src/main/KbdMouseCtrl.h
    src/main/resource.h
    src/main/stdafx.h
    src/main/TrayIcon.h
)

target_compile_definitions(DiaKbdMouse PRIVATE
    WIN32
    _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

target_include_directories(DiaKbdMouse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dll
)

target_link_libraries(DiaKbdMouse PRIVATE
    DiaKbdMouseHook
    WinMM
)

add_dependencies(DiaKbdMouse DiaKbdMouseHook)

if(MSVC)
    foreach(target DiaKbdMouse DiaKbdMouseHook)
        set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "${DIA_MSVC_RUNTIME_LIBRARY}"
        )
        target_compile_options(${target} PRIVATE
            /W4
            /wd4996
            /utf-8
            $<$<NOT:$<CONFIG:Debug>>:/Gy>
            $<$<NOT:$<CONFIG:Debug>>:/Gw>
        )
        target_link_options(${target} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:/OPT:REF>
            $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>
        )
    endforeach()

    target_compile_options(DiaKbdMouse PRIVATE /wd4201)

    target_link_options(DiaKbdMouse PRIVATE
        ${DIA_WINDOWS_SUBSYSTEM_OPTION}
        /MANIFEST:NO
    )

    target_link_options(DiaKbdMouseHook PRIVATE
        ${DIA_WINDOWS_SUBSYSTEM_OPTION}
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:DllMain>
        /NODEFAULTLIB:libc.lib
        /MERGE:.rdata=.text
        /SECTION:DIACURSO,RWS
    )
elseif(MINGW)
    foreach(target DiaKbdMouse DiaKbdMouseHook)
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endforeach()
elseif(WATCOM)
    foreach(target DiaKbdMouse DiaKbdMouseHook)
        set_property(TARGET ${target} PROPERTY WATCOM_RUNTIME_LIBRARY MultiThreaded)
    endforeach()
    find_program(DIA_WRC_EXECUTABLE wrc REQUIRED)
    set(DIA_WATCOM_RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/DiaKbdMouseWatcom.res")
    set_property(TARGET DiaKbdMouse APPEND PROPERTY LINK_DEPENDS
        ${DIA_CANONICAL_RESOURCES}
        "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouse.manifest"
        "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouse.ico"
        "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouseSml.ico"
    )
    add_custom_command(TARGET DiaKbdMouse POST_BUILD
        COMMAND "${DIA_WRC_EXECUTABLE}"
            -q
            -bt=nt
            "-i=${DIA_RESOURCE_SOURCE_DIR}"
            "-fo=${DIA_WATCOM_RESOURCE_FILE}"
            "${DIA_RESOURCE_SOURCE_DIR}/DiaKbdMouseWatcom.rc"
            "$<TARGET_FILE:DiaKbdMouse>"
        VERBATIM
        COMMENT "Attaching Watcom resources to DiaKbdMouse.exe"
    )
    message(STATUS "Watcom C/C++ generator selected. Building with the static multithreaded CRT; DLL CRT flags (-br) are removed.")
endif()

install(TARGETS DiaKbdMouse DiaKbdMouseHook
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
)

install(FILES
    DiaKbdMouse.cfg
    DiaKbdMouse.htm
    README.md
    regist_startup.bat
    DESTINATION .
)

install(DIRECTORY config_etc/
    DESTINATION config_etc
    FILES_MATCHING PATTERN "*.reg"
)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME DiaKbdMouse)
set(CPACK_PACKAGE_VENDOR "tenk-a")
set(CPACK_PACKAGE_VERSION "0.91")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${DIA_PACKAGE_FLAVOR}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)

include(CPack)
