#include <iostream>
#include <experiments/Driver.h>
#include "experiments/Configuration.h"

int main(int argc, char** argv) {
  Config config { };
  config.initialize(argc, argv);

  Driver driver(config);
  driver.run();

  cout << "Done" << endl;
  return 0;
}
