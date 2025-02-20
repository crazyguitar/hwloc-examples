#include <stdio.h>
#include <stdlib.h>
#include <hwloc.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

static void dump(
  hwloc_obj_t l,
  hwloc_obj_t parent,
  int i
) {

}

int main(int argc, char *argv[]) {
  int err;
  hwloc_topology_t topology;
  unsigned long flags = HWLOC_TOPOLOGY_FLAG_IMPORT_SUPPORT;

  err = hwloc_topology_init (&topology);
  if (err) {
    fprintf(stderr, "hwloc_topology_init fail. error: %s", strerror(errno));
    goto end;
  }
  hwloc_topology_set_all_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_ALL);
  hwloc_topology_set_io_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_IMPORTANT);
  err = hwloc_topology_set_flags(topology, flags);
  if (err) {
    fprintf(stderr, "hwloc_topology_set_flags fail. error: %s", strerror(errno));
    goto end;
  }

  err = hwloc_topology_load(topology);
  if (err) {
    fprintf(stderr, "hwloc_topology_load fail. error: %s", strerror(errno));
    goto end;
  }

  dump(hwloc_get_root_obj(topology), nullptr, 0);
  fprintf(stdout, "\n");
end:
  hwloc_topology_destroy(topology);
}
