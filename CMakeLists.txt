cmake_minimum_required(VERSION 3.16)
project(widgetsboxmodel-ng  VERSION 8.6)

find_package(Albert REQUIRED)

albert_plugin(QT Widgets StateMachine Svg)

install(
    DIRECTORY "themes/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/themes"
    REGEX "themes\\/\\..+" EXCLUDE  # exclude hidden files
)
