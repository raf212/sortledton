cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments insert"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$higgs_insert" "$bitcoin_insert" "$live_journal_insert" "$graph500_22_insert" "$twitter_insert"
do
  $exe $dataset $default_parameters "bslAL(128'0),vectorAL(0)"
done