#!/usr/bin/env bash

cd /home/fuchs/graph-two/cmake-build-debug-scyper15/ || exit

SRC_PATH=/space/fuchs/shared/datasets/
TARGET_PATH=/space/fuchs/shared/graph-two-datasets/

DEFAULT_ARGS="--insert_percentage 0.1 --delete_percentage 0.1"

mkdir -p $TARGET_PATH
mkdir $TARGET_PATH/twitter
mkdir $TARGET_PATH/higgs
mkdir $TARGET_PATH/yahoo-songs
mkdir $TARGET_PATH/soc-bitcoin
mkdir $TARGET_PATH/rec-amz-books
mkdir $TARGET_PATH/live-journal

#./dataset_converter ${DEFAULT_ARGS} ${SRC_PATH}out.higgs-twitter-social ${TARGET_PATH}higgs/
./dataset_converter ${DEFAULT_ARGS} ${SRC_PATH}out.soc-LiveJournal1 ${TARGET_PATH}live-journal/
#./dataset_converter ${DEFAULT_ARGS} ${SRC_PATH}out.yahoo-song ${TARGET_PATH}yahoo-songs/
#./dataset_converter ${DEFAULT_ARGS} ${SRC_PATH}soc-bitcoin.edges ${TARGET_PATH}soc-bitcoin/
#./dataset_converter ${DEFAULT_ARGS} --densify ${SRC_PATH}rec-amz-books.edges ${TARGET_PATH}rec-ama-books/
#./dataset_converter ${DEFAULT_ARGS} ${SRC_PATH}/out.twitter_mpi ${TARGET_PATH}/twitter
