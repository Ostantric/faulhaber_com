#include <fstream>
#include <sstream>

#include "logging.h"
#include "util.h"

namespace util {

util::ErrorT GetFileContents(const char *filename, std::string &out_string) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (!in || !in.good()) {
    LogWrite(ERROR, "File '%s' does not exist or has problems", filename);
    return util::Error::INVALID;
  }

  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();
  out_string = contents;
  return util::Error::OK;
}

}
