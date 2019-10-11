/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}
void  decoupage_bloc(int indice_case_a_decouper, int nbr_decoupage, unsigned long taille_case_a_decouper){
    if (nbr_decoupage == 0) return; // condition d'arret est quand on est dans la bonne case
    nbr_decoupage--; // on décrémente le nombre de découpage pour arriver à la bonne case (cf appel de cette fct)
    
    //découper == pointer la case précédente vers la moitié du bloc courant
    arena.TZL[indice_case_a_decouper-1] = arena.TZL[indice_case_a_decouper] + taille_case_a_decouper/2;

    arena.TZL[indice_case_a_decouper] = NULL;           // NULL == il existe plus de bloc de la taile 2**indice_case_a_decouper

    // écrire l'adresse de l'élément libre suivant (ie le buddy) dans le nouveau bloc créer 
    // sous forme d'un nombre de 64 bits == sizeof(void*) sur les machines de l'ensimag
    *((uint64_t*)arena.TZL[indice_case_a_decouper-1]) = (uint64_t)((void*)arena.TZL[indice_case_a_decouper-1] ^ taille_case_a_decouper/2);

    decoupage_bloc(indice_case_a_decouper-1,nbr_decoupage,taille_case_a_decouper/2);

}

void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    int bloc_plus_grand_trouve = 0;
    int indice = puiss2(size);
    unsigned long taille_case_i = size/2 ;
    for(int i = indice ; i< FIRST_ALLOC_MEDIUM_EXPOSANT+arena.medium_next_exponant ; i++){  
        // on commence la recherche depuis la case de la taille size pour 
        // trouver le  bloc le + petit qui convient
        taille_case_i *= 2;                    
        if(arena.TZL[i]!=NULL){
            decoupage_bloc(i,i-k,taille_case_i);
            bloc_plus_grand_trouve = 1;
            break;
        }
    }
    // A ce point là soit la case "indice" pointe vers un bloc de taille size soit y avait pas de bloc plus grand disponible
    if(!bloc_plus_grand_trouve){
        unsigned long nv_taille_max = mem_realloc_medium();
        decoupage_bloc(FIRST_ALLOC_MEDIUM_EXPOSANT+arena.medium_next_exponant , 
        FIRST_ALLOC_MEDIUM_EXPOSANT+arena.medium_next_exponant-k , nv_taille_max); 
        //On aurait pu utiliser la variable taille_case_i . Elle aura atteint
        // 2**FIRST_ALLOC_MEDIUM_EXPOSANT+arena.medium_next_exponant
    }

    void* adresse_a_renvoyer = arena.TZL[indice];
    arena.TZL[indice] = (void*) (*((uint64_t*)arena.TZL[indice])) ;

    return (void *) adresse_a_renvoyer;
}


void efree_medium(Alloc a) {
    /* ecrire votre code ici */
}
