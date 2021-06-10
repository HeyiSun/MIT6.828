// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	uint32_t pgnum = PGNUM(addr), pdx = PDX(addr);
	assert(err & FEC_WR);
	assert((uvpd[pdx] & PTE_P) && (uvpt[pgnum] & PTE_P) && (uvpt[pgnum] & PTE_COW));

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	if ((r = sys_page_alloc(0, (void*) PFTEMP, PTE_P | PTE_U | PTE_W)) < 0) {
		panic("pgfault: sys_page_alloc -- %e", r);
	}
	memcpy(PFTEMP, (void*) ROUNDDOWN((uint32_t) addr, PGSIZE), PGSIZE);
	if ((r = sys_page_map(0,
			      (void*) PFTEMP,
			      0,
			      (void*) ROUNDDOWN((uint32_t) addr, PGSIZE),
			      PTE_P | PTE_U | PTE_W)) < 0) {
		panic("pgfault: sys_page_map -- %e", r);
	}
	if ((r = sys_page_unmap(0, (void*) PFTEMP)) < 0) {
		panic("pgfault: sys_page_unmap -- %e", r);
	}

}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.
	int r;
	assert((uvpt[pn] & PTE_COW) || (uvpt[pn] & PTE_W));
	void* addr = (void *)(pn * PGSIZE);;

	r = sys_page_map(0, addr, envid, addr, PTE_U | PTE_P | PTE_COW);
	if (r < 0) {
		panic("duppage: sys_page_map failed on child mapping\n");
	}
	r = sys_page_map(0, addr, 0, addr, PTE_U | PTE_P | PTE_COW);
	if (r < 0) {
		panic("duppage: sys_page_map failed on parent mapping\n");
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	envid_t envid;
	int r;
	extern void _pgfault_upcall();
	uint32_t addr;

	envid = sys_exofork();
	if (envid < 0) {
		panic("sys_exofork: %e", envid);
	}
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	set_pgfault_handler(pgfault);
	for (addr = (uint32_t) UTEXT; addr < USTACKTOP; addr += PGSIZE) {
		uint32_t pgnum = PGNUM(addr), pdx = PDX(addr);
		if ((uvpd[pdx] & PTE_P) &&
		    (uvpt[pgnum] & PTE_P) &&
		    (uvpt[pgnum] & PTE_U)) {
            if (uvpt[pgnum] & PTE_SHARE) {
				r = sys_page_map(0, (void *)addr, envid, (void *)addr, uvpt[pgnum]&PTE_SYSCALL);
				if (r < 0) {
					panic("fork: pte_share sys_page_map fail\n");
				}
            } else if ((uvpt[pgnum] & PTE_COW) || (uvpt[pgnum] & PTE_W)) {
				duppage(envid, pgnum);
			} else {
				r = sys_page_map(0, (void *)addr, envid, (void *)addr, PTE_U | PTE_P);
				if (r < 0) {
					panic("fork: read-only sys_page_map fail\n");
				}
			}
		}
	}
	// TODO: Also copy the stack we are currently running on.
	r = sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE), PTE_W | PTE_U | PTE_P);
	if (r < 0) {
		panic("sys_page_alloc: %e", r);
	}
	r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
	if (r < 0) {
		panic("sys_env_set_pgfault_upcall: %e", r);
	}
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
