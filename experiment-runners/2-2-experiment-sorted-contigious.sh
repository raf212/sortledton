cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments bfs,2-neighbour"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$dimacs_base_u" "$higgs_base" "$bitcoin_base" "$twitter_base" "$live_journal_base" "$graph500_22_base" "$graph500_23_base" "$graph500_24_base"
do
  $exe $dataset $default_parameters "csr(0),csr(1),csrMallocAL(0'0)csrMallocAL(0'1),csrMallocAL(10000000'0),csrMallocAL(10000000'1),csrMallocAL(32'0),csrMallocAL(32'1),csrMallocAL(512'0),csrMallocAL(512'1)"
done