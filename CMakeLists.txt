cmake_minimum_required(VERSION 3.10)
project(sortledton VERSION 1.0 DESCRIPTION "A Universal, Transactional Graph Data Structure")

list(APPEND CMAKE_MODULE_PATH "~/cmake_modules/FindTBB")
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Wno-unused-parameter -finline-functions)

# Compile with 32 bit identifiers. Not used.
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -DBITS32)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -DBITS64)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g -march=native -mtune=native -shared-libgcc")

find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy)
find_package (Threads)
find_package(OpenMP)

include_directories(.)
include_directories(data-structures)

set(SRC
        data-structures/ToplogyInterface.cpp
        data-structures/ToplogyInterface.h
        data-src/EdgeList.cpp data-src/EdgeList.h
        data-src/SortedCSRDataSource.cpp
        data-src/SortedCSRDataSource.h
        data-src/DataSource.cpp data-src/DataSource.h
        utils/NotImplemented.cpp utils/NotImplemented.h
        internal-driver/Driver.cpp internal-driver/Driver.h
        internal-driver/Configuration.cpp internal-driver/Configuration.h
        utils/utils.cpp utils/utils.h
        data-src/IOException.cpp data-src/IOException.h
        internal-driver/Reporter.cpp internal-driver/Reporter.h
        internal-driver/BFSSourceSelector.cpp internal-driver/BFSSourceSelector.h
        algorithms/Algorithms.cpp algorithms/Algorithms.h
        algorithms/PageRank.cpp algorithms/PageRank.h
        data_types.cpp
        versioning/SizeVersionChainEntry.cpp versioning/SizeVersionChainEntry.h
        versioning/VersionedTopologyInterface.cpp versioning/VersionedTopologyInterface.h
        versioning/VersioningBlockedSkipListAdjacencyList.cpp versioning/VersioningBlockedSkipListAdjacencyList.h
        versioning/Precondition.cpp versioning/Precondition.h
        versioning/IllegalOperation.cpp versioning/IllegalOperation.h
        versioning/Transaction.cpp versioning/Transaction.h
        versioning/SnapshotTransaction.cpp versioning/SnapshotTransaction.h
        versioning/TransactionManager.cpp versioning/TransactionManager.h
        versioning/VertexExistsPrecondition.cpp versioning/VertexExistsPrecondition.h
        versioning/EdgeDoesNotExistsPrecondition.cpp versioning/EdgeDoesNotExistsPrecondition.h
        algorithms/GAPBSAlgorithms.cpp algorithms/GAPBSAlgorithms.h
        third-party/gapbs.h
        versioning/VertexIndex.cpp
        versioning/VertexIndex.h utils/pointerTagging.h
        versioning/EdgeBlock.cpp versioning/EdgeBlock.h
        versioning/AdjacencySetTypes.h
        algorithms/WCC.cpp algorithms/WCC.h
        algorithms/CDLP.cpp algorithms/CDLP.h
        algorithms/SSSP.cpp algorithms/SSSP.h
        algorithms/LCC.cpp algorithms/LCC.h
        internal-driver/GraphalyticsProperties.cpp internal-driver/GraphalyticsProperties.h
        internal-driver/Experiments.cpp internal-driver/Experiments.h
        versioning/VersionedBlockedEdgeIterator.cpp versioning/VersionedBlockedEdgeIterator.h
        versioning/memory_allocation/BlockProvider.cpp versioning/memory_allocation/BlockProvider.h
        versioning/EdgeVersionRecord.cpp versioning/EdgeVersionRecord.h
        versioning/VersionedBlockedPropertyEdgeIterator.cpp versioning/VersionedBlockedPropertyEdgeIterator.h
        third-party/RWSpinLock.cpp third-party/RWSpinLock.h)

# Library to use with the GFE Driver
add_library(sortledton STATIC ${SRC})
target_link_libraries(sortledton OpenMP::OpenMP_CXX)
target_link_libraries (sortledton Threads::Threads)
target_link_libraries(sortledton ${TBB_LIBRARIES})
set_target_properties(sortledton PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(sortledton PROPERTIES SOVERSION 1)

# Binary used for internal driver
add_executable(sortledton_bin main.cpp ${SRC})
target_link_libraries (sortledton_bin Threads::Threads)
target_link_libraries(sortledton_bin ${TBB_LIBRARIES})
target_link_libraries(sortledton_bin OpenMP::OpenMP_CXX)

# Dataset converter for internally binary input format.
set(DATASET_CONVERTER_SRC
        data-src/dataset-converter/DatasetConverter.cpp
        data-src/dataset-converter/Options.cpp
        data-src/SortedCSRDataSource.cpp
        data-src/DataSource.cpp
        utils/utils.cpp)
add_executable(dataset_converter data-src/dataset-converter/main.cpp ${DATASET_CONVERTER_SRC})


