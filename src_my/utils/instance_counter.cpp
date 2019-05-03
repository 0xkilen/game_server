#include "instance_counter.hpp"
#include <mutex>

using namespace std;
using namespace chrono;

namespace nora {

        mutex instance_counter::lock_;
        bool instance_counter::live_ = false;

}
