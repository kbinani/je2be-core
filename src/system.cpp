#include "_system.hpp"

#if __has_include(<windows.h>) && __has_include(<sysinfoapi.h>)
#define NOMINMAX
#include <windows.h>

#include <sysinfoapi.h>
#elif __has_include(<unistd.h>)
#include <unistd.h>
#endif

#if __has_include(<mach/mach.h>)
#include <mach/mach.h>
#include <mach/mach_host.h>
#endif

namespace je2be {

u64 System::GetInstalledMemory() {
#if __has_include(<windows.h>) && __has_include(<sysinfoapi.h>)
  ULONGLONG ret = 0;
  if (GetPhysicallyInstalledSystemMemory(&ret)) {
    return ret;
  } else {
    return 0;
  }
#elif __has_include(<mach/mach.h>)
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
  vm_statistics_data_t vmstat;
  if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS) {
    return 0;
  }
  u64 total = (u64)vmstat.wire_count + (u64)vmstat.active_count + (u64)vmstat.inactive_count + (u64)vmstat.free_count;
  return (u64)total * u64(vm_page_size);
#elif __has_include(<unistd.h>)
  long pages = sysconf(_SC_PHYS_PAGES);
  long pageSize = sysconf(_SC_PAGE_SIZE);
  if (pages < 0 || pageSize < 0) {
    return 0;
  }
  return u64(pages) * u64(pageSize);
#else
  return 0;
#endif
}

u64 System::GetAvailableMemory() {
#if __has_include(<windows.h>) && __has_include(<sysinfoapi.h>)
  MEMORYSTATUSEX st;
  st.dwLength = sizeof(st);
  if (GlobalMemoryStatusEx(&st)) {
    return st.ullAvailPhys;
  } else {
    return 0;
  }
#elif __has_include(<mach/mach.h>)
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
  vm_statistics_data_t vmstat;
  if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS) {
    return 0;
  }
  return (u64)vmstat.free_count * u64(vm_page_size);
#elif __has_include(<unistd.h>) && defined(_SC_AVPHYS_PAGES)
  long pages = sysconf(_SC_AVPHYS_PAGES);
  long pageSize = sysconf(_SC_PAGE_SIZE);
  if (pages < 0 || pageSize < 0) {
    return 0;
  }
  return u64(pages) * u64(pageSize);
#else
  return 0;
#endif
}

} // namespace je2be
