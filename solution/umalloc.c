#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

static Header hugebase;
static Header *huge_freep;

void free(void *ap) { vfree(ap); }
// Header *bp, *p;

// bp = (Header*)ap - 1;
// for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
//   if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
//     break;
// if(bp + bp->s.size == p->s.ptr){
//   bp->s.size += p->s.ptr->s.size;
//   bp->s.ptr = p->s.ptr->s.ptr;
// } else
//   bp->s.ptr = p->s.ptr;
// if(p + p->s.size == bp){
//   p->s.size += bp->s.size;
//   p->s.ptr = bp->s.ptr;
// } else
//   p->s.ptr = bp;
// freep = p;

#define HUGE_PAGE_VSTART 0x1E000000
#define HUGE_PAGE_VEND 0x3E000000
void vfree(void *ap) {
  int huge = (((uint)ap >= HUGE_PAGE_VSTART) && ((uint)ap < HUGE_PAGE_VEND));
  Header **free_ptr;
  if (huge == 0) {
    free_ptr = &freep;
  } else if (huge == 1) {
    free_ptr = &huge_freep;
  }
  Header *bp, *p;

  bp = (Header *)ap - 1;
  for (p = *free_ptr; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
  if (bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  *free_ptr = p;
}

// static Header*
// morecore(uint nu)
// {
//   char *p;
//   Header *hp;

//   if(nu < 4096)
//     nu = 4096;
//   p = sbrk(nu * sizeof(Header), 0);
//   if (p == (char *)-1) return 0;
//   hp = (Header *)p;
//   hp->s.size = nu;
//   free((void *)(hp + 1));
//   return freep;
// }
static Header *vmorecore(uint nu, int huge) {
  char *p;
  Header *hp;

  if (nu < 4096) nu = 4096;
  p = sbrk(nu * sizeof(Header), huge);
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  vfree((void *)(hp + 1));
  if (huge) {
    return huge_freep;
  } else {
    return freep;
  }
}

void *malloc(uint nbytes) {
  if ((getthp()) && (nbytes > 1024 * 1024))
    return vmalloc(nbytes, VMALLOC_SIZE_HUGE);
  else
    return vmalloc(nbytes, VMALLOC_SIZE_BASE);
}
// Header *p, *prevp;
// uint nunits;

// nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
// if((prevp = freep) == 0){
//   base.s.ptr = freep = prevp = &base;
//   base.s.size = 0;
// }
// for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
//   if(p->s.size >= nunits){
//     if(p->s.size == nunits)
//       prevp->s.ptr = p->s.ptr;
//     else {
//       p->s.size -= nunits;
//       p += p->s.size;
//       p->s.size = nunits;
//     }
//     freep = prevp;
//     return (void*)(p + 1);
//   }
//   if(p == freep)
//     if((p = morecore(nunits)) == 0)
//       return 0;
// }

void *vmalloc(uint nbytes, int huge) {
  Header **free_ptr, *base_ptr;
  if (huge == VMALLOC_SIZE_BASE) {
    free_ptr = &freep;
    base_ptr = &base;
  } else if (huge == VMALLOC_SIZE_HUGE) {
    free_ptr = &huge_freep;
    base_ptr = &hugebase;
  } else {
    printf(1, "Please pass VMALLOC_SIZE_BASE or VMALLOC_SIZE_HUGE as flag.\n");
    exit();
  }
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if ((prevp = *free_ptr) == 0) {
    base_ptr->s.ptr = *free_ptr = prevp = base_ptr;
    base_ptr->s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      *free_ptr = prevp;
      return (void *)(p + 1);
    }
    if (p == *free_ptr)
      if ((p = vmorecore(nunits, huge)) == 0) return 0;
  }
}
