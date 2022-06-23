@echo off

.\mingw32\bin\g++.exe -std=c++17 -Wall -I.\includes -L. -L.\lib -lPvZ_Controller_by_CPP_DLL .\pvz_controller_by_cpp.cc .\lib\PvZ_Controller_by_CPP_DLL.lib -o PvZ_Controller_by_CPP.exe