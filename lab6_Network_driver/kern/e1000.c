#include <inc/assert.h>
#include <inc/string.h>
#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile void *e1000_bar_va;

// tx
#define TXDESC_CNT      32
#define TXPKT_SIZE      1518
struct e1000_tx_desc e1000_tx_desc_arr[TXDESC_CNT] __attribute__ ((aligned (16)));
char e1000_tx_buf[TXDESC_CNT][TXPKT_SIZE];

// rx
#define RXDESC_CNT      128
// #define RXPKT_SIZE      1518
struct e1000_rx_desc e1000_rx_desc_arr[RXDESC_CNT] __attribute__ ((aligned (16)));
char e1000_rx_buf[RXDESC_CNT][RXPKT_SIZE];

// mmio doesn't always preserve incremental write semantics. When using C bitfields, first
// write to a temporary variable incrementally, then write this variable to mmio
static uint32_t mmio_buf;

int e1000_attach(struct pci_func *pcif)
{
    // enable device and negotiate resouce, update pcif
    pci_func_enable(pcif);

    // create x86 mmio memory mapping, make sure status reg is correct
    e1000_bar_va = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    assert((*(int *)E1000MEM(E1000_STATUS_ADDR)) == 0x80080783);

    e1000_tx_init();
    e1000_rx_init();

    return 0;
}

static void e1000_tx_init()
{
    for (int i = 0; i < TXDESC_CNT; ++i) {
        e1000_tx_desc_arr[i].addr = PADDR(e1000_tx_buf[i]);
        e1000_tx_desc_arr[i].status |= E1000_TXD_STATUS_DD;
        // e1000_tx_desc_arr[i].cmd |= E1000_TXD_CMD_RS;
    }

    // set transmit descriptor length to the size of the descriptor ring
    struct e1000_tdlen *tdlen = (struct e1000_tdlen *)E1000MEM(E1000_TDLEN_ADDR);
    tdlen->len = TXDESC_CNT / 8;

    // set transmit descriptor base address with address of the region
    uint32_t *tdbal = (uint32_t *)E1000MEM(E1000_TDBAL_ADDR);
    uint32_t *tdbah = (uint32_t *)E1000MEM(E1000_TDBAH_ADDR);
    *tdbal = PADDR(e1000_tx_desc_arr);
    *tdbah = 0; // used for 64-bits only

    // transmit descriptor head and tail are initialized to 0
    struct e1000_tdh *tdh = (struct e1000_tdh *)E1000MEM(E1000_TDH_ADDR);
    tdh->tdh = 0;
    struct e1000_tdt *tdt = (struct e1000_tdt *)E1000MEM(E1000_TDT_ADDR);
    tdt->tdt = 0;

    // init transmit control register, see manual 14.5
    struct e1000_tctl *tctl = (struct e1000_tctl *)E1000MEM(E1000_TCTL_ADDR);
    tctl->en = 1;
    tctl->psp = 1;
    tctl->ct = 0x10;
    tctl->cold = 0x40;

    // init transmit IPG register, see manual 14.5
    struct e1000_tipg *tipg = (struct e1000_tipg *)E1000MEM(E1000_TIPG_ADDR);
    tipg->ipgt = 10;
    tipg->ipgr1 = 4;
    tipg->ipgr2 = 6;
}

