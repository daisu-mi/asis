/*-
 * Copyright (C) 2014 Daisuke Miyamoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "asisd.h"
#include "asis_lookup.h"
#include "patricia.h"

/* extern patricia_tree_t *cf_tree; */

int asis_lookup(patricia_tree_t *cf_tree, char *cf_key, char *cf_value)
{
	int ret = ASIS_FALSE;
	patricia_node_t *cf_node;
	prefix_t *prefix = NULL;

	do {
		if (cf_tree == NULL){
			printf("asis: cf_tree is null\r\n");
			break;
		}

		if (asis_buffer_trim(cf_key) < 0){
			break;
		}

		if ((prefix = ascii2prefix(AF_INET, cf_key)) == NULL){
			break;
		}

		if ((cf_node = patricia_search_best(cf_tree, prefix)) == NULL){
			break;
		}

		if (cf_node->data == NULL){
			printf("asis : bogas memory data key %s\n", cf_key);
			break;
		}

		memcpy((void *)cf_value, cf_node->data, ASIS_BUFSIZ);
		ret = ASIS_TRUE;

	} while(0);

	if (ret == ASIS_TRUE){
		printf("asis : Query:%s, Answer:%s\n", cf_key, cf_value);
	}

	return ret;
}
