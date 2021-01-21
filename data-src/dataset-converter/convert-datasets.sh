#!/usr/bin/env bash

cd /home/fuchs/graph-two/cmake-build-release-scyper15/ || exit

SRC_PATH=/space/fuchs/shared/datasets/
TARGET_PATH=/space/fuchs/shared/graph-two-datasets/

DEFAULT_ARGS="--insert_percentage 0.1 --delete_percentage 0.1"
NO_INSERT_DELETE="--insert_percentage 0.0 --delete_percentage 0.0"
ALL_INSERT="--insert_percentage 0.99 --delete_percentage 0.0"

mkdir -p $TARGET_PATH
mkdir $TARGET_PATH/twitter
mkdir $TARGET_PATH/higgs
mkdir $TARGET_PATH/soc-bitcoin
mkdir $TARGET_PATH/live-journal
mkdir $TARGET_PATH/live-journal-full
mkdir $TARGET_PATH/live-journal-full-insert
mkdir $TARGET_PATH/dimacs-us
mkdir $TARGET_PATH/graph500-22
mkdir $TARGET_PATH/graph500-23
mkdir $TARGET_PATH/graph500-24
mkdir $TARGET_PATH/graph500-26

#mkdir $TARGET_PATH/rec-amz-books
#mkdir $TARGET_PATH/yahoo-songs

mkdir $TARGET_PATH/graph500-22-u
mkdir $TARGET_PATH/live-journal-u
mkdir $TARGET_PATH/example-u
mkdir $TARGET_PATH/dimacs-us-u

#./dataset_converter ${NO_INSERT_DELETE} --make_undirected --densify ${SRC_PATH}example-undirected.e ${TARGET_PATH}/example-u/

#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}out.higgs-twitter-social ${TARGET_PATH}higgs/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}out.soc-LiveJournal1 ${TARGET_PATH}live-journal/
#./dataset_converter ${NO_INSERT_DELETE} --densify ${SRC_PATH}out.soc-LiveJournal1 ${TARGET_PATH}live-journal-full/
./dataset_converter ${ALL_INSERT} --densify ${SRC_PATH}out.soc-LiveJournal1 ${TARGET_PATH}live-journal-full-insert/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}soc-bitcoin.edges ${TARGET_PATH}soc-bitcoin/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}graph500-22.e ${TARGET_PATH}/graph500-22/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}graph500-23.e ${TARGET_PATH}/graph500-23/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}graph500-24.e ${TARGET_PATH}/graph500-24/

#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}out.yahoo-song ${TARGET_PATH}yahoo-songs/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}rec-amz-books.edges ${TARGET_PATH}rec-ama-books/

#./dataset_converter ${NO_INSERT_DELETE} --make_undirected --densify ${SRC_PATH}graph500-22.e ${TARGET_PATH}/graph500-22-u/
#./dataset_converter ${NO_INSERT_DELETE} --make_undirected --densify ${SRC_PATH}out.soc-LiveJournal1 ${TARGET_PATH}/live-journal-u/
#./dataset_converter ${DEFAULT_ARGS} --make_undirected --densify ${SRC_PATH}out.dimacs9-USA ${TARGET_PATH}/dimacs-us-u/
#
#
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}/out.twitter_mpi ${TARGET_PATH}/twitter/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}graph500-26.e ${TARGET_PATH}/graph500-26/

