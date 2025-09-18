cmake_minimum_required(VERSION 3.16)

project(PrettyDopeFileviewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------
# Qt
# -------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Pdf PdfWidgets)

# -------------------
# Poppler via pkg-config
# -------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_QT6 REQUIRED poppler-qt6)

# -------------------
# Fuentes del proyecto
# -------------------
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    pdfdocument.cpp
    pdfdocument.h
    pdfpage.cpp
    pdfpage.h
    pagemanager.cpp
    pagemanager.h
    navigationcontroller.cpp
    navigationcontroller.h
    zoomcontroller.cpp
    zoomcontroller.h
    pdfviewer.cpp
    pdfviewer.h
    mainwindow.ui
    resources.qrc
)

# -------------------
# Ejecutable Qt
# -------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PrettyDopeFileviewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(PrettyDopeFileviewer ${PROJECT_SOURCES})
endif()

# -------------------
# Incluir Poppler
# -------------------
target_include_directories(PrettyDopeFileviewer PRIVATE ${POPPLER_QT6_INCLUDE_DIRS})
target_link_directories(PrettyDopeFileviewer PRIVATE ${POPPLER_QT6_LIBRARY_DIRS})

target_link_libraries(PrettyDopeFileviewer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Pdf
    Qt${QT_VERSION_MAJOR}::PdfWidgets
    ${POPPLER_QT6_LIBRARIES}
)

# -------------------
# Copiar DLLs de runtime para Windows
# -------------------
if(WIN32)
    # Ruta donde MSYS2 MinGW64 instala DLLs
    set(MSYS2_BIN "C:/msys64/mingw64/bin")

    # Patrón de DLLs Poppler que necesitamos copiar
    file(GLOB POPPLER_DLLS
        "${MSYS2_BIN}/libpoppler-*.dll"
        "${MSYS2_BIN}/libpoppler-qt6-*.dll"
    )

    foreach(dll ${POPPLER_DLLS})
        add_custom_command(TARGET PrettyDopeFileviewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}"
                $<TARGET_FILE_DIR:PrettyDopeFileviewer>
        )
    endforeach()
endif()

# -------------------
# Propiedades y bundle
# -------------------
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PrettyDopeFileviewer)
endif()
set_target_properties(PrettyDopeFileviewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PrettyDopeFileviewer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PrettyDopeFileviewer)
endif()
