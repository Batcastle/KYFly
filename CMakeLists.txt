cmake_minimum_required(VERSION 3.10)
project(kyfly VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Clickable sets CLICK_MODE=ON (see clickable.yaml). Desktop builds leave it OFF.
option(CLICK_MODE "Install into a click package layout" OFF)
option(KYFLY_BUILD_TOOLS "Build the desktop probe tool and unit tests" OFF)

# Ubuntu Touch 24.04 ships Qt 5.15.
find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Network DBus Qml Quick)

set(PROJECT_NAME "kyfly")
set(FULL_PROJECT_NAME "kyfly.batcastle")
set(DESKTOP_FILE_NAME ${PROJECT_NAME}.desktop)

# ---- core library: protocol, video, input (no QML) ----
add_library(kyfly_core STATIC
    src/rtpjpeg.cpp src/rtpjpeg.h src/jpeg_tables.inc
    src/framedecoder.cpp src/framedecoder.h
    src/rtspworker.cpp src/rtspworker.h
    src/videoreceiver.cpp src/videoreceiver.h
    src/flightinput.cpp src/flightinput.h
    src/dronelink.cpp src/dronelink.h
    src/jpegutil.cpp src/jpegutil.h
    src/mkvwriter.cpp src/mkvwriter.h
    src/mp4writer.cpp src/mp4writer.h
    src/recorder.cpp src/recorder.h
    src/mediacapture.cpp src/mediacapture.h
)
target_include_directories(kyfly_core PUBLIC src)
target_link_libraries(kyfly_core PUBLIC Qt5::Core Qt5::Gui Qt5::Network)
target_compile_definitions(kyfly_core PUBLIC KYFLY_VERSION="${PROJECT_VERSION}")

# ---- the app ----
add_executable(${PROJECT_NAME} src/main.cpp src/videoitem.cpp src/videoitem.h
               src/screenkeeper.cpp src/screenkeeper.h qml/qml.qrc)
target_link_libraries(${PROJECT_NAME} kyfly_core Qt5::DBus Qt5::Qml Qt5::Quick)

if(CLICK_MODE)
    set(CMAKE_INSTALL_PREFIX /)
    set(DATA_DIR /)

    # Clickable provides framework, AppArmor policy and architecture through the
    # environment. If a value isn't there, leave Clickable's placeholder in the
    # file so Clickable (>= 8.4) fills it in when packaging.
    if(DEFINED ENV{CLICK_FRAMEWORK})
        set(CLICK_FRAMEWORK "$ENV{CLICK_FRAMEWORK}")
    elseif(DEFINED ENV{SDK_FRAMEWORK})
        set(CLICK_FRAMEWORK "$ENV{SDK_FRAMEWORK}")
    else()
        set(CLICK_FRAMEWORK "@CLICK_FRAMEWORK@")
    endif()
    # Must end up as a bare JSON number (click-review rejects a string).
    if(DEFINED ENV{APPARMOR_POLICY})
        set(APPARMOR_POLICY "$ENV{APPARMOR_POLICY}")
    else()
        set(APPARMOR_POLICY "2404.1")   # matches the framework in clickable.yaml
    endif()
    if(DEFINED ENV{ARCH})
        set(CLICK_ARCH "$ENV{ARCH}")
    else()
        execute_process(COMMAND dpkg-architecture -qDEB_HOST_ARCH
                        OUTPUT_VARIABLE CLICK_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    configure_file(manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/manifest.json @ONLY)
    configure_file(${PROJECT_NAME}.apparmor.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.apparmor @ONLY)
    configure_file(${DESKTOP_FILE_NAME}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME} @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME} DESTINATION ${DATA_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.apparmor DESTINATION ${DATA_DIR})
    install(DIRECTORY assets DESTINATION ${DATA_DIR})
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

    # WEBP photos need Qt's webp image plugin. Ubuntu Touch may not ship it, so
    # bundle it (plus libwebp) when it's present in the build environment
    # (clickable.yaml installs qt5-image-formats-plugins there). Best effort:
    # without it the app simply hides the WEBP option.
    option(KYFLY_BUNDLE_WEBP "Bundle Qt's WebP image plugin in the click" ON)
    if(KYFLY_BUNDLE_WEBP)
        get_target_property(_qmake Qt5::qmake IMPORTED_LOCATION)
        execute_process(COMMAND ${_qmake} -query QT_INSTALL_PLUGINS
                        OUTPUT_VARIABLE _qt_plugins OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(_qwebp "${_qt_plugins}/imageformats/libqwebp.so")
        if(EXISTS "${_qwebp}")
            execute_process(COMMAND ${_qmake} -query QT_INSTALL_LIBS
                            OUTPUT_VARIABLE _qt_libs OUTPUT_STRIP_TRAILING_WHITESPACE)
            get_filename_component(_triplet "${_qt_libs}" NAME)
            install(FILES "${_qwebp}" DESTINATION plugins/imageformats)
            foreach(_soname libsharpyuv.so.0 libwebp.so.7 libwebpdemux.so.2 libwebpmux.so.3)
                if(EXISTS "${_qt_libs}/${_soname}")
                    get_filename_component(_real "${_qt_libs}/${_soname}" REALPATH)
                    install(FILES "${_real}" DESTINATION lib/${_triplet} RENAME ${_soname})
                endif()
            endforeach()
            message(STATUS "Bundling WebP image plugin from ${_qwebp}")
        else()
            message(STATUS "WebP image plugin not found; WEBP photos depend on the system")
        endif()
    endif()
else()
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# ---- desktop tools & tests ----
if(KYFLY_BUILD_TOOLS)
    add_executable(kyfly-probe tools/probe.cpp)
    target_link_libraries(kyfly-probe kyfly_core)

    enable_testing()
    find_package(Python3 COMPONENTS Interpreter)
    add_executable(depay_test tests/depay_test.cpp)
    target_link_libraries(depay_test kyfly_core)
    add_executable(packet_test tests/packet_test.cpp)
    target_link_libraries(packet_test kyfly_core)
    add_executable(capture_test tests/capture_test.cpp)
    target_link_libraries(capture_test kyfly_core)
    if(Python3_FOUND)
        add_test(NAME photo_and_video_capture
                 COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/capture_cases.py $<TARGET_FILE:capture_test>)
        add_test(NAME rtp_jpeg_depayloader
                 COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/depay_cases.py $<TARGET_FILE:depay_test>)
        add_test(NAME control_packets_match_java
                 COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/packet_cases.py $<TARGET_FILE:packet_test>)
    endif()
endif()
