#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

// pci setup
#define E1000_VENDOR_ID     0x8086
#define E1000_DEVICE_ID     0x100E

// pci mmio
#define E1000MEM(offset) (e1000_bar_va + offset)
#define E1000_STATUS_ADDR   0x8
#define E1000_TDBAL_ADDR    0x3800
#define E1000_TDBAH_ADDR    0x3804
#define E1000_TDLEN_ADDR    0x3808
#define E1000_TDH_ADDR      0x3810
#define E1000_TDT_ADDR      0x3818
#define E1000_TCTL_ADDR     0x0400
#define E1000_TIPG_ADDR     0x0410

// tx descriptor field masks
#define E1000_TXD_CMD_EOP   0x0
#define E1000_TXD_CMD_RS    0x3
#define E1000_TXD_STATUS_DD 0x1

// return codes
#define E1000_TX_RETRY  1
#define E1000_RX_RETRY  2


// legacy transmit descriptor
struct e1000_tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
} __attribute__((packed));

// transmit descriptor length (TDL)
struct e1000_tdlen
{
    uint32_t zero: 7;
    uint32_t len: 13;
    uint32_t rsv: 12;
};

// transmit descriptor head (TDH)
struct e1000_tdh
{
    uint16_t tdh;
    uint16_t rsv;
};

// transmit descriptor tail (TDT)
struct e1000_tdt
{
    uint16_t tdt;
    uint16_t rsv;
};

// transmit control register (TCTL)
struct e1000_tctl
{
    uint32_t rsv1:      1;
    uint32_t en:        1;
    uint32_t rsv2:      1;
    uint32_t psp:       1;
    uint32_t ct:        8;
    uint32_t cold:      10;
    uint32_t swxoff:    1;
    uint32_t rsv3:      1;
    uint32_t rtlc:      1;
    uint32_t nrtu:      1;
    uint32_t rsv4:      6;
};

// transmit inter packet gap register (TIPG)
struct e1000_tipg
{
    uint32_t ipgt:  10;
    uint32_t ipgr1: 10;
    uint32_t ipgr2: 10;
    uint32_t rsv:   2;
};


int e1000_attach(struct pci_func *pcif);
static void e1000_tx_init();
static void e1000_rx_init();
int e1000_tx(void *data, size_t len);

#endif  // SOL >= 6