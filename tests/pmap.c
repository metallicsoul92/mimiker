#include <stdc.h>
#include <mips/tlb.h>
#include <pmap.h>
#include <physmem.h>
#include <vm.h>
#include <ktest.h>

static int test_kernel_pmap() {
  pmap_t *pmap = get_kernel_pmap();

  vm_page_t *pg = pm_alloc(16);
  size_t size = pg->size * PAGESIZE;
  vm_addr_t vaddr1 = pmap->start;
  vm_addr_t vaddr2 = pmap->start + size / 2;
  vm_addr_t vaddr3 = pmap->start + size;

  pmap_map(pmap, vaddr1, vaddr3, pg->paddr, VM_PROT_READ | VM_PROT_WRITE);

  {
    log("TLB before:");
    tlb_print();

    int *x = (int *)vaddr1;
    for (int i = 0; i < size / sizeof(int); i++)
      *(x + i) = i;
    for (int i = 0; i < size / sizeof(int); i++)
      ktest_assert(*(x + i) == i);

    log("TLB after:");
    tlb_print();
  }

  ktest_assert(pmap_probe(pmap, vaddr1, vaddr3, VM_PROT_READ | VM_PROT_WRITE));

  pmap_unmap(pmap, vaddr1, vaddr2);
  pmap_protect(pmap, vaddr2, vaddr3, VM_PROT_READ);

  ktest_assert(pmap_probe(pmap, vaddr1, vaddr2, VM_PROT_NONE));
  ktest_assert(pmap_probe(pmap, vaddr2, vaddr3, VM_PROT_READ));

  pmap_unmap(pmap, vaddr2, vaddr3);
  pm_free(pg);

  log("Test passed.");
  return KTEST_SUCCESS;
}

static int test_user_pmap() {
  pmap_t *orig = get_user_pmap();

  pmap_t *pmap1 = pmap_new();
  pmap_t *pmap2 = pmap_new();

  vm_addr_t start = 0x1001000;
  vm_addr_t end = 0x1002000;

  vm_page_t *pg1 = pm_alloc(1);
  vm_page_t *pg2 = pm_alloc(1);

  pmap_activate(pmap1);
  pmap_map(pmap1, start, end, pg1->paddr, VM_PROT_READ | VM_PROT_WRITE);
  pmap_activate(pmap2);
  pmap_map(pmap2, start, end, pg2->paddr, VM_PROT_READ | VM_PROT_WRITE);

  volatile int *ptr = (int *)start;
  *ptr = 100;
  pmap_activate(pmap1);
  *ptr = 200;
  pmap_activate(pmap2);
  ktest_assert(*ptr == 100);
  log("*ptr == %d", *ptr);
  pmap_activate(pmap1);
  ktest_assert(*ptr == 200);
  log("*ptr == %d", *ptr);

  pmap_delete(pmap1);
  pmap_delete(pmap2);

  /* Restore original user pmap */
  pmap_activate(orig);

  log("Test passed.");
  return KTEST_SUCCESS;
}

KTEST_ADD(pmap_kernel, test_kernel_pmap, 0);
KTEST_ADD(pmap_user, test_user_pmap, 0);
