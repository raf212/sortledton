exe="numactl -l -C 1 /home/fuchs/graph-two/cmake-build-release-scyper15/live_graph_two"

experiments="--experiments bfs,2-neighbour"
repetitions="--repetitions 1"
dataset_higgs="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr"
dataset_yahoo="--dataset_base /space/fuchs/shared/graph-two-datasets/yahoo-songs/base.csr"
dataset_bitcoin="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr"

data_structures="--data_structures mallocAL(0)"

for i in {1..10}
do
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "mallocAL(0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bllAL(32'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bllAL(64'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bllAL(128'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bllAL(248'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bllAL(512'0)"
done