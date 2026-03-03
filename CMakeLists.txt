cmake_minimum_required(VERSION 3.16)

project(csv-runner VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(KF6 REQUIRED COMPONENTS Config CoreAddons I18n Runner)

add_library(krunner_csvrunner MODULE
    src/csvrunner.cpp
)

target_link_libraries(krunner_csvrunner
    PRIVATE
        Qt6::Core
        Qt6::Gui
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::I18n
        KF6::Runner
)

target_include_directories(krunner_csvrunner PRIVATE src)

install(TARGETS krunner_csvrunner DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt6/plugins/kf6/krunner)

install(FILES
    icons/joplin.svg
    icons/teams.svg
    icons/pass.svg
    icons/otp.svg
    icons/www.svg
    icons/mail.svg
    icons/csv-runner.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps
)