#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/*
 * Initialize token list
 */
void token_list_init(token_list_t *list);

/*
 * Add one token
 */
void token_add(token_list_t *list,
               token_type_t type,
               const char *value);

/*
 * Convert input string into tokens
 */
void lexer(const char *input,
           token_list_t *list);

#endif
