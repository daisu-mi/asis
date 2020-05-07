/*
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
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h> 

#include <event.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "asisd.h"
#include "asis_config.h"
#include "asis_lookup.h"
#include "patricia.h"

char bind_addr[ASIS_BUFSIZ];

/* uid */
#ifdef USE_SETUID
uid_t uid = 0;
gid_t gid = 0;
#endif

/* isset default option */
int bind_set_o;
#ifdef USE_SETUID
int uid_set_o;
#endif

patricia_tree_t *cf_tree;
char configfile[ASIS_BUFSIZ];

extern char copyright[];

int main(int argc, char *argv[])
{
	int server_sock;

	if ((server_sock = asis_init(argc, argv)) < 0){
		fprintf(stderr, "asis : server failed\r\n");
		asis_exit(ASIS_FALSE);
	}

	if (asis_start(server_sock) < 0){
		fprintf(stderr, "asis : stopped\r\n");
	}
	asis_exit(ASIS_FALSE);
}

int asis_start(int server_sock)
{

	struct event ev;

	event_init();
	event_set(&ev, server_sock, EV_READ | EV_PERSIST, asis_accept, &ev);
	event_add(&ev, NULL);
	event_dispatch();

	return ASIS_FALSE;
}

int asis_stop(int server_sock)
{
	if (close(server_sock) < 0){
		perror("close");
		return ASIS_FALSE;
	}
	else {
		return ASIS_TRUE;
	}
}

int asis_init(int argc, char *argv[])
{
	int server_sock;
	int op;
	int len;
#ifdef USE_SETUID
	struct passwd *pw;
#endif

	setvbuf(stdout, 0, _IONBF, 0);

	if ((cf_tree = New_Patricia(PATRICIA_MAXBITS)) == NULL){
		fprintf(stderr, "fatal error in creating patricia trie\n");
		asis_exit(EXIT_FAILURE);
	}

	bind_set_o = ASIS_UNSET;
	len = 0;

  while((op = getopt(argc, argv, "b:c:u:h?")) != -1){
		switch(op){
			case 'b': /* binding Address */
				if (bind_set_o == ASIS_SET){
					fprintf(stderr, "asis : duplicate bind address\n");
					asis_exit(EXIT_FAILURE);
				}
				else {
					bind_set_o = ASIS_SET;
					strncpy(bind_addr, optarg, ASIS_BUFSIZ);
				}
				break;
			
#ifdef USE_SETUID
			case 'u': /* uid */
				uid_set_o = ASIS_SET;
				if ((pw = getpwnam(optarg)) == NULL){
					fprintf(stderr, "asis : unknown username \"%s\"\n", optarg);
					asis_usage();
					asis_exit(EXIT_FAILURE);
				}
				else {
					uid = pw->pw_uid;
					gid = pw->pw_gid;
					pw = NULL;
				}
				break;
#endif
			case 'c': /* config file */
				strncpy(configfile, optarg, ASIS_BUFSIZ);
				if (asis_config_load(cf_tree, configfile) != ASIS_SET){
					asis_usage();
					asis_exit(EXIT_FAILURE);
				}
				if (cf_tree == NULL){
					asis_usage();
					asis_exit(EXIT_FAILURE);
				}
				break;

			case 'h': /* usage */
			case '?':
			default:
				asis_usage();
				asis_exit(EXIT_SUCCESS);
		}
	}

	/* set signal hander */
	signal(SIGINT, asis_sig_handler);
	signal(SIGHUP, asis_sig_handler);
	signal(SIGUSR1, asis_sig_handler);

	/* create server socket */
	if ((server_sock = asis_socket()) < 0){
		fprintf(stderr, "asis : server failed\r\n");
		asis_exit(EXIT_FAILURE);
	}

#ifdef USE_SETUID
	if (setgid(gid) < 0){
		perror("setgid");
		close(server_sock);
		return(ASIS_FALSE);
	}

	if (setuid(uid) < 0){
		perror("setuid");
		close(server_sock);
		return(ASIS_FALSE);
	}
#endif

	return server_sock;
}


int asis_socket()
{
	int yes = 1;
  int server_sock;
  struct sockaddr_in addr;

  /* socket */
  if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket");
    return(ASIS_FALSE);
  }

  memset((void *)&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(ASIS_DNSPORT);

	if (bind_set_o > 0){
		if (inet_pton(AF_INET, bind_addr, (void *)&(addr.sin_addr.s_addr)) < 0){
			perror("inet_pton(4)");
		}
	}
	else {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes))){
		perror("setsockopt");
    return(ASIS_FALSE);
	}

	if ( bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("bind");
		return(ASIS_FALSE);
	}

	if (listen(server_sock, -1) < 0) {
		perror("listen");
		return(ASIS_FALSE);
	}
	return server_sock;
}

void asis_accept(int server_sock, short event, void *arg)
{
  struct event *ev;
  struct sockaddr_in addr;
  int client_sock;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  if (event & EV_READ) {
    client_sock = accept(server_sock, (struct sockaddr*)&addr, &addrlen);
    ev = (struct event*)malloc(sizeof(struct event));
    event_set(ev, client_sock, EV_READ, asis_recv, ev);
    event_add(ev, NULL);
  }
}

void asis_recv(int client_sock, short event, void *arg)
{
	char buf[ASIS_BUFSIZ];
	char asnumber[ASIS_BUFSIZ];
	struct event *ev = (struct event*)arg;
	ssize_t recvlen;
	FILE *fp;

	memset(buf, '\0', ASIS_BUFSIZ);
	memset(asnumber, '\0', ASIS_BUFSIZ);

  if (event & EV_READ) {
		if ((fp = fdopen(client_sock, "r+")) == NULL){
      event_del(ev);
      free(ev);
      close(client_sock);
			return;
		}
		else if (fgets(buf, ASIS_BUFSIZ, fp) == NULL){
      event_del(ev);
      free(ev);
      close(client_sock);
			return;
		}
		else {
			asis_buffer_trim(buf);
			if (asis_lookup(cf_tree, buf, asnumber) == ASIS_TRUE){
				fprintf(fp, "%s", asnumber);
			}
			fclose(fp);
      event_del(ev);
      free(ev);
			close(client_sock);
    }
  }
}

void asis_usage()
{
	printf("asis : Nara Internet Name Fake Daemon \r\n");
	printf("\r\n");
	printf("    asis  -b [ Bind Address ] (optional) \r\n");
	printf("          -c [ Config File ] (optional) \r\n");
#ifdef USE_SETUID
	printf("          -u [ User Name ] (optional: default is \"root\") \r\n");
#endif
	printf("\r\n");
	printf("eg) asis -b 127.0.0.1\r\n");
	printf("\r\n");

	asis_exit(EXIT_SUCCESS);
}

void asis_sig_handler(int sig){
	switch(sig){
		case SIGINT:
		case SIGUSR1:
			asis_exit(EXIT_SUCCESS);
			break;
		case SIGHUP:
			asis_destroy_tree(cf_tree);
			asis_config_reload(cf_tree, configfile);
			break;
		default:
			break;
	}
	return;
}

void asis_exit(int status)
{
	asis_destroy_tree(cf_tree);

	if (status < 0){
		exit(EXIT_FAILURE);
	}
	else {
		exit(EXIT_SUCCESS);
	}
	return;
}
