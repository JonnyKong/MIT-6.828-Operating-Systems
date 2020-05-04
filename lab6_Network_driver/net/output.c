#include "ns.h"
#include "inc/lib.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";
	envid_t srcenv;
	int perm, ret;
	struct jif_pkt *pkt;

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while (true) {
		ret = ipc_recv(&srcenv, &nsipcbuf, &perm);
		if (ret != NSREQ_OUTPUT) {
			cprintf("not a NSREQ_OUTPUT ipc");
			continue;
		}

		pkt = &(nsipcbuf.pkt);
		// wait until wait buffer gets empty. This blocks ipc_recv()
		while (sys_net_send(pkt->jp_data, pkt->jp_len) < 0)
			sys_yield();
	}
}
