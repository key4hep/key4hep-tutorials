cmake_minimum_required(VERSION 3.16)
project(make_pi0_plots LANGUAGES CXX)

# Find required packages
find_package(ROOT REQUIRED COMPONENTS Core Hist)
find_package(podio REQUIRED)
find_package(EDM4HEP REQUIRED)

# Create executable
add_executable(make_pi0_plots make_pi0_plots.cpp)

# Link libraries
target_link_libraries(make_pi0_plots
    ROOT::Core
    ROOT::Hist
    podio::podioIO
    EDM4HEP::edm4hep
)

# Set C++ standard
target_compile_features(make_pi0_plots PRIVATE cxx_std_20)
