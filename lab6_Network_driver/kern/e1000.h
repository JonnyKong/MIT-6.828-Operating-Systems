#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define E1000_82540EM_VENDOR_ID     0x8086
#define E1000_82540EM_DEVICE_ID     0x100E

#define E1000MEM(offset) (e1000_bar_va + offset)
#define E1000_82540EM_STATUS_ADDR   8

int e1000_82540em_attach(struct pci_func *pcif);

#endif  // SOL >= 6