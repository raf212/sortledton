
#include "gtest/gtest.h"

#include <string>
#include <algorithm>
#include <gmock/gmock-matchers.h>
#include <data-src/SortedCSRDataSource.h>
#include <data-structures/HashSetSimulatorAdjacencyList.h>
#include <data-structures/adjacency-lists/FilteredVectorIterator.h>

namespace testing {

    using namespace std;

    class CountingEmptyFilteredVectorIterator : public FilteredVectorIterator {
    public:
        bool has_next() {
          if (data == nullptr) {
            return false;
          }
          while (data < end && *data == empty) {
            if (print_list) {
              cout << ".";
            }
            empty_spots++;
            data++;
          }

          if (data < end && print_list) {
              cout << "x";
          } else {
            cout << endl;
          }
          return data < end;
        };

        uint empty_spots = 0;
        bool print_list = true;
    };


    float get_fill_rate(HashSetSimulatorAdjacencyList& ds) {
      cout << "new ds" << endl;
      uint full_spots = 0;
      auto ns = CountingEmptyFilteredVectorIterator();
      for (int i = 0; i < 100; i++) {
        if (3 < i) {
          ns.print_list = false;
        }
        ds.neighbourhood(i, ns);

        while (ns.has_next()) {
          ns.next();
          full_spots++;
        }
      }
      return (float) full_spots / (float) (ns.empty_spots + full_spots);
    }

    TEST(HashSetSimulatorTestSuite, validate_empty_spot_distribution) {
      const string HIGGS_DATASET = "/space/fuchs/shared/graph-two-datasets/higgs/base.csr";

      SortedCSRDataSource out;
      out.read_from_binary_file(HIGGS_DATASET);

      auto h10 = HashSetSimulatorAdjacencyList(0.1);
      h10.bulkload(out);

      EXPECT_THAT(get_fill_rate(h10), FloatEq(0.1));

      auto h50 = HashSetSimulatorAdjacencyList(0.5);
      h50.bulkload(out);

      EXPECT_THAT(get_fill_rate(h50), FloatEq(0.5));

      auto h90 = HashSetSimulatorAdjacencyList(0.9);
      h90.bulkload(out);

      EXPECT_THAT(get_fill_rate(h90), FloatNear(0.9, 0.005));
    }
}
