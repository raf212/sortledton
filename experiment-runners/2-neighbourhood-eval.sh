cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments 2-neighbour"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$higgs_base" "$bitcoin_base" "$twitter_base" "$live_journal_base" "$graph500_22_base" "$dimacs_base_u" "$graph500_23_base" "$graph500_24_base"
do
  $exe $dataset $default_parameters "bllAL(512'0'adjust'raw),bllAL(128'0'adjust),csr(0'raw),hsAL(0.9'raw),hsAL(0.6'raw),hsAL(0.1'raw)"
done