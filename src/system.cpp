#include "_system.hpp"

#if __has_include(<windows.h>) && __has_include(<sysinfoapi.h>)
#define NOMINMAX
#include <windows.h>

#include <sysinfoapi.h>
#elif __has_include(<unistd.h>)
#include <unistd.h>
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
#elif __has_include(<unistd.h>)
  long pages = sysconf(_SC_PHYS_PAGES);
  long pageSize = sysconf(_SC_PAGE_SIZE);
  if (pages < 0 || pageSize < 0) {
    return 0;
  }
  return pages * pageSize;
#else
  return 0;
#endif
}

} // namespace je2be
