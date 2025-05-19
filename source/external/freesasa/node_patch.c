#ifdef FREESASA_DISABLE_CIF
/* Fix for conflicting type in node.c */
char
freesasa_node_atom_chain(const freesasa_node *node)
{
    /* Simple implementation that returns the first character of the chain */
    if (node->type == FREESASA_NODE_ATOM) {
        if (node->properties.atom.chain && node->properties.atom.chain[0]) {
            return node->properties.atom.chain[0];
        }
    }
    return '\0';
}
#endif