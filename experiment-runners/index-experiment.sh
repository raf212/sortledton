cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments bfs,2-neighbour"
repetitions="--repetitions 10"

default_parameters="--release_run $experiments $repetitions --data_structures"

for dataset in "$dimacs_base_u" "$higgs_base" "$bitcoin_base" "$twitter_base" "$live_journal_base" "$graph500_22_base" "$graph500_23_base" "$graph500_24_base"
do
  $exe $dataset $default_parameters "csrMallocAL(0'0),mallocAL(0'0),vectorAL(0),mallocAL(0'1)"
done