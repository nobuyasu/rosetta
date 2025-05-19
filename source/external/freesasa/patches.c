/* Patch file for missing functions needed by FreeSASA on macOS */

#include "freesasa.h"
#include <string.h>
#include <stddef.h>

/* Function for node.c:236 - Convert char to const char* */
const char* freesasa_structure_atom_chain_lcl(const freesasa_structure *structure, int atom_index)
{
    static char chain_buffer[2] = {'\0', '\0'};
    if (structure == NULL || atom_index < 0) {
        return chain_buffer;
    }
    chain_buffer[0] = freesasa_structure_atom_chain(structure, atom_index);
    return chain_buffer;
}

/* Function for node.c:335 - Get chain label by index */
const char *freesasa_structure_chain_label(const freesasa_structure *structure, int index)
{
    if (structure == NULL || index < 0) {
        return "A"; // Default chain
    }
    
    const char *labels = freesasa_structure_chain_labels(structure);
    if (labels != NULL && index < strlen(labels)) {
        static char label[2] = {'\0', '\0'};
        label[0] = labels[index];
        return label;
    }
    
    return "A"; // Default chain
}

/* Function for node.c:339 - Redirect to public API */
int freesasa_structure_chain_atoms_lcl(const freesasa_structure *structure,
                                      const char *chain,
                                      int *first,
                                      int *last)
{
    return freesasa_structure_chain_atoms(structure, chain[0], first, last);
}

/* Function for node.c:349 - Redirect to public API */
int freesasa_structure_chain_residues_lcl(const freesasa_structure *structure,
                                         const char *chain,
                                         int *first,
                                         int *last)
{
    return freesasa_structure_chain_residues(structure, chain[0], first, last);
}