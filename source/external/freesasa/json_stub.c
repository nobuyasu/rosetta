#include "freesasa.h"
#include "freesasa_internal.h"

/* Stub implementations for json.c functions */

int
freesasa_write_json(FILE *output, 
                   freesasa_node *tree,
                   int options)
{
    /* Don't do anything in the disabled version */
    return FREESASA_FAIL;
}

int
freesasa_write_json_value(FILE *output, 
                         const char *key,
                         const char *value,
                         int options)
{
    /* Don't do anything in the disabled version */
    return FREESASA_FAIL;
}