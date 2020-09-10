exe="numactl -l -C 1 /home/fuchs/graph-two/cmake-build-release-scyper15/live_graph_two"

experiments="--experiments bfs,2-neighbour"
repetitions="--repetitions 20"
dataset_higgs="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr"
dataset_yahoo="--dataset_base /space/fuchs/shared/graph-two-datasets/yahoo-songs/base.csr"
dataset_bitcoin="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr"

data_structures="--data_structures csr"

for i in {1..1}
do
  $exe $experiments $data_structures  $dataset_bitcoin $repetitions
done