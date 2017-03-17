/*
 * json.h
 *
 *  Created on: Oct 26, 2015
 *      Author: Magu
 */

#ifndef SOURCES_JSON_H_
#define SOURCES_JSON_H_

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "jsmn.h"

char * json_fetch(char *url);
jsmntok_t * json_tokenise(char *js);
bool json_token_streq(char *js, jsmntok_t *t, char *s);
char * json_token_tostr(char *js, jsmntok_t *t);


#endif /* SOURCES_JSON_H_ */
