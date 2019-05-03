#include "mime_types.hpp"

namespace nora {
        namespace net {
                namespace http {

                        struct mapping {
                                const char* extension;
                                const char* mime_type;
                        } mappings[] = {
                                { "gif", "image/gif" },
                                { "htm", "text/html" },
                                { "html", "text/html" },
                                { "jpg", "image/jpeg" },
                                { "png", "image/png" }
                        };

                        string extension_to_type(const string& extension) {
                                for (mapping m: mappings)
                                {
                                        if (m.extension == extension)
                                        {
                                                return m.mime_type;
                                        }
                                }

                                return "text/plain";
                        }

                }
        }
}
