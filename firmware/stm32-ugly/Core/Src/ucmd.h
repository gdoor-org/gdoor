/*! \file ********************************************************************
 *
 *  \brief      uCmd source file
 *
 *  This file declares Command_cb() and Command types for use with ucmd_parse()
 *
 *  \author     Dan Fekete <thefekete@gmail.com>
 *  \date       March 13, 2015
 *  \copyright  Copyright 2015, Dan Fekete <thefekete@gmail.com>, Released
 *  under the <a href="http://www.gnu.org/licenses/gpl-2.0.html">GNU GPL v2</a>
 *
 *****************************************************************************/
#ifndef _UCMD_H_
#define _UCMD_H_

#include <limits.h>

/** \brief return value when no command match is found */
#define UCMD_CMD_NOT_FOUND INT_MIN

/** \brief default delimeter to use if given NULL */
#define UCMD_DEFAULT_DELIMETER " "

/** \brief type for command callback functions
 *
 *  See \ref cmd_funcs for more information
 *
 *  \param int number of arguments (argc style)
 *  \param char*[] array of strings (argv style)
 *
 *  \return whatever you want (except #UCMD_CMD_NOT_FOUND)
 */
typedef int (*Command_cb)(int, char *[]);

/** \brief simple struct to hold data for a single command */
typedef struct Command {
    const char *cmd;                /**< the command string to match against */
    const char *help;                 /**< the help text associated with cmd */
    Command_cb fn;             /**< the function to call when cmd is matched */
} Command;

int ucmd_parse(Command [], const char *, const char *);

#endif /* _UCMD_H_ */
