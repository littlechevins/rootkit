#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysproto.h>
#include <stdlib.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_var.h>

struct port_finding_args {
	u_int16_t lport;	/* local port */
};

int
main(int argc, char *argv[])
{
	struct port_finding_args *uap;
	uap = (struct port_finding_args *)argv[1];

	INP_INFO_WLOCK(&tcbinfo);

	struct inpcb *inpb;
	/* Iterate through the TCP-based inpcb list. */
	LIST_FOREACH(inpb, tcbinfo.ipi_listhead, inp_list) {
		if (inpb->inp_vflag & INP_TIMEWAIT)
			continue;

			INP_LOCK(inpb);

		/* Do we want to hide this local open port? */
		if (uap->lport == ntohs(inpb->inp_inc.inc_ie.ie_lport))
			printf("ALERT! Insecure port detected\n");

			INP_UNLOCK(inpb);

	}

	INP_INFO_WUNLOCK(&tcbinfo);

	exit(0);
}
