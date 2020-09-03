cmake_minimum_required(VERSION 3.10)
project(live_graph_two)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(data-structures)

# TODO make glob
set(SRC
        data-structures/ToplogyInterface.cpp
        data-structures/CSR.cpp
        data-structures/adjacency-lists/EdgeBatch.cpp
        data-structures/adjacency-lists/BatchedEdgeIterator.cpp
        data-structures/adjacency-lists/AdjacencyList.cpp
        data-structures/adjacency-lists/ContiguousEdgeBatch.cpp
        data-src/EdgeList.cpp data-src/EdgeList.h data-src/SortedCSRDataSource.cpp data-src/SortedCSRDataSource.h data-src/DataSource.cpp data-src/DataSource.h data-structures/AdjacencyLists.cpp data-structures/AdjacencyLists.h data-structures/VectorAdjacencyLists.cpp data-structures/VectorAdjacencyLists.h data-structures/adjacency-lists/VectorAdjacencyList.cpp data-structures/adjacency-lists/VectorAdjacencyList.h utils/NotImplemented.cpp utils/NotImplemented.h data-structures/adjacency-lists/VectorBatchedEdgeIterator.cpp data-structures/adjacency-lists/VectorBatchedEdgeIterator.h experiments/Driver.cpp experiments/Driver.h experiments/Configuration.cpp experiments/Configuration.h utils/utils.cpp utils/utils.h data-src/IOException.cpp data-src/IOException.h
        experiments/Reporter.cpp experiments/Reporter.h)

set(DATASET_CONVERTER_SRC
        data-src/dataset-converter/DatasetConverter.cpp
        data-src/dataset-converter/Options.cpp
        data-src/SortedCSRDataSource.cpp
        data-src/DataSource.cpp
        utils/utils.cpp)


add_executable(live_graph_two main.cpp ${SRC})
add_executable(dataset_converter data-src/dataset-converter/main.cpp ${DATASET_CONVERTER_SRC})