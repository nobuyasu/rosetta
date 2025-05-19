/* Additional patch functions for FreeSASA node.c compatibility */

#include "freesasa.h"
#include "freesasa_internal.h"
#include <string.h>

/* Convert char result to char* for atom chain */
const char* freesasa_structure_atom_chain_lcl(const freesasa_structure *structure, int atom_index)
{
    static char chain_buffer[2] = {'\0', '\0'};
    chain_buffer[0] = freesasa_structure_atom_chain(structure, atom_index);
    return chain_buffer;
}

/* Wrapper for chain atoms function - these functions already exist in structure.c 
   but are not exposed in the header, so we'll create stubs that redirect to
   the public API functions */
int freesasa_structure_chain_atoms_lcl(const freesasa_structure *structure,
                                      const char *chain,
                                      int *first,
                                      int *last)
{
    return freesasa_structure_chain_atoms(structure, chain, first, last);
}

int freesasa_structure_chain_residues_lcl(const freesasa_structure *structure,
                                         const char *chain,
                                         int *first,
                                         int *last)
{
    return freesasa_structure_chain_residues(structure, chain, first, last);
}