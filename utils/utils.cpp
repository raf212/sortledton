//
// Created by per on 31.08.20.
//

#include <sys/stat.h>
#include "utils.h"

bool file_exists (const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

/**
 * https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
 * @param fullString
 * @param ending
 * @return
 */
bool endsWith (string const &fullString, string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}
