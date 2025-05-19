#include "freesasa.h"
#include "freesasa_internal.h"
#include <string.h>

/* Define CIF types needed by the structure.c file */
typedef struct {
    char auth_atom_id[PDB_ATOM_NAME_STRL+1];     /* atom name */
    char auth_comp_id[PDB_ATOM_RES_NAME_STRL+1]; /* residue name */
    char auth_seq_id[PDB_ATOM_RES_NUMBER_STRL+1]; /* residue number */
    char auth_asym_id[PDB_ATOM_CHAIN_STRL+1];    /* chain */
    char pdbx_PDB_ins_code[2];             /* insertion code (plus null) */
    char type_symbol[PDB_ATOM_SYMBOL_STRL+1];    /* element symbol */
    double Cartn_x, Cartn_y, Cartn_z;      /* coordinates */
} freesasa_cif_atom;

/* For the lcl version which is specific to this library */
typedef freesasa_cif_atom freesasa_cif_atom_lcl;

/* Define chain group type */
typedef struct {
    int n;
    chain_label_t *chains;
} freesasa_chain_group;

/* Stub implementation for structure.c:812 */
int 
freesasa_structure_add_cif_atom_lcl(freesasa_structure *structure,
                                  freesasa_cif_atom_lcl *atom,
                                  const freesasa_classifier *classifier,
                                  int options)
{
    /* Don't do anything in the disabled version */
    return FREESASA_FAIL;
}

/* Stub implementation for structure.c:1012 */
static int
chain_group_has_chain(const freesasa_chain_group *chains, chain_label_t chain)
{
    /* Don't do anything in the disabled version */
    return 0;
}

/* Stub implementation for structure.c:1026 */
char **
freesasa_structure_get_chains_lcl(const freesasa_structure *structure,
                               const freesasa_chain_group *chains,
                               int options,
                               int *n)
{
    /* Don't do anything in the disabled version */
    if (n) *n = 0;
    return NULL;
}

/* Missing function for node.c:335 - Get chain label by index */
const char *freesasa_structure_chain_label(const freesasa_structure *structure, int index)
{
    static const char *default_chain = "A";
    if (structure == NULL || index < 0) {
        return default_chain;
    }
    /* For our stub, just return a default chain ID */
    return default_chain;
}