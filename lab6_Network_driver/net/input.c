#include "ns.h"
#include "kern/e1000.h"
#include "inc/lib.h"

extern union Nsipc nsipcbuf;

void
sleep(int msec)
{
	int now = sys_time_msec();
	int end = now + msec;
	while (sys_time_msec() < end)
		sys_yield();
}

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	uint8_t buf[RXPKT_SIZE];
	size_t len;

	while (true) {
		if (sys_net_recv(buf, &len) < 0)
			continue;
		
		memcpy(nsipcbuf.pkt.jp_data, buf, len);
		nsipcbuf.pkt.jp_len = len;
		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P | PTE_U | PTE_W);

		sleep(50);
	}
}
