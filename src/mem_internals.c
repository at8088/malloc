/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    /* ecrire votre code ici */
    uint64_t taille_marquee = size  ;      // delta == 32
    uint64_t * adresse_marquee = (uint64_t*)ptr;
    unsigned long magic = knuth_mmix_one_round((unsigned long)adresse_marquee);
    switch (k){
        case SMALL_KIND:
            magic &= ~0b11UL;
            break;
        case MEDIUM_KIND:
            magic &= ~0b10UL;
            break;
        case LARGE_KIND:
            magic &= ~0b01UL;
            break;
        default : break;
    }
    *adresse_marquee = (uint64_t)taille_marquee;
    *(adresse_marquee+1) = (uint64_t) magic;
    *(adresse_marquee +size/8 -2) = (uint64_t)taille_marquee;
    *(adresse_marquee +size/8 -1 ) = (uint64_t)magic;
    return (void *)adresse_marquee;
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    /* recup de marquage */
    Alloc a = {};
    a.ptr = (void*)((uint64_t*)ptr - 2 );
    switch ((*(uint64_t*)ptr - 1) & (0b11UL )){
        case 0: 
            a.kind=SMALL_KIND;
            break;
        case 1:
            a.kind = MEDIUM_KIND;
            break;
        case 2:
            a.kind = LARGE_KIND;
            break;
        default:break;
    }
    a.size = (unsigned long) (*(uint64_t*)a.ptr); 
    /*verification---------*/
    unsigned long magic = knuth_mmix_one_round((unsigned long)a.ptr);
    switch (a.kind){
        case SMALL_KIND:
            magic &=0b00UL;
            break;
        case MEDIUM_KIND:
            magic &=0b01UL;
            break;
        case LARGE_KIND:
            magic &=0b10UL;
            break;
        default : break;
    }
    if((uint64_t)magic != *((uint64_t*)a.ptr + 1)){
        fprintf(stderr,"Le nombre magique est invalide");
        exit(1);
    }
    if( *(uint64_t*)a.ptr != (uint64_t)(*(uint8_t*)ptr+a.size-16) ){
        fprintf(stderr,"Le marquage est invalide (la taille du debut != taille fin)");
        exit(1);

    }
    if( *(uint64_t*)a.ptr+1 != (uint64_t)(*(uint8_t*)ptr+a.size-8) ){
        fprintf(stderr,"Le marquage est invalide (le nombre magique du debut != celui de la fin)");
        exit(1);
    }

    return a;
}


unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
