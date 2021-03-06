#ifndef _MYSQLMON_H
#define _MYSQLMON_H
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
#include	<server.h>
#include	<spinlock.h>
#include	<mysql.h>

/**
 * @file mysqlmon.h - The MySQL monitor functionality within the gateway
 *
 * @verbatim
 * Revision History
 *
 * Date		Who			Description
 * 08/07/13	Mark Riddoch		Initial implementation
 * 26/05/14	Massimiliano	Pinto	Default values for MONITOR_INTERVAL
 * 28/05/14	Massimiliano	Pinto	Addition of new fields in MYSQL_MONITOR struct
 *
 * @endverbatim
 */

/**
 * The linked list of servers that are being monitored by the MySQL 
 * Monitor module.
 */
typedef struct monitor_servers {
	SERVER		*server;	/**< The server being monitored */
	MYSQL		*con;		/**< The MySQL connection */
	struct monitor_servers
			*next;		/**< The next server in the list */
} MONITOR_SERVERS;

/**
 * The handle for an instance of a MySQL Monitor module
 */
typedef struct {
        SPINLOCK  lock;	            /**< The monitor spinlock */
        pthread_t tid;              /**< id of monitor thread */ 
        int       shutdown;         /**< Flag to shutdown the monitor thread */
        int       status;           /**< Monitor status */
        char      *defaultUser;     /**< Default username for monitoring */
        char      *defaultPasswd;   /**< Default password for monitoring */
        unsigned long   interval;   /**< Monitor sampling interval */
        unsigned long         id;   /**< Monitor ID */
	int	replicationHeartbeat;	/**< Monitor flag for MySQL replication heartbeat */
	int	master_id;		/**< Master server-id for MySQL Master/Slave replication */
        MONITOR_SERVERS	*databases; /**< Linked list of servers to monitor */
} MYSQL_MONITOR;

#define MONITOR_RUNNING		1
#define MONITOR_STOPPING	2
#define MONITOR_STOPPED		3

#define MONITOR_INTERVAL 10000 // in milliseconds
#define MONITOR_DEFAULT_ID 1UL // unsigned long value

#endif
