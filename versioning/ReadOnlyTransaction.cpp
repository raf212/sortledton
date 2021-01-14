//
// Created by per on 23.12.20.
//

#include <utils/NotImplemented.h>
#include "ReadOnlyTransaction.h"

ReadOnlyTransaction::~ReadOnlyTransaction() {
  transaction.execute();  // Call execute to release the version number.
}

size_t ReadOnlyTransaction::vertex_count() {
  return transaction.vertex_count();
}
