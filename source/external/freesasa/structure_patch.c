#ifndef FREESASA_DISABLE_CIF
int
freesasa_structure_add_cif_atom_lcl(freesasa_structure *structure,
                                  freesasa_cif_atom_lcl *atom,
                                  const freesasa_classifier *classifier,
                                  int options)
{
    /* Implementation only needed when CIF support is enabled */
    return FREESASA_FAIL;
}
#endif

#ifndef FREESASA_DISABLE_CIF
/* CIF chain group functions */
typedef struct {
    size_t n;
    chain_label_t *chains;
} freesasa_chain_group;

static int
chain_group_has_chain(const freesasa_chain_group *chains, chain_label_t chain)
{
    /* Empty implementation for disabled CIF support */
    return 0;
}

char **
freesasa_structure_get_chains_lcl(const freesasa_structure *structure,
                               const freesasa_chain_group *chains,
                               int options,
                               int *n)
{
    /* Empty implementation for disabled CIF support */
    *n = 0;
    return NULL;
}
#endif