static void e1000_rx_init()
{
    // set receive address register with hardcoded qemu MAC address
    uint8_t qemu_macaddr[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
    uint32_t *ral = (uint32_t *)E1000MEM(E1000_RAL_ADDR);
    uint32_t *rah = (uint32_t *)E1000MEM(E1000_RAH_ADDR);
    set_macaddr(ral, rah, qemu_macaddr);

    // // init the multicast table array to 0
    // uint32_t *mta = (uint32_t *)E1000MEM(E1000_MTA_ADDR);
    // for (int i = 0; i < 128; ++i)
    //     mta[i] = 0;

    // set interrupt mask set/read to enable interrupt 
    uint32_t *ims = (uint32_t *)E1000MEM(E1000_IMS_ADDR);
    *ims = 0;
    // uint32_t *ics = (uint32_t *)E1000MEM(E1000_ICS_ADDR);
    // *ics = 0;

    // set the receiver descriptor list
    for (int i = 0; i < RXDESC_CNT; ++i) {
        e1000_rx_desc_arr[i].addr = PADDR(e1000_rx_buf[i]);
    }
    uint32_t *rdbal = (uint32_t *)E1000MEM(E1000_RDBAL_ADDR);
    uint32_t *rdbah = (uint32_t *)E1000MEM(E1000_RDBAH_ADDR);
    *rdbal = PADDR(e1000_rx_desc_arr);
    *rdbah = 0;

    // set the receive descriptor length
    struct e1000_rdlen *rdlen = (struct e1000_rdlen *)E1000MEM(E1000_RDLEN_ADDR);
    rdlen->len = RXDESC_CNT / 8;

    // set the receive descriptor head and tail
    struct e1000_rdh *rdh = (struct e1000_rdh *)E1000MEM(E1000_RDH_ADDR);
    struct e1000_rdt *rdt = (struct e1000_rdt *)E1000MEM(E1000_RDT_ADDR);
    rdh->rdh = 0;
    rdt->rdt = RXDESC_CNT - 1;

    // set the receive control register
    // uint32_t *rctl = (uint32_t *)E1000MEM(E1000_RCTL_ADDR);
    // *rctl = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC;
    struct e1000_rctl *rctl = (struct e1000_rctl *)(&mmio_buf);
    rctl->en = 1;
    rctl->bam = 1;
    rctl->secrc = 1;
    *(uint32_t *)E1000MEM(E1000_RCTL_ADDR) = mmio_buf;
}

int e1000_tx(void *data, size_t len) 
{
    struct e1000_tdt *tdt = (struct e1000_tdt *)E1000MEM(E1000_TDT_ADDR);
    uint32_t tail = tdt->tdt;
    
    // if next descriptor not free (buffer is full), let caller retry
    if (!(e1000_tx_desc_arr[tail].status & E1000_TXD_STATUS_DD)) 
        return -E1000_TX_RETRY;
    
    memcpy(e1000_tx_buf[tail], data, len);
    e1000_tx_desc_arr[tail].length = len;
    e1000_tx_desc_arr[tail].status &= ~E1000_TXD_STATUS_DD;
    e1000_tx_desc_arr[tail].cmd |= (E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS);

    tdt->tdt = (tail + 1) % TXDESC_CNT;
    return 0;
}

int e1000_rx(void *data, size_t *len)
{
    struct e1000_rdt *rdt = (struct e1000_rdt *)E1000MEM(E1000_RDT_ADDR);
    // uint32_t tail = rdt->rdt;
    uint32_t next = (rdt->rdt + 1) % RXDESC_CNT;

    // if next descriptor free (buffer empty), let caller retry
    if (!(e1000_rx_desc_arr[next].status & E1000_RXD_STATUS_DD)) {
        return -E1000_RX_RETRY;
    } else if (e1000_rx_desc_arr[next].errors) {
        cprintf("e1000_rx() error\n");
        return -E1000_RX_RETRY;
    }
    
    cprintf("e1000_rx(): received content\n");
    *len = e1000_rx_desc_arr[next].length;
    memcpy(data, e1000_rx_buf[next], *len);
    
    // rdt->rdt = (tail + 1) % RXDESC_CNT;
    rdt->rdt = next;
    return 0;
}

// write MAC address to ral and rah
static void set_macaddr(uint32_t *ral, uint32_t *rah, uint8_t *mac)
{
    // the following is incorrect: can't do incremental write in x86 mmio
    // for (int i = 0; i < 4; ++i)
    //     ((uint8_t *)ral)[i] = mac[i]; 
    // for (int i = 0; i < 2; ++i)
    //     ((uint8_t *)rah)[i] = mac[i + 4];

    uint32_t ral_val = 0, rah_val = 0;
    for (int i = 0; i < 4; ++i)
        ral_val |= (uint32_t)(mac[i]) << (i * 8);
    for (int i = 0; i < 2; ++i)
        rah_val |= (uint32_t)(mac[i + 4]) << (i * 8);
    *ral = ral_val;
    *rah = rah_val | E1000_RAH_AV;
}