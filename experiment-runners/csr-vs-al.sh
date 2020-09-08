#!/usr/bin/env bash

exe=/home/fuchs/graph-two/cmake-build-release-scyper15/live_graph_two

experiments="--experiments 2-neighbour,bfs"
dataset_higgs="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr"
dataset_yahoo="--dataset_base /space/fuchs/shared/graph-two-datasets/yahoo-songs/base.csr"
dataset_bitcoin="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr"
repetitions="--repetitions 20"

# Higgs dataset
$exe $experiments --data_structures csr  $dataset_higgs $repetitions

$exe $experiments --data_structures csrMallocAL\(0\'0\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(0\'1\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(32\'0\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(64\'0\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(128\'0\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'0\)  $dataset_higgs $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'1\)  $dataset_higgs $repetitions

$exe $experiments --data_structures mallocAL\(1\)  $dataset_higgs $repetitions
$exe $experiments --data_structures mallocAL\(0\)  $dataset_higgs $repetitions

$exe $experiments --data_structures vectorAL $dataset_higgs $repetitions


# Yahoo dataset
$exe $experiments --data_structures csr  $dataset_yahoo $repetitions

$exe $experiments --data_structures csrMallocAL\(0\'0\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(0\'1\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(32\'0\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(64\'0\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(128\'0\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'0\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'1\)  $dataset_yahoo $repetitions

$exe $experiments --data_structures mallocAL\(1\)  $dataset_yahoo $repetitions
$exe $experiments --data_structures mallocAL\(0\)  $dataset_yahoo $repetitions

$exe $experiments --data_structures vectorAL $dataset_yahoo $repetitions


# Bitcoin dataset
$exe $experiments --data_structures csr  $dataset_bitcoin $repetitions

$exe $experiments --data_structures csrMallocAL\(0\'0\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(0\'1\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(32\'0\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(64\'0\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(128\'0\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'0\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures csrMallocAL\(10000000\'1\)  $dataset_bitcoin $repetitions

$exe $experiments --data_structures mallocAL\(1\)  $dataset_bitcoin $repetitions
$exe $experiments --data_structures mallocAL\(0\)  $dataset_bitcoin $repetitions

$exe $experiments --data_structures vectorAL $dataset_bitcoin $repetitions