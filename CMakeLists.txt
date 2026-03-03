cmake_minimum_required(VERSION 3.10.0)
project(c-garbage-collector VERSION 0.1.0 LANGUAGES C)

add_executable(c-garbage-collector main.c dyn_obj.c)