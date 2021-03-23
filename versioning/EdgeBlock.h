//
// Created by per on 23.03.21.
//

#ifndef LIVE_GRAPH_TWO_EDGEBLOCK_H
#define LIVE_GRAPH_TWO_EDGEBLOCK_H

#include <utils/utils.h>

#define MIN_BLOCK_SIZE 2u


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
            : start(start), capacity(capacity), end(start + capacity * sizeof(dst_t)), edges_and_versions(edges_and_versions), properties(properties),
              property_size(property_size) {};


    static EdgeBlock from_vskip_list_header(VSkipListHeader* header);
    static EdgeBlock from_single_block(dst_t* start, size_t edges_and_versions, size_t properties, size_t property_size) {
      return EdgeBlock(start, max(MIN_BLOCK_SIZE, round_up_power_of_two(edges_and_versions)), edges_and_versions, properties, property_size);
    }

    // TODO move methods definitions to cpp file

    bool has_space_to_insert_edge() {
      return edges_and_versions + 2 <= get_block_capacity();
      // TODO needs to take properties into account
//      int remaining_space = size() - edges_and_versions * sizeof(dst_t) - properties * property_size;
      // One edge, one version record and the size of the properties.
//      int required_space =  2 * sizeof(dst_t) - property_size;
//      return 0 <= remaining_space - required_space;
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
      size_t offset = insert_edge_and_version_by_shift(e, version);
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

      // Copy properties into new block.
      memcpy((char*) other.end - property_split * property_size, (char*) end - property_split * property_size, property_split * property_size);
      // Move properties in existing block
      memmove((char*) end - (properties - property_split) * property_size, (char*) properties_start(), (properties - property_split) * properties);

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

    size_t assert_block_consistency(version_t min_version) {
      dst_t before = 0;
      auto versions = 0;
      for (auto i = start; i < start + edges_and_versions; i++) {
        auto e = *i;
        if (is_versioned(e)) {
          versions++;
          assert(before <= make_unversioned(e));
          before = make_unversioned(e);
          assert(i + 1 < end);
          assert(min_version <= timestamp(*(i + 1)));
          i += 1; // Jump over version
        } else {
          assert(before <= e);
          before = e;
        }
      }
      return versions;
      // TODO needs to assert properties
    };


    dst_t get_max_edge() {
      if (1 < edges_and_versions && is_versioned(start[edges_and_versions - 2])) {
        return make_unversioned(start[edges_and_versions - 2]);
      } else {
        return start[edges_and_versions - 1];
      }
    }

private:

    /**
     * Start of the memory region
     */
    dst_t *start;

    size_t capacity;

    /**
     * End of the memory region.
     */
    dst_t *end;

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
      return (end - start) * sizeof(dst_t);
    };

    char *properties_start() {
      return (char *) end - properties * property_size;
    }

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
      return i - start;  // TODO offset computation incorrect, does not take versions into account
    }

    void insert_properties_by_shift(char *properties, size_t offset) {
      // TODO implement
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
