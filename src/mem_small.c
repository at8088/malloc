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
 
    void * adresse_allouee_utilisable = arena.chunkpool;
    if(arena.chunkpool != NULL){
        arena.chunkpool = arena.chunkpool + CHUNKSIZE ;                   // supprimer la tete de la liste .
        adresse_allouee_utilisable = mark_memarea_and_get_user_ptr(adresse_allouee_utilisable
        ,CHUNKSIZE,SMALL_KIND);
        return (void *)adresse_allouee_utilisable;
    }else{
        unsigned long nv_size = mem_realloc_small();
        void * chunk_suivant = arena.chunkpool + CHUNKSIZE;
        void ** chunk_courant = arena.chunkpool;
        while(chunk_suivant < arena.chunkpool + nv_size - CHUNKSIZE){
            *chunk_courant=(void*)chunk_suivant;
            chunk_suivant+=CHUNKSIZE;
            chunk_courant+=CHUNKSIZE/sizeof(void*);
        }
        adresse_allouee_utilisable = mark_memarea_and_get_user_ptr(arena.chunkpool
        ,CHUNKSIZE,SMALL_KIND);
        arena.chunkpool = arena.chunkpool + CHUNKSIZE ;                  
        return (void *)adresse_allouee_utilisable;

    }


}

void efree_small(Alloc a) {
    void * tmp = a.ptr;
    void ** ptr_tete = (void**)a.ptr;
    *ptr_tete = (void*)arena.chunkpool;       //ecrire dans la nouvelle tete l'adresse du chunk suivant
    /*ajout en tete de l'adresse du chunk */
    arena.chunkpool = tmp;
}
