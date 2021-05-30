//
// Created by per on 30.05.21.
//

#include <cassert>
#include <forward_list>
#include <cstring>
#include "EdgeVersionRecord.h"
#include <utils/pointerTagging.h>
#include <utils/NotImplemented.h>


struct VersionChainRecord {
    VersionChainRecord(version_t v, weight_t w, EdgeOperation operation) : v(v), w(w), operation(operation) {};
    version_t v;
    weight_t w;
    EdgeOperation operation;
};

forward_list<VersionChainRecord> *get_chain(version_t v) {
  assert(v & MORE_VERSION_MASK);
  return (forward_list<VersionChainRecord> *) get_pointer(v);
}

forward_list<VersionChainRecord>::iterator
get_version_from_chain(forward_list<VersionChainRecord> &chain, version_t version) {
  auto i = chain.begin();
  while (i->v > version) {
    i++;
    assert(i != chain.end());  // This should not happen as the last entry in a chain has v == FIRST_VERSION.
  }
  return i;
}

EdgeVersionRecord::EdgeVersionRecord(dst_t e, version_t *v, char *w, bool has_weight, size_t weight_size)
  : e(e), v(v), w(w), has_weight(has_weight), weight_size(weight_size) {
  if (*v & MORE_VERSION_MASK) {
    state = MULTIPLE_VERSIONS;
  } else if (*v == FIRST_VERSION) {
    state = SINGLE_VERSION;
  } else {
    state = TWO_VERSIONS;
  }

}

weight_t EdgeVersionRecord::get_weight(version_t version) const {
  assert(has_weight);
  switch (state) {
    case SINGLE_VERSION:
    case TWO_VERSIONS: {
      return *(weight_t*) w;
    }
    case MULTIPLE_VERSIONS: {
      auto chain = get_chain(*v);
      auto i = get_version_from_chain(*chain, version);
      return i->w;
    }
    default: throw NotImplemented();
  }
}

bool EdgeVersionRecord::exists_in_version(version_t version) const {
  switch (state) {
    case SINGLE_VERSION: {
      return true;
    }
    case TWO_VERSIONS: {
      if (version >= timestamp(*v)) {
        return !is_deletion(*v);
      } else {
        return is_deletion(*v);
      }
    }
    case MULTIPLE_VERSIONS: {
      auto chain = get_chain(*v);
      auto i = get_version_from_chain(*chain, version);
      return i->operation != DELETION;
    }
    default:
      throw NotImplemented();
  }
}

void EdgeVersionRecord::gc(version_t min_version) {
  if (state == MULTIPLE_VERSIONS) {
    // Get the chain and the element read by min_version.
    auto chain = get_chain(*v);
    forward_list<VersionChainRecord>::iterator last_element_to_keep = get_version_from_chain(*chain, min_version);
    last_element_to_keep->v = FIRST_VERSION;

    // We move all other elements into a forward_list which will be deleted when we leave the scope.
    forward_list<VersionChainRecord> to_drop;
    to_drop.splice_after(to_drop.before_begin(), *chain, last_element_to_keep, chain->end());

    auto size = 0;
    auto i = chain->begin();
    while (i != chain->end() && size < 3) {
      size += 1;
      i++;
    }

    // We try to inline short lists.
    bool inlined = false;
    switch (size) {
      case 1: {
        auto r = chain->begin();
        *v = r->v;
        if (r->operation == DELETION) {
          *v |= DELETION_MASK;
        }
        inlined = true;
        break;
      }
      case 2: {
        auto r1 = *chain->begin();
        auto r2 = *chain->begin()++;

        // We have only one weight and can inline
        if (r1.operation == DELETION || r2.operation == DELETION) {
          *v = r1.v;
          if (r1.operation == DELETION) {
            *v |= DELETION_MASK;
          }
          inlined = true;
        }
        break;
      }
      default: {
        // NOP cannot inline
      }
    }

    if (inlined) {
      state = TWO_VERSIONS;
      free(chain);
    }
  }
}

void EdgeVersionRecord::write(version_t version, EdgeOperation kind, char *weight) {
  assert_can_write(version, kind);

  switch (state) {
    case SINGLE_VERSION: {
      switch (kind) {
        case UPDATE: {
          create_version_chain();
          write(version, kind, weight);
          return;
        }
        case INSERTION: {
          *v = version;
          *w = copy_weight(weight);
          break;
        }
        case DELETION: {
          *v = version;
          *v |= DELETION_MASK;
          break;
        }
      }
      break;
    }
    case TWO_VERSIONS: {
      create_version_chain();
      write(version, kind, weight);
      return;
    }
    case MULTIPLE_VERSIONS: {
      auto chain = (forward_list<VersionChainRecord> *) get_pointer(*v);
      weight_t temp = weight != nullptr ? copy_weight(weight) : 0;
      chain->push_front(VersionChainRecord(version, temp, kind));
      break;
    }
  }
}

void EdgeVersionRecord::create_version_chain() {
  assert(has_weight);
  switch (state) {
    case SINGLE_VERSION: {
      auto chain = new forward_list<VersionChainRecord>();
      chain->push_front(VersionChainRecord(FIRST_VERSION, *w, INSERTION));
      *v = (version_t) chain;
      *v |= MORE_VERSION_MASK;
      state = MULTIPLE_VERSIONS;
      break;
    }
    case TWO_VERSIONS: {
      auto chain = new forward_list<VersionChainRecord>();
      auto first_operation = is_deletion(*v) ? INSERTION : DELETION;
      auto second_operation = is_deletion(*v) ? DELETION : INSERTION;

      chain->push_front(VersionChainRecord(FIRST_VERSION, *w, first_operation));
      chain->push_front(VersionChainRecord(timestamp(*v), *w, second_operation));
      *v = (version_t) chain;
      *v |= MORE_VERSION_MASK;
      state = MULTIPLE_VERSIONS;
      break;
    }
    case MULTIPLE_VERSIONS: {
      return;
    }

  }

}

weight_t EdgeVersionRecord::copy_weight(char *weight) {
  weight_t temp = 0;
  memcpy((char *) &temp, weight, weight_size);
  return temp;
}

void EdgeVersionRecord::assert_can_write(version_t version, EdgeOperation operation) {
#ifdef DEBUG
  version_t latest_timestamp = NO_TRANSACTION;
  EdgeOperation latest_operation;
  switch (state) {
    case SINGLE_VERSION: {
      return; // We can always write
    }
    case TWO_VERSIONS: {
      latest_timestamp = timestamp(*v);
      latest_operation = is_deletion(*v) ? DELETION : UPDATE;
      break;
    }
    case MULTIPLE_VERSIONS: {
      auto chain = get_chain(*v);
      auto r = *(chain->begin());
      latest_timestamp = r.v;
      latest_operation = r.operation;
      break;
    }
    default: {
      throw NotImplemented();
    }
  }

  assert (latest_timestamp <= version);
  switch (operation) {
    case DELETION: {
      assert(latest_operation != DELETION);
      break;
    }
    case INSERTION: {
      assert(latest_operation != INSERTION);
      break;
    }
    case UPDATE: {
      assert(latest_operation != DELETION);
    }

  }
#endif
}

void EdgeVersionRecord::assert_version_list(version_t min_version) {
  assert(state == MULTIPLE_VERSIONS);
  auto chain = get_chain(*v);

  auto i = chain->begin();
  version_t last_timestamp = NO_TRANSACTION;
  while (i != chain->end()) {
    assert(i->v < last_timestamp);
    assert(i->v >= min_version);
    last_timestamp = i->v;
    i++;
  }
  assert(last_timestamp == FIRST_VERSION);
}

