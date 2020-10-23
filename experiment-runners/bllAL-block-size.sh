cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments 2-neighbour,bfs"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$higgs_base" "$bitcoin_base" "$twitter_base" "$live_journal_base" "$graph500_22_base" "$dimacs_base_u" "$graph500_23_base" "$graph500_24_base"
do
  $exe $dataset $default_parameters "bllAL(516'0'adjust)"
done