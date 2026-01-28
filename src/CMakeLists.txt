cmake_minimum_required(VERSION 3.16)

project(cogp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== build cogp ==============================
add_executable(cogp cogp.cpp)
# ==============================================================

