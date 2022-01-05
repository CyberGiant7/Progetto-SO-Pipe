cmake_minimum_required(VERSION 3.21)
project(Progetto_SO__Pipe C)

set(CMAKE_C_STANDARD 99)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
add_executable(${PROJECT_NAME}  main.c menu.c macro.h stampa.c stampa.h collisioni.c collisioni.h generazione_e_movimento.c generazione_e_movimento.h)
target_link_libraries(${PROJECT_NAME} -lncursesw)