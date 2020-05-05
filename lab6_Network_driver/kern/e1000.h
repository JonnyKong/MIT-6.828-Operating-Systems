#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

// pci setup
#define E1000_VENDOR_ID     0x8086
#define E1000_DEVICE_ID     0x100E

// pci mmio
#define E1000MEM(offset) (e1000_bar_va + offset)
#define E1000_STATUS_ADDR   0x8
#define E1000_RCTL_ADDR     0x0100
#define E1000_ICS_ADDR      0x00c8
#define E1000_IMS_ADDR      0x00d0
#define E1000_RDBAL_ADDR    0x2800
#define E1000_RDBAH_ADDR    0x2804
#define E1000_RDLEN_ADDR    0x2808
#define E1000_RDH_ADDR      0x2810
#define E1000_RDT_ADDR      0x2818
#define E1000_TDBAL_ADDR    0x3800
#define E1000_TDBAH_ADDR    0x3804
#define E1000_TDLEN_ADDR    0x3808
#define E1000_TDH_ADDR      0x3810
#define E1000_TDT_ADDR      0x3818
#define E1000_TCTL_ADDR     0x0400
#define E1000_TIPG_ADDR     0x0410
#define E1000_MTA_ADDR      0x5200
#define E1000_RAL_ADDR      0x5400
#define E1000_RAH_ADDR      0x5404

// field masks
#define E1000_TXD_CMD_EOP   0x1
#define E1000_TXD_CMD_RS    0x8
#define E1000_TXD_STATUS_DD 0x1
#define E1000_RXD_STATUS_DD 0x1
#define E1000_RAH_AV        0x80000000

// return codes
#define E1000_TX_RETRY  1
#define E1000_RX_RETRY  2

#define RXPKT_SIZE      1518


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

// legacy receive descriptor
struct e1000_rx_desc
{
    uint64_t addr;
    uint16_t length;
    uint16_t cksm;
    uint8_t status;
    uint8_t errors;
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

// receive descriptor length (RDL)
struct e1000_rdlen
{
    uint32_t zero:  7;
    uint32_t len:   13;
    uint32_t rsv:   12;
};

// receive descriptor head (RDH)
struct e1000_rdh
{
    uint16_t rdh;
    uint16_t rsv;
};

// receive descriptor tail (RDT)
struct e1000_rdt
{
    uint16_t rdt;
    uint16_t rsv;
};

// receive control register (RCTL)
struct e1000_rctl
{
    uint32_t rsv1:  1;
    uint32_t en:    1;
    uint32_t sbp:   1;
    uint32_t upe:   1;
    uint32_t mpe:   1;
    uint32_t lpe:   1;
    uint32_t lbm:   2;
    uint32_t rdmts: 2;
    uint32_t rsv2:  2;
    uint32_t mo:    2;
    uint32_t rsv3:  1;
    uint32_t bam:   1;
    uint32_t bsize: 2;
    uint32_t vfe:   1;
    uint32_t cfien: 1;
    uint32_t cfi:   1;
    uint32_t rsv4:  1;
    uint32_t dpf:   1;
    uint32_t pmcf:  1;
    uint32_t rsv5:  1;
    uint32_t bsex:  1;
    uint32_t secrc: 1;
    uint32_t rsv6:  5;
};

int e1000_attach(struct pci_func *pcif);
static void e1000_tx_init();
static void e1000_rx_init();
int e1000_tx(void *data, size_t len);
int e1000_rx(void *data, size_t *len);
static void set_macaddr(uint32_t *ral, uint32_t *rah, uint8_t *mac);

#endif  // SOL >= 6