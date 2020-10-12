cd "$(dirname "$0")" || exit 1

. ./variables.sh

prepare_experiment

experiments="--experiments 2-neighbour"
repetitions="--repetitions 10"

default_parameters="--release_run $experiments $repetitions --release_run --data_structures"

for dataset in "$dimacs_base_u" "$higgs_base" "$bitcoin_base" "$twitter_base" "$live_journal_base" "$graph500_22_base" "$graph500_23_base" "$graph500_24_base"
do
  echo $dataset
  # sorted, uncontiguous
  $exe $dataset $default_parameters "csr(0),csrMallocAL(0'0),csrMallocAL(32'512)"
  # sorted, contiguous
#  $exe $dataset $default_parameters "csr(0),csrMallocAL(10000000'0)"
  # unsorted, uncontiguous
#  $exe $dataset $default_parameters "csr(1),csrMallocAL(0'1)"
  # unsorted, contiguous
#  $exe $dataset $default_parameters "csr(1),csrMallocAL(10000000'1)"

  # in between values, sorted
#  $exe $dataset $default_parameters "csrMallocAL(0'32)"
#  $exe $dataset $default_parameters "csrMallocAL(0'64)"
#  $exe $dataset $default_parameters "csrMallocAL(0'128)"
#  $exe $dataset $default_parameters "csrMallocAL(0'256)"
#  $exe $dataset $default_parameters "csrMallocAL(0'512)"

  # in between values, unsorted
#  $exe $dataset $default_parameters "csrMallocAL(1'32)"
#  $exe $dataset $default_parameters "csrMallocAL(1'64)"
#  $exe $dataset $default_parameters "csrMallocAL(1'128)"
#  $exe $dataset $default_parameters "csrMallocAL(1'256)"
#  $exe $dataset $default_parameters "csrMallocAL(1'512)"
done