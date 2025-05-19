#include "freesasa.h"
#include "freesasa_internal.h"
#include <string.h>

/* Stub function for node.c:599 - Returns first character of chain string */
char 
freesasa_node_atom_chain(const freesasa_node *node)
{
    if (node == NULL) return '\0';
    
    const char *chain = NULL;
    if (node->type == FREESASA_NODE_ATOM && 
        (chain = node->properties.atom.chain) != NULL) {
        return chain[0];
    }
    return '\0';
}