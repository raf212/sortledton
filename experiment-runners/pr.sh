cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments pr"
repetitions="--repetitions 1"

default_parameters="$experiments $repetitions --release_run --data_structures"

for dataset in "$higgs_base" "$bitcoin_base" "$live_journal_base" "$graph500_22_base" "$graph500_23_base" "$graph500_24_base" "$twitter_base"
do
  $exe $dataset $default_parameters "bslAL(256'0'adjust'si),csr(0'pr)"
done
