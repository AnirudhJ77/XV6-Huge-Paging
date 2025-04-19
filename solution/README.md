Name : Anirudh Jagannath
Email : ajagannath@wisc.edu
CS Login : jagannath
ID : 908 604 6837
Status : All works fine.

Files changed :
vm.c - all vm functions(copyuvm, freevm, alloc_huge_uvm, dealloc_huge_uvm, mappages, map_hugepages, kmap, etc..)
proc.c - growproc
sys_proc.c - sbrk, setthp, getthp
user.h - added definitions
kalloc.c - khugealloc, khugefree, khugeinit
umalloc.c - malloc, free, vmalloc, vfree, vmorecore
syscall.c, syscall.h, uSYS.S - added syscall definitions
mmu.h - added HUGE_PTE_ADDR() macro
main.c - added khugeinit call
defs.h - changed func signatures of all modified functions
