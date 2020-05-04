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
    }

    // set transmit descriptor length to the size of the descriptor ring
    struct e1000_tdlen *tdlen = (struct e1000_tdlen *)E1000MEM(E1000_TDLEN_ADDR);
    tdlen->len = TXDESC_CNT;

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

    int ret;
    ret = e1000_tx((void *)e1000_tx_desc_arr, 32);
    ret = e1000_tx((void *)e1000_tx_desc_arr, 32);
    cprintf("e1000_tx() returned %d\n", ret);
}

static void e1000_rx_init()
{

}

int e1000_tx(void *data, size_t len) 
{
    struct e1000_tdt *tdt = (struct e1000_tdt *)E1000MEM(E1000_TDT_ADDR);
    uint32_t tail = tdt->tdt;
    
    // if next descriptor not free, let caller retry
    if (!(e1000_tx_desc_arr[tail].status & E1000_TXD_STATUS_DD)) 
        return -E1000_TX_RETRY;
    
    memcpy(e1000_tx_buf[tail], data, len);
    e1000_tx_desc_arr[tail].length = len;
    e1000_tx_desc_arr[tail].status &= ~E1000_TXD_STATUS_DD;
    e1000_tx_desc_arr[tail].cmd |= (E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS);

    tdt->tdt = (tail + 1) % TXDESC_CNT;
    return 0;
}