//
// Created by per on 14.10.20.
//

#include "FilteredVectorIterator.h"

dst_t FilteredVectorIterator::next() {
  auto ret = *data;
  data++;
  return ret;
}

bool FilteredVectorIterator::has_next() {
  if (data == nullptr) {
    return false;
  }
  while(data < end && *data == empty) {
    data++;
  }

  return data < end;
}

void FilteredVectorIterator::initialize(dst_t *data, size_t size) {
  this->data = data;
  end = data + size;
}
