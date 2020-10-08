source variables

experiments="--experiments bfs,2-neighbour"
repetitions="--repetitions 10"

default_parameters="$experiments $repetitions --data_structures"

for dataset in $higgs_base $bitcoin_base $twitter_base $dimacs_us_base $live_journal_base $graph500_22_base $graph500_23_base $graph500_24_base
do
  # sorted, uncontiguous
  $exe "--dataset $dataset" $default_parameters "csr(0),csrMalloc(0'0)"
  # sorted, contiguous
  $exe "--dataset $dataset" $default_parameters "csr(0),csrMalloc(0'10000000)"
  # unsorted, uncontiguous
  $exe "--dataset $dataset" $default_parameters "csr(1),csrMalloc(1'0)"
  # unsorted, contiguous
  $exe "--dataset $dataset" $default_parameters "csr(1),csrMalloc(1'10000000)"

  # in between values, sorted
  $exe "--dataset $dataset" $default_parameters "csrMalloc(0'32)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(0'64)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(0'128)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(0'256)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(0'512)"

  # in between values, unsorted
  $exe "--dataset $dataset" $default_parameters "csrMalloc(1'32)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(1'64)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(1'128)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(1'256)"
  $exe "--dataset $dataset" $default_parameters "csrMalloc(1'512)"
done