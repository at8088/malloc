/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_small(unsigned long size)
{
    /* ecrire votre code ici */
    void * adresse_allouee_utilisabe = arena.chunkpool;
    if(arena.chunkpool != NULL){
        arena.chunkpool = arena.chunkpool + 96 ;                   // supprimer la tete de la liste .
        adresse_allouee_utilisabe = mark_memarea_and_get_user_ptr(adresse_allouee_utilisabe
        ,CHUNKSIZE,SMALL_KIND);
        return (void *)adresse_allouee_utilisabe;
    }else{
        unsigned long nv_size = mem_realloc_small();
        

    }


}

void efree_small(Alloc a) {
    /* ecrire votre code ici */
}
