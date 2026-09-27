cmake_minimum_required(VERSION 3.21)

project(Portvane VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# moc must run on every file that declares Q_OBJECT.
set(CMAKE_AUTOMOC ON)

# Applies to every target below, so the tests are held to the same standard.
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

# WIN32 makes this a GUI app on Windows, so no console window opens next to it.
qt_add_executable(portvane WIN32
    src/main.cpp
    src/CommandLine.cpp
    src/CommandLine.h
    src/MainWindow.cpp
    src/MainWindow.h
    src/NmapLocator.cpp
    src/NmapLocator.h
    src/NmapRunner.cpp
    src/NmapRunner.h
    src/NmapXmlParser.cpp
    src/NmapXmlParser.h
    src/OptionsPanel.cpp
    src/OptionsPanel.h
    src/Privileges.cpp
    src/Privileges.h
    src/ScanData.h
    src/ScanOptions.cpp
    src/ScanOptions.h
    src/ScanTableModel.cpp
    src/ScanTableModel.h
)

target_link_libraries(portvane PRIVATE Qt6::Widgets)

# Privileges.cpp asks Windows whether the process token is elevated.
if(WIN32)
    target_link_libraries(portvane PRIVATE advapi32)
endif()

enable_testing()

qt_add_executable(test_nmaplocator
    tests/test_nmaplocator.cpp
    src/NmapLocator.cpp
    src/NmapLocator.h
)
target_include_directories(test_nmaplocator PRIVATE src)
target_link_libraries(test_nmaplocator PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_nmaplocator COMMAND test_nmaplocator)

qt_add_executable(test_nmaprunner
    tests/test_nmaprunner.cpp
    src/CommandLine.cpp
    src/CommandLine.h
    src/NmapLocator.cpp
    src/NmapLocator.h
    src/NmapRunner.cpp
    src/NmapRunner.h
)
target_include_directories(test_nmaprunner PRIVATE src)
target_link_libraries(test_nmaprunner PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_nmaprunner COMMAND test_nmaprunner)

# Fixtures are read from the source tree, so they never have to be copied.
qt_add_executable(test_nmapxmlparser
    tests/test_nmapxmlparser.cpp
    src/NmapXmlParser.cpp
    src/NmapXmlParser.h
    src/ScanData.h
)
target_include_directories(test_nmapxmlparser PRIVATE src)
target_compile_definitions(test_nmapxmlparser PRIVATE
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
target_link_libraries(test_nmapxmlparser PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_nmapxmlparser COMMAND test_nmapxmlparser)

qt_add_executable(test_scantablemodel
    tests/test_scantablemodel.cpp
    src/NmapXmlParser.cpp
    src/NmapXmlParser.h
    src/ScanData.h
    src/ScanTableModel.cpp
    src/ScanTableModel.h
)
target_include_directories(test_scantablemodel PRIVATE src)
target_compile_definitions(test_scantablemodel PRIVATE
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
target_link_libraries(test_scantablemodel PRIVATE Qt6::Gui Qt6::Test)
add_test(NAME test_scantablemodel COMMAND test_scantablemodel)

qt_add_executable(test_scanoptions
    tests/test_scanoptions.cpp
    src/ScanOptions.cpp
    src/ScanOptions.h
)
target_include_directories(test_scanoptions PRIVATE src)
target_link_libraries(test_scanoptions PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_scanoptions COMMAND test_scanoptions)

qt_add_executable(test_commandline
    tests/test_commandline.cpp
    src/CommandLine.cpp
    src/CommandLine.h
)
target_include_directories(test_commandline PRIVATE src)
target_link_libraries(test_commandline PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_commandline COMMAND test_commandline)

qt_add_executable(test_privileges
    tests/test_privileges.cpp
    src/Privileges.cpp
    src/Privileges.h
)
target_include_directories(test_privileges PRIVATE src)
target_link_libraries(test_privileges PRIVATE Qt6::Core Qt6::Test)
if(WIN32)
    target_link_libraries(test_privileges PRIVATE advapi32)
endif()
add_test(NAME test_privileges COMMAND test_privileges)

qt_add_executable(test_optionspanel
    tests/test_optionspanel.cpp
    src/OptionsPanel.cpp
    src/OptionsPanel.h
    src/Privileges.cpp
    src/Privileges.h
    src/ScanOptions.cpp
    src/ScanOptions.h
)
target_include_directories(test_optionspanel PRIVATE src)
target_link_libraries(test_optionspanel PRIVATE Qt6::Widgets Qt6::Test)
if(WIN32)
    target_link_libraries(test_optionspanel PRIVATE advapi32)
endif()
add_test(NAME test_optionspanel COMMAND test_optionspanel)
# A widget test needs a display; CI machines have none, so draw offscreen.
set_tests_properties(test_optionspanel PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
