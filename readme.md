# Sortledton: a Universal Transactional Data Structure

The data structure to the paper "Sortledton: a Universal, Transactional Graph Data Structure".
To be used with the [GFE experiment driver](TODO).

## Prerequisite

- A C++17 compliant compiler with support for OpenMP. We used GCC 10.
- Intel Threading Building Blocks 2 (version 2020.1-2)
- Disable NUMA balancing feature to avoid the Linux Kernel to swap pages during insertions: `echo 0 | sudo tee  /proc/sys/kernel/numa_balancing`

## Building
To build the static Sortledton library:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make sortledton
```

## Repository Structure

* `data-structure/` the sortledton data structure
* `algorithms/`     the GAPBS algorithms
* `internal-driver/` driver used for internal test runs - not used for the paper
* `internal-driver/data-source` program used to generate binary graph data for the internal driver

## Usage in your own work

TODO see Sortledton Driver

TODO move data_types to utils
TODO move main to internal driver
TODO delete lib
TODO move LCC and CDLP to gfe driver?

TODO check if analysis, build, bin, ... in Git