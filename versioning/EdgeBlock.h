//
// Created by per on 23.03.21.
//

#ifndef LIVE_GRAPH_TWO_EDGEBLOCK_H
#define LIVE_GRAPH_TWO_EDGEBLOCK_H

#include <cassert>
#include <cstring>
#include <utils/utils.h>
#include "AdjacencySetTypes.h"

inline version_t inline_version(bool deletion, bool more_versions, version_t version) {
  if (more_versions) {
    version |= MORE_VERSION_MASK;
  }
  if (deletion) {
    version |= DELETION_MASK;
  }
  return version;
}


/**
 * Represents a block of memory which contains edges, versions and properties.
 *
 * The block starts with edges, interleaved with versions and ends with properties. The edges and versions grow
 * upwards and the properties grow downwards.
 *
 * In the unversioned case, the property value belonging to an edge has the same offset in the property section.
 * In the versioned case, the property value belonging to an edge has the same offset in the property section minus all versions that are before the edge in question.
 * This allows random access to edge and property in the unversioned case but requires scanning edges from the beginning in the versioned case.
 *
 * We do not support multiple property versions yet. They can be supported the same way as supporting edge versions but requires to clean the property section on GC.
 */
class EdgeBlock {
public:
    EdgeBlock(dst_t *start, size_t capacity, size_t edges_and_versions, size_t properties, size_t property_size)
            : start(start), capacity(capacity), end(((char*) start) + capacity * sizeof(dst_t) + capacity * property_size), edges_and_versions(edges_and_versions), properties(properties),
              property_size(property_size) {};


    static EdgeBlock from_vskip_list_header(VSkipListHeader* header, size_t block_size, size_t property_size) {
      return EdgeBlock(header->data, block_size, header->size, header->properties, property_size);
    };

    static EdgeBlock from_single_block(dst_t* start, size_t capacity, size_t edges_and_versions, size_t properties, size_t property_size) {
      return EdgeBlock(start, capacity, edges_and_versions, properties, property_size);
    }

    // TODO move methods definitions to cpp file

    bool has_space_to_insert_edge() {
      return edges_and_versions + 2 <= get_block_capacity();
    };

    /**
     * Finds the correct place to add edge, version record and properties and inserts them by shifthing.
     *
     * @param e
     * @param version
     * @param properties
     */
    void insert_edge(dst_t e, version_t version, char *properties) {
      assert(has_space_to_insert_edge());
      int offset = insert_edge_and_version_by_shift(e, version);
      offset -= count_versions_before(offset);

      insert_properties_by_shift(properties, offset);
      edges_and_versions += 2;
      this->properties += 1;
    };

    /**
     * Removes all version records < min_version.
     * @param min_version
     */
    bool gc(version_t min_version) {
      // Removes unncessary versions and shifts remaining destinations and versions forward.
      auto shift = 0; // The forward shift to use, increases when versions are removed.
      bool version_remaining = false;
      size_t new_size = edges_and_versions;
      for (auto i = start; i < start + edges_and_versions; i++) {
        auto e = *i;
        if (is_versioned(e) && timestamp(*(i + 1) < min_version)) {
          if (is_deletion(*(i + 1))) {
            new_size -= 2;
            shift += 2;
            i += 2;
          } else {
            *(i - shift) = make_unversioned(e);
            new_size -= 1;
            shift += 1;
            i += 1;
          }
        } else {
          if (is_versioned(e)) {
            version_remaining = true;
          }
          *(i - shift) = e;
        }
      }
      edges_and_versions = new_size;
      return version_remaining;
      // TODO need to clean properties
    };

    void copy_into(EdgeBlock &other) {
      assert(size() <= other.size());

      other.edges_and_versions = edges_and_versions;
      other.properties = properties;
      other.property_size = property_size;

      memcpy(other.start, start, edges_and_versions * sizeof(dst_t));
      memcpy(other.properties_start(), properties_start(), properties * property_size);
    };

    tuple<size_t, size_t> split_into(EdgeBlock &other) {
      auto split = edges_and_versions / 2;

      if (is_versioned(start[split-1])) { // Keep the versioned edge together with its version.
        split -= 1;
      }
      memcpy(other.start, start + split, (edges_and_versions - split) * sizeof(dst_t));

      auto property_split = split - count_versions_before(split);
      auto properties_to_move = properties - property_split;

      // Copy properties into new block.
      memcpy((char*) other.end - properties_to_move * property_size, (char*) end - properties_to_move * property_size, properties_to_move * property_size);
      // Move properties in existing block
      memmove(end - property_split * property_size, properties_start(), property_split * property_size);

      other.edges_and_versions = edges_and_versions - split;
      other.properties = properties - property_split;
      edges_and_versions = split;
      properties = property_split;

      return {split, property_split};
    }

    dst_t* get_single_block_pointer() {
      return start;
    }

    size_t get_edges_and_versions() {
      return edges_and_versions;
    }

    size_t get_property_count() {
      return properties;
    }

    size_t get_block_capacity() {
      return capacity;
    }

    dst_t get_max_edge() {
      if (1 < edges_and_versions && is_versioned(start[edges_and_versions - 2])) {
        return make_unversioned(start[edges_and_versions - 2]);
      } else {
        return start[edges_and_versions - 1];
      }
    }

    void update_skip_list_header(VSkipListHeader *h) {
      h->size = edges_and_versions;
      h->properties = properties;
      h->max = get_max_edge();
    }

    char *properties_start() {
      return end - properties * property_size;
    }

    /**
   * Start of the memory region
   */
    dst_t *start;

private:
    size_t capacity;

    /**
     * End of the memory region.
     */
    char* end;

    /**
     * The number of version records and edges.
     */

    size_t edges_and_versions;
    /**
     * The number of properties.
     * Equals the number of edges.
     */
    size_t properties;

    /**
     * The size in bytes of each property.
     */
    size_t property_size;

    size_t size() {
      return (end - (char*) start);
    };



    size_t insert_edge_and_version_by_shift(dst_t e, version_t version) {
      auto i = start + edges_and_versions - 1;
      for (; start <= i; i--) {
        if (start < i && is_versioned(*(i - 1))) {
          if (e < make_unversioned(*(i - 1))) {
            *(i + 2) = *i;
            i--;
            *(i + 2) = *i;
          } else {
            break;
          }
        } else if (e < make_unversioned(*i)) {
          *(i + 2) = *i;
        } else {
          break;
        }
      }

      i++;

      *i = make_versioned(e);
      *(i + 1) = inline_version(false, false, version);
      return i - start;
    }

    void insert_properties_by_shift(char *properties, size_t offset) {
      if (offset == 0) {
        memcpy(properties_start() - property_size, properties, property_size);
      } else {
        memmove(properties_start() - property_size, properties_start(), offset * property_size);
        memcpy(properties_start() + (offset - 1) * property_size, properties, property_size);
      }
    }

    size_t count_versions_before(size_t offset) {
      size_t versions = 0;
      for (uint i = 0; i < offset; i++) {
        if (is_versioned(start[i])) {
          versions += 1;
          i++; // Skip the version
        }
      }
      return versions;
    };
};



#endif //LIVE_GRAPH_TWO_EDGEBLOCK_H
