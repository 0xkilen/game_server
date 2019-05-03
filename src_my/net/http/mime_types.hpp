#pragma once

#include <string>

using namespace std;

namespace nora {
        namespace net {
                namespace http {

                        // Convert a file extension into a MIME type.
                        string extension_to_type(const string& extension);

                }
        }
}
