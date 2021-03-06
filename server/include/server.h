#ifndef _SERVER_H
#define _SERVER_H
/*
 * This file is distributed as part of the SkySQL Gateway.  It is free
 * software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation,
 * version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright SkySQL Ab 2013
 */
#include <dcb.h>

/**
 * @file service.h
 *
 * The server level definitions within the gateway
 *
 * @verbatim
 * Revision History
 *
 * Date		Who			Description
 * 14/06/13	Mark Riddoch		Initial implementation
 * 21/06/13	Mark Riddoch		Addition of server status flags
 * 22/07/13	Mark Riddoch		Addition of JOINED status for Galera
 * 18/05/14	Mark Riddoch		Addition of unique_name field
 * 20/05/14	Massimiliano Pinto	Addition of server_string field
 * 20/05/14	Massimiliano Pinto	Addition of node_id field
 * 23/05/14	Massimiliano Pinto	Addition of rlag and node_ts fields
 * 03/06/14	Mark Riddoch		Addition of maintainance mode
 *
 * @endverbatim
 */

/**
 * The server statistics structure
 *
 */
typedef struct {
	int		n_connections;	/**< Number of connections */
	int		n_current;	/**< Current connections */
} SERVER_STATS;

/**
 * The SERVER structure defines a backend server. Each server has a name
 * or IP address for the server, a port that the server listens on and
 * the name of a protocol module that is loaded to implement the protocol
 * between the gateway and the server.
 */
typedef struct server {
	char		*unique_name;	/**< Unique name for the server */
	char		*name;		/**< Server name/IP address*/
	unsigned short	port;		/**< Port to listen on */
	char		*protocol;	/**< Protocol module to use */
	unsigned int	status;		/**< Status flag bitmap for the server */
	char		*monuser;	/**< User name to use to monitor the db */
	char		*monpw;		/**< Password to use to monitor the db */
	SERVER_STATS	stats;		/**< The server statistics */
	struct	server	*next;		/**< Next server */
	struct	server	*nextdb;	/**< Next server in list attached to a service */
	char		*server_string;	/**< Server version string, i.e. MySQL server version */
	long		node_id;	/**< Node id, server_id for M/S or local_index for Galera */
	int		rlag;		/**< Replication Lag for Master / Slave replication */
	unsigned long	node_ts;	/**< Last timestamp set from M/S monitor module */
} SERVER;

/**
 * Status bits in the server->status member.
 *
 * These are a bitmap of attributes that may be applied to a server
 */
#define	SERVER_RUNNING	0x0001		/**<< The server is up and running */
#define SERVER_MASTER	0x0002		/**<< The server is a master, i.e. can handle writes */
#define SERVER_SLAVE	0x0004		/**<< The server is a slave, i.e. can handle reads */
#define SERVER_JOINED	0x0008		/**<< The server is joined in a Galera cluster */
#define SERVER_MAINT	0x1000		/**<< Server is in maintenance mode */

/**
 * Is the server running - the macro returns true if the server is marked as running
 * regardless of it's state as a master or slave
 */
#define	SERVER_IS_RUNNING(server)	(((server)->status & (SERVER_RUNNING|SERVER_MAINT)) == SERVER_RUNNING)
/**
 * Is the server marked as down - the macro returns true if the server is beleived
 * to be inoperable.
 */
#define	SERVER_IS_DOWN(server)		(((server)->status & SERVER_RUNNING) == 0)
/**
 * Is the server a master? The server must be both running and marked as master
 * in order for the macro to return true
 */
#define	SERVER_IS_MASTER(server) \
			(((server)->status & (SERVER_RUNNING|SERVER_MASTER|SERVER_SLAVE|SERVER_MAINT)) == (SERVER_RUNNING|SERVER_MASTER))
/**
 * Is the server a slave? The server must be both running and marked as a slave
 * in order for the macro to return true
 */
#define	SERVER_IS_SLAVE(server)	\
			(((server)->status & (SERVER_RUNNING|SERVER_MASTER|SERVER_SLAVE|SERVER_MAINT)) == (SERVER_RUNNING|SERVER_SLAVE))

/**
 * Is the server joined Galera node? The server must be running and joined. 
 */
#define SERVER_IS_JOINED(server) \
	(((server)->status & (SERVER_RUNNING|SERVER_JOINED|SERVER_MAINT)) == (SERVER_RUNNING|SERVER_JOINED))

/**
 * Is the server in maintenance mode. 
 */
#define SERVER_IN_MAINT(server)		((server)->status & SERVER_MAINT)

extern SERVER	*server_alloc(char *, char *, unsigned short);
extern int	server_free(SERVER *);
extern SERVER	*server_find_by_unique_name(char *);
extern SERVER	*server_find(char *, unsigned short);
extern void	printServer(SERVER *);
extern void	printAllServers();
extern void	dprintAllServers(DCB *);
extern void	dprintServer(DCB *, SERVER *);
extern void	dListServers(DCB *);
extern char	*server_status(SERVER *);
extern void	server_set_status(SERVER *, int);
extern void	server_clear_status(SERVER *, int);
extern void	serverAddMonUser(SERVER *, char *, char *);
extern void	server_update(SERVER *, char *, char *, char *);
extern void     server_set_unique_name(SERVER *, char *);
#endif
