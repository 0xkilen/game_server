#pragma once

#include <string>

using namespace std;

namespace nora {

        void deamonize(char **argv);
        void write_pid_file(const string& file_name);
        void write_stacktrace_to_file(const string& prefix);

}
