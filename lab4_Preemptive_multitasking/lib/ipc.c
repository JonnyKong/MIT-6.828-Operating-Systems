// User-level IPC library routines

#include <inc/lib.h>

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'thisenv' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	// LAB 4: Your code here.
	if (pg == NULL)
		pg = (void *)UTOP;
	
	int syscall_ret = sys_ipc_recv(pg);
	int ipc_from = 0, ipc_perm = 0, ipc_value = 0;
	if (syscall_ret == 0) {
		ipc_from = thisenv->env_ipc_from;
		ipc_perm = thisenv->env_ipc_perm;
        ipc_value = thisenv->env_ipc_value;
	}

	if (from_env_store)
		*from_env_store = ipc_from;
	if (perm_store)
		*perm_store = ipc_perm;

	return syscall_ret == 0 ? ipc_value : syscall_ret;
}

// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_try_send a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	// LAB 4: Your code here.
	if (pg == NULL)
		pg = (void *)UTOP;
	
	int syscall_ret;
	while ((syscall_ret = sys_ipc_try_send(to_env, val, pg, perm)) != 0) {
		if (syscall_ret != -E_IPC_NOT_RECV)
			panic("ipc_send error: %e", syscall_ret);
		sys_yield();
	}
}

// Find the first environment of the given type.  We'll use this to
// find special environments.
// Returns 0 if no such environment exists.
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}
