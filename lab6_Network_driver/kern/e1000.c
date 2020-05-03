#include <inc/assert.h>
#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile void *e1000_bar_va;

int e1000_82540em_attach(struct pci_func *pcif) {
    cprintf("e1000_82540em_attach() called\n");
    // enable device and negotiate resouce, update pcif
    pci_func_enable(pcif);

    // create x86 mmio memory mapping, make sure status reg is correct
    e1000_bar_va = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    assert((*(int *)E1000MEM(E1000_82540EM_STATUS_ADDR)) == 0x80080783);

    return 0;
}