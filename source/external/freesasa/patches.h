/* Header for patch functions needed by FreeSASA on macOS */

#ifndef FREESASA_PATCHES_H
#define FREESASA_PATCHES_H

#include "freesasa.h"

/* Function declarations for missing _lcl functions */

const char* freesasa_structure_atom_chain_lcl(const freesasa_structure *structure, int atom_index);

const char *freesasa_structure_chain_label(const freesasa_structure *structure, int index);

int freesasa_structure_chain_atoms_lcl(const freesasa_structure *structure,
                                      const char *chain,
                                      int *first,
                                      int *last);

int freesasa_structure_chain_residues_lcl(const freesasa_structure *structure,
                                         const char *chain,
                                         int *first,
                                         int *last);

#endif // FREESASA_PATCHES_H