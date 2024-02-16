/*! \file ********************************************************************
 *
 *  \brief      uCmd source file
 *
 *  This file defines the only function in this module: ucmd_parse()
 *
 *  \author     Dan Fekete <thefekete@gmail.com>
 *  \date       March 13, 2015
 *  \copyright  Copyright 2015, Dan Fekete <thefekete@gmail.com>, Released
 *  under the <a href="http://www.gnu.org/licenses/gpl-2.0.html">GNU GPL v2</a>
 *
 *****************************************************************************/
#include <stdlib.h>  // free
#include <string.h>  // strlen, strcpy, strtok, strcmp
#include "ucmd.h"

/** \brief Parse a command string and match against a \ref cmd_list
 *
 *  \param  cmd_list a Command array (see the \ref cmd_list section
 *          for more info)
 *  \param  delim a null terminated string used to split the command string
 *  \param  in a null terminated command string to parse
 *
 *  \return if a command is found, returns commands return code. Otherwise,
 *          returns UCMD_CMD_NOT_FOUND for unmatched command or 0 for an empty
 *          or NULL command string.
 */
int ucmd_parse(Command cmd_list[], const char *delim, const char *in)
{
    if (!in || strlen(in) == 0) return 0;        // return 0 for empty commands
    if (!cmd_list) return UCMD_CMD_NOT_FOUND;   // obviously not found, no list

    delim = (delim) ? delim : UCMD_DEFAULT_DELIMETER;

    int retval = 0;

    char *s = malloc(strlen(in)+1);                 // just in case in is const
    strcpy(s, in);

    int argc = 0;
    char **argv = NULL;

    char *arg = strtok(s, delim);
    while (arg) {
        argc++;
        argv = realloc(argv, argc * sizeof(*argv));
        argv[argc-1] = arg;
        arg = strtok(NULL, delim);
    }

    if (argc) {
        Command *c = NULL;
        for (Command *p = cmd_list; p->cmd; p++) {
            if (strcmp(p->cmd, argv[0]) == 0) {
                c = p;
            }
        }
        if (c) {
            retval = c->fn(argc, argv);
        } else {
            retval = UCMD_CMD_NOT_FOUND;
        }
    }

    free(argv);
    free(s);

    return retval;
}
