cmake_minimum_required(VERSION 3.10)
project(live_graph_two)

add_subdirectory(tests)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

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
        data-structures/adjacency-lists/ContigiousBlockIterator.h data-structures/BlockedSkipListAdjacencyLists.cpp data-structures/BlockedSkipListAdjacencyLists.h data-structures/memory_pools/NonContigiousMemoryPool.cpp data-structures/memory_pools/NonContigiousMemoryPool.h experiments/BFSSourceSelector.cpp experiments/BFSSourceSelector.h experiments/Algorithms.cpp experiments/Algorithms.h experiments/TwoNeighbourSourceSelector.cpp experiments/TwoNeighbourSourceSelector.h data-structures/HashSetSimulatorAdjacencyList.cpp data-structures/HashSetSimulatorAdjacencyList.h data-structures/adjacency-lists/FilteredVectorIterator.cpp data-structures/adjacency-lists/FilteredVectorIterator.h data-structures/adjacency-lists/EdgeIterator.cpp data-structures/adjacency-lists/EdgeIterator.h data-structures/HashSetAdjacencyLists.cpp data-structures/HashSetAdjacencyLists.h experiments/PageRank.cpp experiments/PageRank.h
        data_types.cpp
        versioning/VersionChainEntry.cpp versioning/VersionChainEntry.h
        versioning/SizeVersionChainEntry.cpp versioning/SizeVersionChainEntry.h
        versioning/VersionedTopologyInterface.cpp versioning/VersionedTopologyInterface.h
        versioning/VersioningBlockedSkipListAdjacencyList.cpp versioning/VersioningBlockedSkipListAdjacencyList.h
        versioning/Precondition.cpp versioning/Precondition.h
        versioning/IllegalOperation.cpp versioning/IllegalOperation.h
        versioning/Transaction.cpp versioning/Transaction.h
        versioning/SnapshotTransaction.cpp versioning/SnapshotTransaction.h
        versioning/ReadOnlyTransaction.cpp versioning/ReadOnlyTransaction.h
        versioning/SerializableUpdateTransaction.cpp versioning/SerializableUpdateTransaction.h
        versioning/TransactionManager.cpp versioning/TransactionManager.h
        versioning/VertexExistsPrecondition.cpp versioning/VertexExistsPrecondition.h)

set(DATASET_CONVERTER_SRC
        data-src/dataset-converter/DatasetConverter.cpp
        data-src/dataset-converter/Options.cpp
        data-src/SortedCSRDataSource.cpp
        data-src/DataSource.cpp
        utils/utils.cpp)


add_executable(dataset_converter data-src/dataset-converter/main.cpp ${DATASET_CONVERTER_SRC})
add_executable(bitset_analysis dataset_bitset_analyser.cpp ${SRC})
target_link_libraries (bitset_analysis Threads::Threads)


add_executable(Google_Tests_run ${TESTS} ${DATASET_CONVERTER_SRC} ${SRC})

target_link_libraries(Google_Tests_run gtest gtest_main gmock)

find_package (Threads)
add_executable(live_graph_two main.cpp ${SRC})
target_link_libraries (live_graph_two Threads::Threads)