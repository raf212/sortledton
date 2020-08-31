#include <iostream>
#include "experiments/Configuration.h"

int main(int argc, char** argv) {
  Configuration::get_config().initialize(argc, argv);



  std::cout << "Hello, World!" << std::endl;
  return 0;
}
