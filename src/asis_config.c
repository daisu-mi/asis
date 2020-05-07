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
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "asisd.h"
#include "asis_config.h"
#include "asis_lookup.h"
#include "patricia.h"

/* extern patricia_tree_t *cf_tree; */

int asis_config_load(patricia_tree_t *cf_tree, char *filename)
{
	FILE *fp;
	patricia_node_t *node;
	int	 ret = ASIS_FALSE;
	char buf[ASIS_BUFSIZ];				/* store each lines in config file */
	char network[ASIS_BUFSIZ];			/* network value */
	char netmask[ASIS_BUFSIZ];			/* secnod value */
	char asnumber[ASIS_BUFSIZ];			/* asnumber value */
	char key[ASIS_BUFSIZ];				/* key of hash : maybe "network netmask" */

	if ((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "asis : configuration %s is not readable\n", filename);
		return ASIS_FALSE;
	}

	printf("asis : config-load start\n");
	while(fgets(buf, ASIS_BUFSIZ, fp) != NULL){
		/*
			broken line : break
		*/
		if (buf == NULL || strlen(buf) < 1 ){
			break;
		}

		/*
			comment line : read next
		*/
		if (buf[0] == '#'){
			continue;
		}

		/*
			trimming buffer 
		*/
		if (asis_buffer_trim(buf) < 1){
			continue;
		}

		memset(network, '\0', ASIS_BUFSIZ);	
		memset(netmask, '\0', ASIS_BUFSIZ);	
		memset(asnumber, '\0', ASIS_BUFSIZ);	

		sscanf(buf, "%s\t%s\t%s", (char *)&network, (char *)&netmask, (char *)&asnumber);

		snprintf(key, ASIS_BUFSIZ, "%s/%s", network, netmask);

    if ((node = make_and_lookup(cf_tree, key)) == NULL){
      fprintf(stderr, "whois : libpatricia failed (fatal error)\n");
    }
    else {
      char *value;
      if ((value = (char *)malloc(ASIS_BUFSIZ)) == NULL){
        ret = ASIS_FALSE;
				break;
      }
      memcpy((void *)value, (void *)asnumber, ASIS_BUFSIZ);
      node->data = value;
			ret = ASIS_TRUE;
    }
	}
	fclose(fp);	
	printf("asis : config-load done\n");

	/*
	if (cf_tree != NULL){
		printf("kitayo not null\n");
		asis_destroy_tree(cf_tree);	
	}	
	else {
		printf("konaiyo\n");
	}
	*/

	return ret;
}

int asis_buffer_trim(char buf[])
{
	char newbuf[ASIS_BUFSIZ];
	int i, j, space, frontspace;

	/* init variables */
	frontspace = space = i = j = 0;

	for (i = 0; i < strlen(buf); i++){
		if (frontspace == 0 && (buf[i] == '\t' || buf[i] == ' ')){
				continue;
		}
		else {
			frontspace = 1;
		}	

		switch(buf[i]){
			case '\r':
			case '\n':
				break;

			case '\t': 
			case ' ':
				if (space == 0){
					newbuf[j++] = ' ';
					space = 1;
				}
				break;

			default:
				newbuf[j++] = buf[i];
				space = 0;
				break;
		}
	}
	newbuf[j] = '\0';

	/* copy */
	memcpy((void *)buf, (void *)newbuf, ASIS_BUFSIZ);

	return j;
}

void asis_config_reload(patricia_tree_t *cf_tree, char *configfile) {

  if ((cf_tree = New_Patricia(PATRICIA_MAXBITS)) == NULL){
    fprintf(stderr, "fatal error in creating patricia tree\n");
		asis_exit(EXIT_FAILURE);
  }

	if (asis_config_load(cf_tree, configfile) != ASIS_TRUE){
		fprintf(stderr, "asis : cannnot reconstruct tree\n");
		asis_exit(EXIT_FAILURE);
	}

	if (cf_tree == NULL){
		fprintf(stderr, "asis : parameter missing\n");
		asis_usage();
		asis_exit(EXIT_FAILURE);
	}

  return;
}

void asis_destroy_tree(patricia_tree_t *cf_tree) {
	if (cf_tree != NULL){
		Destroy_Patricia(cf_tree, (void *)asis_destroy_func);
	}
	return;
}

void *asis_destroy_func(void *data) {
	char *tmp;
	if (data == NULL){
		return;
	}
	else {
		tmp = (char *)data;
		free(tmp);
	}
	return;
}
