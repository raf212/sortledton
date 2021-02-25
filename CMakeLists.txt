cmake_minimum_required(VERSION 3.10)
project(live_graph_two VERSION 0.1 DESCRIPTION "Data structure supporting transactional, ananlytical and pattern matching workloads on sorted data with a simple design.")

list(APPEND CMAKE_MODULE_PATH "~/cmake_modules/FindTBB")

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -DBITS32)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -DBITS64)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g -shared-libgcc")

find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy)
find_package (Threads)
find_package(OpenMP)

#add_subdirectory(tests)

include_directories(.)
include_directories(data-structures)

set(TESTS
        tests/data-src/dataset-converter/DatasetConverterTests.cpp
        tests/data-src/Asserts.h
        tests/data-src/Asserts.cpp tests/data-structures/HashSetSimulatorAdjacencyList.cpp experiments/TwoNeighbour.cpp experiments/TwoNeighbour.h)

# TODO make glob
set(SRC
        data-structures/ToplogyInterface.cpp
        data-structures/ToplogyInterface.h
        data-structures/CSR.cpp
        data-structures/adjacency-lists/EdgeBatch.cpp
        data-structures/adjacency-lists/BatchedEdgeIterator.cpp
        data-structures/adjacency-lists/AdjacencyList.cpp
        data-structures/adjacency-lists/ContiguousEdgeBatch.cpp
        data-src/EdgeList.cpp data-src/EdgeList.h
        data-src/SortedCSRDataSource.cpp
        data-src/SortedCSRDataSource.h
        data-src/DataSource.cpp data-src/DataSource.h
        data-structures/AdjacencyLists.cpp
        data-structures/AdjacencyLists.h
        data-structures/VectorAdjacencyLists.cpp
        data-structures/VectorAdjacencyLists.h
        data-structures/adjacency-lists/VectorAdjacencyList.cpp
        data-structures/adjacency-lists/VectorAdjacencyList.h
        utils/NotImplemented.cpp utils/NotImplemented.h
        data-structures/adjacency-lists/VectorBatchedEdgeIterator.cpp
        data-structures/adjacency-lists/VectorBatchedEdgeIterator.h
        experiments/Driver.cpp experiments/Driver.h experiments/Configuration.cpp experiments/Configuration.h utils/utils.cpp utils/utils.h data-src/IOException.cpp data-src/IOException.h
        experiments/Reporter.cpp experiments/Reporter.h
        data-structures/MallocAdjacencyLists.cpp
        data-structures/MallocAdjacencyLists.h
        data-structures/CSRMallocAdjacencyLists.cpp
        data-structures/CSRMallocAdjacencyLists.h
        data-structures/BlockedLinkedListAdjacencyLists.cpp
        data-structures/BlockedLinkedListAdjacencyLists.h
        data-structures/memory_pools/BlockMemoryPool.cpp
        data-structures/memory_pools/BlockMemoryPool.h
        data-structures/memory_pools/OutOfMemoryError.cpp
        data-structures/memory_pools/OutOfMemoryError.h
        data-structures/adjacency-lists/BlockedBatchedEdgeIterator.cpp
        data-structures/adjacency-lists/BlockedBatchedEdgeIterator.h
        data-structures/adjacency-lists/ContigiousBlockIterator.cpp
        utils/robin_hood.h
        experiments/TwoNeighbour.cpp
        experiments/TwoNeighbour.h
        data-structures/adjacency-lists/ContigiousBlockIterator.h data-structures/BlockedSkipListAdjacencyLists.cpp
        data-structures/BlockedSkipListAdjacencyLists.h data-structures/memory_pools/NonContigiousMemoryPool.cpp
        data-structures/memory_pools/NonContigiousMemoryPool.h experiments/BFSSourceSelector.cpp experiments/BFSSourceSelector.h
        experiments/Algorithms.cpp experiments/Algorithms.h experiments/TwoNeighbourSourceSelector.cpp experiments/TwoNeighbourSourceSelector.h
        data-structures/HashSetSimulatorAdjacencyList.cpp data-structures/HashSetSimulatorAdjacencyList.h data-structures/adjacency-lists/FilteredVectorIterator.cpp
        data-structures/adjacency-lists/FilteredVectorIterator.h data-structures/adjacency-lists/EdgeIterator.cpp
        data-structures/adjacency-lists/EdgeIterator.h data-structures/HashSetAdjacencyLists.cpp data-structures/HashSetAdjacencyLists.h experiments/PageRank.cpp experiments/PageRank.h
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
        versioning/VersionedEdgeIterator.cpp versioning/VersionedEdgeIterator.h experiments/GAPBSAlgorithms.cpp experiments/GAPBSAlgorithms.h
        third-party/gapbs.h
        versioning/VertexIndex.cpp
        versioning/VertexIndex.h)

set(DATASET_CONVERTER_SRC
        data-src/dataset-converter/DatasetConverter.cpp
        data-src/dataset-converter/Options.cpp
        data-src/SortedCSRDataSource.cpp
        data-src/DataSource.cpp
        utils/utils.cpp)


add_executable(dataset_converter data-src/dataset-converter/main.cpp ${DATASET_CONVERTER_SRC})
add_executable(bitset_analysis dataset_bitset_analyser.cpp ${SRC})
target_link_libraries (bitset_analysis Threads::Threads)
target_link_libraries(bitset_analysis OpenMP::OpenMP_CXX)


#add_executable(Google_Tests_run ${TESTS} ${DATASET_CONVERTER_SRC} ${SRC})
#target_link_libraries(Google_Tests_run gtest gtest_main gmock)
#target_link_libraries(Google_Tests_run OpenMP::OpenMP_CXX)


add_executable(live_graph_two main.cpp ${SRC})
target_link_libraries (live_graph_two Threads::Threads)
target_link_libraries(live_graph_two ${TBB_LIBRARIES})
target_link_libraries(live_graph_two OpenMP::OpenMP_CXX)

add_executable(vector_vs_blockek_linked_list  vector_vs_blockek_linked_list.h vector_vs_blockek_linked_list.cpp)
target_link_libraries(live_graph_two ${TBB_LIBRARIES})


add_library(sortledton STATIC ${SRC})
target_link_libraries(sortledton OpenMP::OpenMP_CXX)
target_link_libraries (sortledton Threads::Threads)
target_link_libraries(sortledton ${TBB_LIBRARIES})
set_target_properties(sortledton PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(sortledton PROPERTIES SOVERSION 0)
#set_target_properties(mylib PROPERTIES PUBLIC_HEADER include/mylib.h)  // TODO I should decide for a public API
# TODO support installation of library? https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
