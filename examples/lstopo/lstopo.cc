/*
 * Copyright (c) hwloc development team 2022
 * Modified by crazyguitar 2025
 *
 * This file has been modified from its original version.
 * Modifications include simplifying the original lstopo.
 *
 * Licensed under the BSD License.
 * You may obtain a copy of the License at https://opensource.org/licenses/BSD-3-Clause.
 */
#include <stdio.h>
#include <stdlib.h>
#include <hwloc.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

/*
 * ref: https://github.com/open-mpi/hwloc/blob/v2.7/include/private/misc.h#L433-L436
 */
#define for_each_child(child, parent) for(child = parent->first_child; child; child = child->next_sibling)
#define for_each_memory_child(child, parent) for(child = parent->memory_first_child; child; child = child->next_sibling)
#define for_each_io_child(child, parent) for(child = parent->io_first_child; child; child = child->next_sibling)
#define for_each_misc_child(child, parent) for(child = parent->misc_first_child; child; child = child->next_sibling)

/*
 * ref: https://github.com/open-mpi/hwloc/blob/v2.7/include/private/misc.h#L541-L544
 */
#define hwloc_memory_size_printf_value(_size, _verbose) \
  ((_size) < (10ULL<<20) || _verbose ? (((_size)>>9)+1)>>1 : (_size) < (10ULL<<30) ? (((_size)>>19)+1)>>1 : (((_size)>>29)+1)>>1)
#define hwloc_memory_size_printf_unit(_size, _verbose) \
  ((_size) < (10ULL<<20) || _verbose ? "KB" : (_size) < (10ULL<<30) ? "MB" : "GB")


/*
 * ref: https://github.com/open-mpi/hwloc/blob/v2.7/utils/lstopo/lstopo-text.c#L24-L25
 */
#define indent(output, i) \
  fprintf (output, "%*s", (int) i, "");

/*
 * ref: https://github.com/open-mpi/hwloc/blob/b56eebaa847cc4bdb783a4f6a5198703e64d1d6f/hwloc/pci-common.c#L939
 */
const char *
hwloc_pci_class_string(unsigned short class_id)
{
  /* See https://pci-ids.ucw.cz/read/PD/ */
  switch ((class_id & 0xff00) >> 8) {
    case 0x00:
      switch (class_id) {
	case 0x0001: return "VGA";
      }
      break;
    case 0x01:
      switch (class_id) {
	case 0x0100: return "SCSI";
	case 0x0101: return "IDE";
	case 0x0102: return "Floppy";
	case 0x0103: return "IPI";
	case 0x0104: return "RAID";
	case 0x0105: return "ATA";
	case 0x0106: return "SATA";
	case 0x0107: return "SAS";
	case 0x0108: return "NVMExp";
      }
      return "Storage";
    case 0x02:
      switch (class_id) {
	case 0x0200: return "Ethernet";
	case 0x0201: return "TokenRing";
	case 0x0202: return "FDDI";
	case 0x0203: return "ATM";
	case 0x0204: return "ISDN";
	case 0x0205: return "WorldFip";
	case 0x0206: return "PICMG";
	case 0x0207: return "InfiniBand";
	case 0x0208: return "Fabric";
      }
      return "Network";
    case 0x03:
      switch (class_id) {
	case 0x0300: return "VGA";
	case 0x0301: return "XGA";
	case 0x0302: return "3D";
      }
      return "Display";
    case 0x04:
      switch (class_id) {
	case 0x0400: return "MultimediaVideo";
	case 0x0401: return "MultimediaAudio";
	case 0x0402: return "Telephony";
	case 0x0403: return "AudioDevice";
      }
      return "Multimedia";
    case 0x05:
      switch (class_id) {
	case 0x0500: return "RAM";
	case 0x0501: return "Flash";
        case 0x0502: return "CXLMem";
      }
      return "Memory";
    case 0x06:
      switch (class_id) {
	case 0x0600: return "HostBridge";
	case 0x0601: return "ISABridge";
	case 0x0602: return "EISABridge";
	case 0x0603: return "MicroChannelBridge";
	case 0x0604: return "PCIBridge";
	case 0x0605: return "PCMCIABridge";
	case 0x0606: return "NubusBridge";
	case 0x0607: return "CardBusBridge";
	case 0x0608: return "RACEwayBridge";
	case 0x0609: return "SemiTransparentPCIBridge";
	case 0x060a: return "InfiniBandPCIHostBridge";
      }
      return "Bridge";
    case 0x07:
      switch (class_id) {
	case 0x0700: return "Serial";
	case 0x0701: return "Parallel";
	case 0x0702: return "MultiportSerial";
	case 0x0703: return "Model";
	case 0x0704: return "GPIB";
	case 0x0705: return "SmartCard";
      }
      return "Communication";
    case 0x08:
      switch (class_id) {
	case 0x0800: return "PIC";
	case 0x0801: return "DMA";
	case 0x0802: return "Timer";
	case 0x0803: return "RTC";
	case 0x0804: return "PCIHotPlug";
	case 0x0805: return "SDHost";
	case 0x0806: return "IOMMU";
      }
      return "SystemPeripheral";
    case 0x09:
      switch (class_id) {
	case 0x0900: return "Keyboard";
	case 0x0901: return "DigitizerPen";
	case 0x0902: return "Mouse";
	case 0x0903: return "Scanern";
	case 0x0904: return "Gameport";
      }
      return "Input";
    case 0x0a:
      return "DockingStation";
    case 0x0b:
      switch (class_id) {
	case 0x0b00: return "386";
	case 0x0b01: return "486";
	case 0x0b02: return "Pentium";
/* 0x0b03 and 0x0b04 might be Pentium and P6 ? */
	case 0x0b10: return "Alpha";
	case 0x0b20: return "PowerPC";
	case 0x0b30: return "MIPS";
	case 0x0b40: return "Co-Processor";
      }
      return "Processor";
    case 0x0c:
      switch (class_id) {
	case 0x0c00: return "FireWire";
	case 0x0c01: return "ACCESS";
	case 0x0c02: return "SSA";
	case 0x0c03: return "USB";
	case 0x0c04: return "FibreChannel";
	case 0x0c05: return "SMBus";
	case 0x0c06: return "InfiniBand";
	case 0x0c07: return "IPMI-SMIC";
	case 0x0c08: return "SERCOS";
	case 0x0c09: return "CANBUS";
      }
      return "SerialBus";
    case 0x0d:
      switch (class_id) {
	case 0x0d00: return "IRDA";
	case 0x0d01: return "ConsumerIR";
	case 0x0d10: return "RF";
	case 0x0d11: return "Bluetooth";
	case 0x0d12: return "Broadband";
	case 0x0d20: return "802.1a";
	case 0x0d21: return "802.1b";
      }
      return "Wireless";
    case 0x0e:
      switch (class_id) {
	case 0x0e00: return "I2O";
      }
      return "Intelligent";
    case 0x0f:
      return "Satellite";
    case 0x10:
      return "Encryption";
    case 0x11:
      return "SignalProcessing";
    case 0x12:
      return "ProcessingAccelerator";
    case 0x13:
      return "Instrumentation";
    case 0x40:
      return "Co-Processor";
  }
  return "Other";
}

