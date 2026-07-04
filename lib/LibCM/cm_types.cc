#include "cm_types.hh"
#include <sstream>
#include <iomanip>

#ifdef DEBUG_ALLOCATIONS

CMSMutex allocator_mutex;
std::map<void *, size_t> allocations;
std::map<char, size_t> allocated_total;
std::ofstream allocation_debug_output("/dev/shm/cm-debug");

void print_allocation_message()
{
    allocation_debug_output << "====================================\n";
    for (std::map<char, size_t>::iterator it = allocated_total.begin(); it != allocated_total.end(); ++it)
        allocation_debug_output << "allocator: " << it->first
                                << " = " << it->second << std::endl;
}

#endif // DEBUG_ALLOCATIONS
