exe="numactl -l -C 1 /home/fuchs/graph-two-release"

higgs_base="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr"
bitcoin_base="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr"
live_journal_base="--dataset_base /space/fuchs/shared/graph-two-datasets/live-journal/base.csr"
twitter_base="--dataset_base /space/fuchs/shared/graph-two-datasets/twitter/base.csr"
graph500_22_base="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-22/base.csr"
graph500_23_base="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-23/base.csr"
graph500_24_base="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-24/base.csr"
graph500_26_base="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-25/base.csr"

higgs_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/higgs/insertions.edgeList"
bitcoin_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/soc-bitcoin/insertions.edgeList"
live_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/live-journal/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/live-journal/insertions.edgeList"
twitter_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/twitter/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/twitter/insertions.edgeList"
graph500_22_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-22/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/graph500-22/insertions.edgeList"
graph500_23_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-23/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/graph500-23/insertions.edgeList"
graph500_24_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-24/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/graph500-24/insertions.edgeList"
graph500_26_insert="--dataset_base /space/fuchs/shared/graph-two-datasets/graph500-25/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/graph500-25/insertions.edgeList"


dimacs_base_u="--dataset_base /space/fuchs/shared/graph-two-datasets/dimacs-us-u/undirected_base.csr"

function prepare_experiment {
  mv /home/fuchs/graph-two/cmake-build-release-scyper15/live_graph_two /home/fuchs/graph-two-release
  rm /home/fuchs/graph-two-results.csv
}
