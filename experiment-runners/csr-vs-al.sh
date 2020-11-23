cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments bfs"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$live_journal_base" #"$higgs_base" "$bitcoin_base" "$live_journal_base" "$graph500_22_base" "$graph500_23_base" "$graph500_24_base" "$twitter_base"
do
  $exe $dataset $default_parameters "bllAL(256'0'adjust'raw),bllAL(512'0'adjust'raw),csrMallocAL(512'0'raw),csrMallocAL(1000000'0'raw),csrMallocAL(0'0'raw),mallocAL(0'0'raw),csr(0'raw)"
done