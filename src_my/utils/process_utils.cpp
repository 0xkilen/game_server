#include "process_utils.hpp"
#include "log.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <fstream>
#include <execinfo.h>
#include <boost/filesystem.hpp>

#define DEAMONIZED_ENV "NORADEAMONIZED"
#define PID_DIR "pids"
#define MAX_STACK_SIZE 256

namespace bfs = boost::filesystem;

namespace nora {

        void deamonize(char **argv) {
                if (!argv) {
                        exit(1);
                }

                char *envv = getenv(DEAMONIZED_ENV);
                if (envv && strcmp(envv, "TRUE")==0) {
                        return;
                }

                pid_t pid = fork();
                if (pid==((pid_t)-1)) {
                        exit(1);
                }
                if (pid!=((pid_t)0)) {
                        exit(0);
                }

                // child process
                if (close(0)) {
                        exit(1);
                } else {
                        if (open("/dev/null", O_RDONLY)!=0) {
                                exit(1);
                        }
                }

                setsid();
                pid = fork();
                if (pid==((pid_t)-1)) {
                        exit(1);
                }
                if (pid!=((pid_t)0)) {
                        exit(0);
                }

                if (putenv(strdup(DEAMONIZED_ENV "=TRUE"))) {
                        exit(1);
                }

                execvp(argv[0], argv);
                exit(1);
        }

        void write_pid_file(const string& file_name) {
                if (!bfs::exists(PID_DIR)) {
                        bfs::create_directory(PID_DIR);
                }
                ostringstream oss;
                oss << PID_DIR << "/" << file_name;
                ofstream pid_file(oss.str(), std::ios::out|std::ios::trunc);
                pid_file << getpid();
        }

        void write_stacktrace_to_file(const string& prefix) {
                std::stringstream ss;

                void *array[MAX_STACK_SIZE];
                size_t size;
                char **strings;
                size_t i;

                size = backtrace(array, MAX_STACK_SIZE);
                strings = backtrace_symbols(array, static_cast<int>(size));
                ss << "Obtained " << size << " stack frames\n";
                for (i = 0; i < size; i++) {
                        ss << strings[i] << "\n";
                }
                free(strings);

                ostringstream oss;
                oss << prefix << ".minicore." << getpid();
                ofstream minicore_file(oss.str(), std::ios::out|std::ios::trunc);
                minicore_file << ss.str();
        }

}
