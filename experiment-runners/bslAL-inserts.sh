exe="numactl -l -C 1 /home/fuchs/graph-two/cmake-build-release-scyper15/live_graph_two"

experiments="--experiments bfs,insert"
repetitions="--repetitions 1"
dataset_higgs="--dataset_base /space/fuchs/shared/graph-two-datasets/higgs/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/higgs/insertions.edgeList"
dataset_yahoo="--dataset_base /space/fuchs/shared/graph-two-datasets/yahoo-songs/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/yahoo-songs/insertions.edgeList"
dataset_bitcoin="--dataset_base /space/fuchs/shared/graph-two-datasets/soc-bitcoin/base.csr --dataset_insert /space/fuchs/shared/graph-two-datasets/soc-bitcoin/insertions.edgeList"

for i in {1..10}
do
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "vectorAL(0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bslAL(32'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bslAL(64'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bslAL(128'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bslAL(248'0)"
  $exe $experiments $dataset_bitcoin $repetitions --data_structures "bslAL(512'0)"
  
  $exe $experiments $dataset_higgs $repetitions --data_structures "vectorAL(0)"
  $exe $experiments $dataset_higgs $repetitions --data_structures "bslAL(32'0)"
  $exe $experiments $dataset_higgs $repetitions --data_structures "bslAL(64'0)"
  $exe $experiments $dataset_higgs $repetitions --data_structures "bslAL(128'0)"
  $exe $experiments $dataset_higgs $repetitions --data_structures "bslAL(248'0)"
  $exe $experiments $dataset_higgs $repetitions --data_structures "bslAL(512'0)"
  
  $exe $experiments $dataset_yahoo $repetitions --data_structures "vectorAL(0)"
  $exe $experiments $dataset_yahoo $repetitions --data_structures "bslAL(32'0)"
  $exe $experiments $dataset_yahoo $repetitions --data_structures "bslAL(64'0)"
  $exe $experiments $dataset_yahoo $repetitions --data_structures "bslAL(128'0)"
  $exe $experiments $dataset_yahoo $repetitions --data_structures "bslAL(248'0)"
  $exe $experiments $dataset_yahoo $repetitions --data_structures "bslAL(512'0)"
done