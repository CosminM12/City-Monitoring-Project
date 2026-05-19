cmake_minimum_required(VERSION 3.26)
project(OS_Project C)

set(CMAKE_C_STANDARD 11)

add_executable(OS_Project src/city_manager.c
        src/monitor_reports.c
        src/report.h
        src/scorer.c
        src/city_hub.c)
