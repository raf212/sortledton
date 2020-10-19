#!/usr/bin/env bash
exe=./cmake-build-release-scyper15/bitset_analysis

dataset_path=/space/fuchs/shared/graph-two-datasets/

$exe "${dataset_path}higgs/base.csr"
$exe "${dataset_path}twitter/base.csr"
$exe "${dataset_path}soc-bitcoin/base.csr"
$exe "${dataset_path}graph500-22/base.csr"
$exe "${dataset_path}live-journal/base.csr"