inline static int dump_busid(char *text, size_t textlen, hwloc_obj_t firstobj) {
   char domain[10] = "";
   return snprintf(text, textlen, "%s%02x:%02x.%01x",
     domain,
     firstobj->attr->pcidev.bus,
     firstobj->attr->pcidev.dev,
     firstobj->attr->pcidev.func
   );
}

static void dump_node(hwloc_obj_t l) {
  char busidstr[32] = {0};
  char lidxstr[32] = {0};

  snprintf(lidxstr, sizeof(lidxstr), "L#%u", l->logical_index);

  if (l->type == HWLOC_OBJ_PCI_DEVICE) {
    dump_busid(busidstr, sizeof(busidstr), l);
  }

  char type[64], attr[64] = {0}, phys[32] = {0};
  hwloc_obj_type_snprintf (type, sizeof(type), l, -1);
  if (l->subtype) {
    fprintf(stdout, "%s(%s)", type, l->subtype);
  } else {
    fprintf(stdout, "%s", type);
  }

  if (l->depth != 0 && ((hwloc_obj_type_is_normal(l->type) || hwloc_obj_type_is_memory(l->type)))) {
    fprintf(stdout, " %s", lidxstr);
  }

  if (l->type == HWLOC_OBJ_PCI_DEVICE) {
    fprintf(stdout, " %s (%s)", busidstr, hwloc_pci_class_string(l->attr->pcidev.class_id));
  }

  hwloc_obj_attr_snprintf (attr, 64, l, " ", 0);
  if (*phys || *attr) {
    fprintf(stdout, " (");
    if (*phys)
      fprintf(stdout, "%s", phys);
    if (*phys && *attr)
      fprintf(stdout, " ");
    if (*attr)
      fprintf(stdout, "%s", attr);
    fprintf(stdout, ")");
  }
  if (!l->parent && l->total_memory) {
    fprintf(stdout, " (%lu%s total)",
	  (unsigned long) hwloc_memory_size_printf_value(l->total_memory, 0),
	  hwloc_memory_size_printf_unit(l->total_memory, 0));
  }
  if (l->name
    and l->type == HWLOC_OBJ_OS_DEVICE
    and l->type != HWLOC_OBJ_MISC
    and l->type != HWLOC_OBJ_GROUP
  ) {
      fprintf(stdout, " \"%s\"", l->name);
  }

}

static void dump(
  hwloc_obj_t l,
  hwloc_obj_t parent,
  int i
) {
  hwloc_obj_t child;
  if (parent
      and parent->arity == 1
      and !parent->memory_arity
      and !parent->io_arity
      and !parent->misc_arity
      and l->cpuset
      and parent->cpuset
      and hwloc_bitmap_isequal(l->cpuset, parent->cpuset)
  ) {
    fprintf(stdout, " + ");
  } else {
    if (parent) fprintf(stdout, "\n");
    indent(stdout, 2*i);
    i++;
  }
  dump_node(l);
  for_each_memory_child(child, l) {
    if (child->type != HWLOC_OBJ_PU) dump(child, l, i);
  }
  for_each_child(child, l) {
    if (child->type != HWLOC_OBJ_PU) dump(child, l, i);
  }
  for_each_io_child(child, l) {
    dump(child, l, i);
  }
  for_each_misc_child(child, l) {
    dump(child, l, i);
  }
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
