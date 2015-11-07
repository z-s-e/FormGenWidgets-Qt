cmake_minimum_required(VERSION 3.1)
project(FormGenWidgets-Qt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Widgets NO_MODULE REQUIRED)

set(FORMGENWIDGETS_QT_VERSION_MAJOR 0)
set(FORMGENWIDGETS_QT_VERSION_MINOR 1)
set(FORMGENWIDGETS_QT_VERSION_PATCH 0)
set(FORMGENWIDGETS_QT_VERSION "${FORMGENWIDGETS_QT_VERSION_MAJOR}.${FORMGENWIDGETS_QT_VERSION_MINOR}.${FORMGENWIDGETS_QT_VERSION_PATCH}")
set(FORMGENWIDGETS_QT_SOVERSION 1)


set(formgenwidgets_src
    lib/sorted_sequence/sorted_sequence.h
    lib/MathUtils/mathutils.cpp
    src/formgencompositionmodels.cpp
    src/formgencompositionwidgets.cpp
    src/formgencompositionwidgets_p.h
    src/formgenregularwidgets.cpp
    src/formgenregularwidgets_p.cpp
    src/formgenwidgetsbase.cpp
    src/formgenwidgets-qt.h
    src/formgenfilelisthead.ui
    src/formgenlistbaghead.ui)


option(
  FORMGENWIDGETS_QT_SHARED_LIB
  "Enable to build a shared lib"
  OFF
)

if(FORMGENWIDGETS_QT_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${formgenwidgets_src})
    set(FORMGENWIDGETS_STATIC 0)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DFORMGENWIDGETS_LIBRARY)
else()
    add_library(${PROJECT_NAME} STATIC ${formgenwidgets_src})
    set(FORMGENWIDGETS_STATIC 1)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC -DSORTEDSEQUENCE_ENABLE_QHASH_WRAPPER -DSORTEDSEQUENCE_ENABLE_STD_HASH_WRAPPER)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/formgenwidgets_global.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/formgenwidgets_global.h
)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_include_directories(${PROJECT_NAME} PRIVATE lib/MathUtils)
set_property(TARGET ${PROJECT_NAME}
             PROPERTY PUBLIC_HEADER
             lib/sorted_sequence/sorted_sequence.h
             src/formgencompositionwidgets.h
             src/formgencompositionmodels.h
             src/formgenregularwidgets.h
             src/formgenwidgetsbase.h
             src/formgenwidgets-qt.h
             ${CMAKE_CURRENT_BINARY_DIR}/include/formgenwidgets_global.h)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      SOVERSION ${FORMGENWIDGETS_QT_SOVERSION}
                      VERSION ${FORMGENWIDGETS_QT_VERSION})
set(CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

set(LIB_INSTALL_DIR lib)
set(LIB_STATIC_INSTALL_DIR lib/static)
set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_STATIC_INSTALL_DIR}
        PUBLIC_HEADER DESTINATION ${INCLUDE_INSTALL_DIR}
        COMPONENT Devel)

target_include_directories(${PROJECT_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/sorted_sequence>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                           $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>)

set(CMAKECONFIG_INSTALL_DIR lib/cmake/${PROJECT_NAME})
include(CMakePackageConfigHelpers)
configure_package_config_file(FormGenWidgets-QtConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/FormGenWidgets-QtConfig.cmake
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR} )
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/FormGenWidgets-QtConfigVersion.cmake
                                 VERSION ${FORMGENWIDGETS_QT_VERSION}
                                 COMPATIBILITY SameMajorVersion )
install(EXPORT ${PROJECT_NAME} FILE FormGenWidgets-QtTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR} )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FormGenWidgets-QtConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/FormGenWidgets-QtConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR} )


option(
  FORMGENWIDGETS_QT_BUILD_TESTAPP
  "Build small test app"
  OFF
)

if(FORMGENWIDGETS_QT_BUILD_TESTAPP)
    add_executable(FormGenWidgets-Test test/manual/main.cpp)
    set(CXX_STANDARD_REQUIRED ON)
    set_property(TARGET FormGenWidgets-Test PROPERTY CXX_STANDARD 11)
    target_link_libraries(FormGenWidgets-Test FormGenWidgets-Qt Qt5::Widgets)
endif()
