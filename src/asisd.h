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

#ifndef __ASISD_H__
#define __ASISD_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <event.h>

#ifndef ASIS_BUFSIZ
#define ASIS_BUFSIZ 1500
#endif

/* 1 or -1 value */
#define ASIS_TRUE	1
#define ASIS_FALSE	-1

/* 1 or 0 value */
#define ASIS_SET	1
#define ASIS_UNSET	0

#define ASIS_DNSPORT	43

int asis_init(int argc, char *argv[]);
int asis_start(int);
int asis_stop(int);
int asis_socket();
int asis_reply(u_char *, int, u_char *, char *);
void asis_dump(u_char *, int);
void asis_usage();
uint16_t asis_pack(char [], int, char []);
void asis_sig_handler(int);
void asis_exit(int);


void asis_accept(int, short, void *);
void asis_recv(int, short, void *);

#endif
