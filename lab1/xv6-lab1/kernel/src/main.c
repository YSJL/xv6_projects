#include <stdatomic.h>

#include "asm/x86.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "string.h"
#include "stdio.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  //USER CHANGE
  uint *addr = (uint *) P2V(0x7E00); //Change this address
  uint numrecords = (uint) *addr;
  uint *e820 = addr + 1;

  size_t largestPa = 0;
  size_t largestLen = 0;


  size_t NEWPHYSTOP = PHYSTOP;

  for (uint i = 0; i < numrecords; i++) {
      size_t *PApointer = ((size_t *) e820) + i * 6;
      size_t PA = (size_t) *PApointer;
      size_t *LEpointer = PApointer + 2;
      size_t LE = (size_t) *LEpointer;
      uint *Tpointer = ((uint *) LEpointer) + 2;
      uint TYPE = (uint) *Tpointer;

      if (TYPE == 1 && PA > largestPa && (PA + LE) >= V2P(KERNBASE) && (PA + LE) < V2P(DEVSPACE)) {
          largestPa = PA;
          largestLen = LE;
          NEWPHYSTOP = largestPa + largestLen;
      } else if (TYPE == 1 && (PA + LE) >= V2P(DEVSPACE)) {
          NEWPHYSTOP = V2P(DEVSPACE) - 1;
          break;
      }
  }

  //ORIGINAL
  kinit1(end, P2V(4*1024*1024)); // phys page allocator
  //kvmalloc(PHYSTOP); // kernel page table
  kvmalloc(NEWPHYSTOP); // kernel page table NEWPHYSTOP
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  seginit();       // segment descriptors
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  pinit();         // process table
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk 
  startothers();   // start other processor
  //kinit2(P2V(4*1024*1024), P2V(PHYSTOP), PHYSTOP); // must come after startothers()
  kinit2(P2V(4*1024*1024), P2V(NEWPHYSTOP), NEWPHYSTOP); // must come after startothers() NEWPHYSTOP
  userinit();      // first user process
  mpmain();        // finish this processor's setup

}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  atomic_store(&mycpu()->started, 1); // tell startothers() we're up -- atomically
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(atomic_load(&c->started) == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

