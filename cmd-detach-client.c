/* $OpenBSD$ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <string.h>

#include "tmux.h"

/*
 * Detach a client.
 */

enum cmd_retval	 cmd_detach_client_exec(struct cmd *, struct cmd_q *);

const struct cmd_entry cmd_detach_client_entry = {
	"detach-client", "detach",
	"as:t:P", 0, 0,
	"[-P] [-a] [-s target-session] " CMD_TARGET_CLIENT_USAGE,
	CMD_READONLY|CMD_PREP_CLIENT_T|CMD_PREP_SESSION_S,
	cmd_detach_client_exec
};

const struct cmd_entry cmd_suspend_client_entry = {
	"suspend-client", "suspendc",
	"t:", 0, 0,
	CMD_TARGET_CLIENT_USAGE,
	CMD_PREP_CLIENT_T,
	cmd_detach_client_exec
};

enum cmd_retval
cmd_detach_client_exec(struct cmd *self, struct cmd_q *cmdq)
{
	struct args	*args = self->args;
	struct client	*c = cmdq->state.c, *c2;
	struct session	*s;
	enum msgtype	 msgtype;
	u_int 		 i;

	if (self->entry == &cmd_suspend_client_entry) {
		tty_stop_tty(&c->tty);
		c->flags |= CLIENT_SUSPENDED;
		server_write_client(c, MSG_SUSPEND, NULL, 0);
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'P'))
		msgtype = MSG_DETACHKILL;
	else
		msgtype = MSG_DETACH;

	if (args_has(args, 's')) {
		s = cmdq->state.sflag.s;
		for (i = 0; i < ARRAY_LENGTH(&clients); i++) {
			c = ARRAY_ITEM(&clients, i);
			if (c == NULL || c->session != s)
				continue;
			server_write_client(c, msgtype, c->session->name,
			    strlen(c->session->name) + 1);
		}
	} else {
		if (args_has(args, 'a')) {
			for (i = 0; i < ARRAY_LENGTH(&clients); i++) {
				c2 = ARRAY_ITEM(&clients, i);
				if (c2 == NULL || c2->session == NULL ||
				    c2 == c)
					continue;
				server_write_client(c2, msgtype,
				    c2->session->name,
				    strlen(c2->session->name) + 1);
			}
		} else {
			server_write_client(c, msgtype, c->session->name,
			    strlen(c->session->name) + 1);
		}
	}

	return (CMD_RETURN_STOP);
}
