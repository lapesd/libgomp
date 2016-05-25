/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/work.c - Work library.
 */

#include <assert.h>
#include <global.h>
#include <message.h>
#include <stdarg.h>
#include <util.h>

/*
 * Creates a message.
 */
struct message *message_create(int type, ...)
{
	va_list ap;          /* Arguments pointer. */
	struct message *msg; /* Message.           */
	
	va_start(ap, type);
	
	msg = smalloc(sizeof(struct message));
	
	/* Parse type of message. */
	switch (type)
	{
		/* SORTWORK. */
		case SORTWORK :
			msg->u.sortwork.id = va_arg(ap, int);
			msg->u.sortwork.size = va_arg(ap, int);
			msg->type = SORTWORK;
			break;
		
		/* SORTRESULT. */
		case SORTRESULT :
			msg->u.sortresult.id = va_arg(ap, int);
			msg->u.sortresult.size = va_arg(ap, int);
			msg->type = SORTRESULT;
			break;
		
		/* FINDWORK. */
		case FINDWORK :
			msg->u.findwork.i0 = va_arg(ap, int);
			msg->u.findwork.j0 = va_arg(ap, int);
			msg->u.findwork.height = va_arg(ap, int);
			msg->u.findwork.width = va_arg(ap, int);
			msg->type = FINDWORK;
			break;
		
		/* FINDRESULT. */
		case FINDRESULT :
			msg->u.findresult.i0 = va_arg(ap, int);
			msg->u.findresult.j0 = va_arg(ap, int);
			msg->u.findresult.ipvt = va_arg(ap, int);
			msg->u.findresult.jpvt = va_arg(ap, int);
			msg->type = FINDRESULT;
			break;
		
		/* REDUCTWORK. */
		case REDUCTWORK :
			msg->u.reductwork.ipvt = va_arg(ap, int);
			msg->u.reductwork.i0 = va_arg(ap, int);
			msg->u.reductwork.j0 = va_arg(ap, int);
			msg->u.reductwork.height = va_arg(ap, int);
			msg->u.reductwork.width = va_arg(ap, int);
			msg->type = REDUCTWORK;
			break;
		
		/* REDUCTRESULT. */
		case REDUCTRESULT :
			msg->u.reductresult.i0 = va_arg(ap, int);
			msg->u.reductresult.j0 = va_arg(ap, int);
			msg->u.reductresult.height = va_arg(ap, int);
			msg->u.reductresult.width = va_arg(ap, int);
			msg->type = REDUCTRESULT;
			break;
		
		/* DIE. */
		default :
			msg->type = DIE;
			break;
	}
	
	msg->next = NULL;
	va_end(ap);
	
	return (msg);
}

/*
 * Destroys a message.
 */
void message_destroy(struct message *msg)
{
	free(msg);
}

/*
 * Sends a message.
 */
void message_send(int outfd, struct message *msg)
{
	communication += data_send(outfd, msg, sizeof(struct message));
}

/*
 * Receives a message.
 */
struct message *message_receive(int infd)
{
	struct message *msg;
	
	msg = message_create(DIE);
	
	communication += data_receive(infd, msg, sizeof(struct message));
	
	return (msg);
}
