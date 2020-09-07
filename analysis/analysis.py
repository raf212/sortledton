#!/usr/bin/env python

import subprocess
import pandas as pd
import numpy as np
from enum import Enum

REMOTE_USER = "fuchs"
REMOTE_URL = "scyper15.in.tum.de"
REMOTE_PATH = "/home/fuchs/graph-two-results.csv"
LOCAL_PATH = "/home/per/graph-two-results.csv"

TEMPLATE_PATH = "analysis/html"

pd.options.display.precision = 2

class DataStructure(Enum):
    CSR = "csr"


def get_report_file(remote_user, remote_url, remote_path, local_path):
    if input("Get report?") == "y":
        file_location = "%s@%s:%s" % (remote_user, remote_url, remote_path)
        result = subprocess.run(["scp", file_location, local_path])
        if result.returncode != 0:
            print("Could update report file from %s" % file_location)


# Drops the first three repetition as warmup.
def filter_out_warmup(data):
    return data[data["repetition"] > 2]


def get_last_execution(data):
    max_execution_id = data["execution_id"].max()
    return data[data["execution_id"] == max_execution_id]


def rewrite_dataset(data):
    data["dataset"] = data["dataset"].apply(lambda ds: ds.split("/")[-2])
    return data


def generate_report():
  data = pd.read_csv(LOCAL_PATH, delimiter=";")
  data = filter_out_warmup(data)
  data = get_last_execution(data)
  data = rewrite_dataset(data)

  pivot = data.pivot_table(index=["experiment", "dataset"],
                           columns=["data_structure"],
                           values=["runtime", "storage"], aggfunc=np.mean)

  for ds in pivot.columns.levels[1]:
      if ds == "csr":
          continue
      pivot["ratios", ds] = pivot["runtime", ds] / pivot["runtime", "csr"]

  print(pivot)
  return pivot


get_report_file(REMOTE_USER, REMOTE_URL, REMOTE_PATH, LOCAL_PATH)

global data
data = generate_report()