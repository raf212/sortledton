cmake_minimum_required(VERSION 3.16)

project(
    sortledton
    VERSION 1.0
    DESCRIPTION "A Universal, Transactional Graph Data Structure"
    LANGUAGES C CXX
)

# ============================================================
# C++ configuration
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 64-bit identifiers used by Sortledton.
add_compile_definitions(BITS64)

# Debug-only definition.
add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
)

# Common compiler warnings/options.
add_compile_options(
    -Wall
    -Wextra
    -Wno-unused-parameter
    -finline-functions
)

# High-performance Release configuration.
#
# Do NOT add -O2 -g here. CMake already supplies the normal
# Release definitions, and these explicitly select our desired
# optimization level.
add_compile_options(
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    $<$<CONFIG:Release>:-mtune=native>
)

# ============================================================
# Dependencies
# ============================================================

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
find_package(TBB REQUIRED COMPONENTS tbb)

# ============================================================
# Link-time optimization
# ============================================================

include(CheckIPOSupported)

check_ipo_supported(
    RESULT SORTLEDTON_IPO_SUPPORTED
    OUTPUT SORTLEDTON_IPO_ERROR
)

if(SORTLEDTON_IPO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    message(STATUS "IPO/LTO enabled for Release builds")
else()
    message(WARNING "IPO/LTO unavailable: ${SORTLEDTON_IPO_ERROR}")
endif()

# ============================================================
# Sortledton sources
# ============================================================

set(SRC
    data-structure/TopologyInterface.cpp
    data-structure/TopologyInterface.h

    internal-driver/data-src/EdgeList.cpp
    internal-driver/data-src/EdgeList.h

    internal-driver/data-src/SortedCSRDataSource.cpp
    internal-driver/data-src/SortedCSRDataSource.h

    internal-driver/data-src/DataSource.cpp
    internal-driver/data-src/DataSource.h

    utils/NotImplemented.cpp
    utils/NotImplemented.h

    internal-driver/Driver.cpp
    internal-driver/Driver.h

    internal-driver/Configuration.cpp
    internal-driver/Configuration.h

    utils/utils.cpp
    utils/utils.h

    internal-driver/data-src/IOException.cpp
    internal-driver/data-src/IOException.h

    internal-driver/Reporter.cpp
    internal-driver/Reporter.h

    internal-driver/BFSSourceSelector.cpp
    internal-driver/BFSSourceSelector.h

    algorithms/Algorithms.cpp
    algorithms/Algorithms.h

    algorithms/PageRank.cpp
    algorithms/PageRank.h

    data-structure/data_types.cpp

    data-structure/SizeVersionChainEntry.cpp
    data-structure/SizeVersionChainEntry.h

    data-structure/VersionedTopologyInterface.cpp
    data-structure/VersionedTopologyInterface.h

    data-structure/VersioningBlockedSkipListAdjacencyList.cpp
    data-structure/VersioningBlockedSkipListAdjacencyList.h

    data-structure/Precondition.cpp
    data-structure/Precondition.h

    data-structure/IllegalOperation.cpp
    data-structure/IllegalOperation.h

    data-structure/Transaction.cpp
    data-structure/Transaction.h

    data-structure/SnapshotTransaction.cpp
    data-structure/SnapshotTransaction.h

    data-structure/TransactionManager.cpp
    data-structure/TransactionManager.h

    data-structure/VertexExistsPrecondition.cpp
    data-structure/VertexExistsPrecondition.h

    data-structure/EdgeDoesNotExistsPrecondition.cpp
    data-structure/EdgeDoesNotExistsPrecondition.h

    algorithms/GAPBSAlgorithms.cpp
    algorithms/GAPBSAlgorithms.h

    third-party/gapbs.h

    data-structure/VertexIndex.cpp
    data-structure/VertexIndex.h

    utils/pointerTagging.h

    data-structure/EdgeBlock.cpp
    data-structure/EdgeBlock.h

    data-structure/AdjacencySetTypes.h

    algorithms/WCC.cpp
    algorithms/WCC.h

    algorithms/CDLP.cpp
    algorithms/CDLP.h

    algorithms/SSSP.cpp
    algorithms/SSSP.h

    algorithms/LCC.cpp
    algorithms/LCC.h

    internal-driver/GraphalyticsProperties.cpp
    internal-driver/GraphalyticsProperties.h

    internal-driver/Experiments.cpp
    internal-driver/Experiments.h

    data-structure/VersionedBlockedEdgeIterator.cpp
    data-structure/VersionedBlockedEdgeIterator.h

    data-structure/memory_allocation/BlockProvider.cpp
    data-structure/memory_allocation/BlockProvider.h

    data-structure/EdgeVersionRecord.cpp
    data-structure/EdgeVersionRecord.h

    data-structure/VersionedBlockedPropertyEdgeIterator.cpp
    data-structure/VersionedBlockedPropertyEdgeIterator.h

    third-party/RWSpinLock.cpp
    third-party/RWSpinLock.h
)

# ============================================================
# Sortledton static library
# ============================================================

add_library(sortledton STATIC ${SRC})

target_include_directories(
    sortledton
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    sortledton
    PUBLIC
        Threads::Threads
        OpenMP::OpenMP_CXX
        TBB::tbb
)

set_target_properties(
    sortledton
    PROPERTIES
        VERSION ${PROJECT_VERSION}
)

# ============================================================
# Sortledton internal driver executable
# ============================================================
#
# IMPORTANT:
# Do not compile ${SRC} again here.
#
# sortledton_bin only needs its main.cpp and then links against
# the already-built sortledton static library.
#
# TBB/OpenMP/Threads propagate automatically because they are
# PUBLIC dependencies of sortledton.
# ============================================================

add_executable(
    sortledton_bin
    internal-driver/main.cpp
)

target_link_libraries(
    sortledton_bin
    PRIVATE
        sortledton
)

# ============================================================
# Dataset converter
# ============================================================

set(
    DATASET_CONVERTER_SRC
    internal-driver/data-src/dataset-converter/DatasetConverter.cpp
    internal-driver/data-src/dataset-converter/Options.cpp
    internal-driver/data-src/SortedCSRDataSource.cpp
    internal-driver/data-src/DataSource.cpp
    utils/utils.cpp
)

add_executable(
    dataset_converter
    internal-driver/data-src/dataset-converter/main.cpp
    ${DATASET_CONVERTER_SRC}
)

target_include_directories(
    dataset_converter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    dataset_converter
    PRIVATE
        Threads::Threads
)