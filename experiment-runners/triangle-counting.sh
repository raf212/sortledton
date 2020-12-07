cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments triangle"
repetitions="--repetitions 1"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$higgs_base" "$live_journal_base" "$graph500_22_base" "$dimacs_base_u" "$twitter_base"
do
  $exe $dataset $default_parameters "csr(0),bslAL(256'0)"
done