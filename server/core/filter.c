/*
 * This file is distributed as part of MaxScale from SkySQL.  It is free
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
 * Copyright SkySQL Ab 2014
 */

/**
 * @file filter.c  - A representation of a filter within MaxScale.
 *
 * @verbatim
 * Revision History
 *
 * Date		Who			Description
 * 29/05/14	Mark Riddoch		Initial implementation
 *
 * @endverbatim
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filter.h>
#include <session.h>
#include <modules.h>
#include <spinlock.h>
#include <skygw_utils.h>
#include <log_manager.h>

extern int lm_enabled_logfiles_bitmask;

static SPINLOCK	filter_spin = SPINLOCK_INIT;
static FILTER_DEF *allFilters = NULL;

/**
 * Allocate a new filter within MaxScale
 *
 *
 * @param name		The filter name
 * @param module	The module to load
 *
 * @return		The newly created filter or NULL if an error occured
 */
FILTER_DEF *
filter_alloc(char *name, char *module)
{
FILTER_DEF 	*filter;

	if ((filter = (FILTER_DEF *)malloc(sizeof(FILTER_DEF))) == NULL)
		return NULL;
	filter->name = strdup(name);
	filter->module = strdup(module);
	filter->filter = NULL;
	filter->options = NULL;
	filter->obj = NULL;
	filter->parameters = NULL;

	spinlock_init(&filter->spin);

	spinlock_acquire(&filter_spin);
	filter->next = allFilters;
	allFilters = filter;
	spinlock_release(&filter_spin);

	return filter;
}


/**
 * Deallocate the specified filter
 *
 * @param server	The service to deallocate
 * @return	Returns true if the server was freed
 */
void
filter_free(FILTER_DEF *filter)
{
FILTER_DEF *ptr;

	/* First of all remove from the linked list */
	spinlock_acquire(&filter_spin);
	if (allFilters == filter)
	{
		allFilters = filter->next;
	}
	else
	{
		ptr = allFilters;
		while (ptr && ptr->next != filter)
		{
			ptr = ptr->next;
		}
		if (ptr)
			ptr->next = filter->next;
	}
	spinlock_release(&filter_spin);

	/* Clean up session and free the memory */
	free(filter->name);
	free(filter->module);
	free(filter);
}

/**
 * Find an existing filter using the unique section name in
 * configuration file
 *
 * @param	name		The filter name
 * @return	The server or NULL if not found
 */
FILTER_DEF *
filter_find(char *name)
{
FILTER_DEF 	*filter;

	spinlock_acquire(&filter_spin);
	filter = allFilters;
	while (filter)
	{
		if (strcmp(filter->name, name) == 0)
			break;
		filter = filter->next;
	}
	spinlock_release(&filter_spin);
	return filter;
}

/**
 * Print all filters to a DCB
 *
 * Designed to be called within a debugger session in order
 * to display all active filters within MaxScale
 */
void
dprintAllFilters(DCB *dcb)
{
FILTER_DEF	*ptr;
int		i;

	spinlock_acquire(&filter_spin);
	ptr = allFilters;
	while (ptr)
	{
		dcb_printf(dcb, "Filter %p (%s)\n", ptr, ptr->name);
		dcb_printf(dcb, "\tModule:	%s\n", ptr->module);
		if (ptr->options)
		{
			dcb_printf(dcb, "\tOptions:	");
			for (i = 0; ptr->options && ptr->options[i]; i++)
				dcb_printf(dcb, "%s ", ptr->options[i]);
			dcb_printf(dcb, "\n");
		}
		if (ptr->obj && ptr->filter)
			ptr->obj->diagnostics(ptr->filter, NULL, dcb);
		else
			dcb_printf(dcb, "\tModule not loaded.\n");
		ptr = ptr->next;
	}
	spinlock_release(&filter_spin);
}

/**
 * Print filter details to a DCB
 *
 * Designed to be called within a debug CLI in order
 * to display all active filters in MaxScale
 */
void
dprintFilter(DCB *dcb, FILTER_DEF *filter)
{
int	i;

	dcb_printf(dcb, "Filter %p (%s)\n", filter, filter->name);
	dcb_printf(dcb, "\tModule:	%s\n", filter->module);
	if (filter->options)
	{
		dcb_printf(dcb, "\tOptions:	");
		for (i = 0; filter->options && filter->options[i]; i++)
			dcb_printf(dcb, "%s ", filter->options[i]);
		dcb_printf(dcb, "\n");
	}
	if (filter->obj && filter->filter)
		filter->obj->diagnostics(filter->filter, NULL, dcb);
}

/**
 * List all filters in a tabular form to a DCB
 *
 */
void
dListFilters(DCB *dcb)
{
FILTER_DEF	*ptr;
int	i;

	spinlock_acquire(&filter_spin);
	ptr = allFilters;
	if (ptr)
	{
		dcb_printf(dcb, "%-18s | %-15s | Options\n",
			"Filter", "Module");
		dcb_printf(dcb, "-------------------------------------------------------------------------------\n");
	}
	while (ptr)
	{
		dcb_printf(dcb, "%-18s | %-15s | ",
				ptr->name, ptr->module);
		for (i = 0; ptr->options && ptr->options[i]; i++)
			dcb_printf(dcb, "%s ", ptr->options[i]);
		dcb_printf(dcb, "\n");
		ptr = ptr->next;
	}
	spinlock_release(&filter_spin);
}

/**
 * Add a router option to a service
 *
 * @param service       The service to add the router option to
 * @param option        The option string
 */
void
filterAddOption(FILTER_DEF *filter, char *option)
{
int     i;

        spinlock_acquire(&filter->spin);
        if (filter->options == NULL)
        {
                filter->options = (char **)calloc(2, sizeof(char *));
                filter->options[0] = strdup(option);
                filter->options[1] = NULL;
        }
        else
        {
                for (i = 0; filter->options[i]; i++)
                        ;
                filter->options = (char **)realloc(filter->options,
                                (i + 2) * sizeof(char *));
                filter->options[i] = strdup(option);
                filter->options[i+1] = NULL;
        }
        spinlock_release(&filter->spin);
}

/**
 * Add a router parameter to a service
 *
 * @param service       The service to add the router option to
 * @param name		The parameter name
 * @param value		The parameter value
 */
void
filterAddParameter(FILTER_DEF *filter, char *name, char *value)
{
int     i;

        spinlock_acquire(&filter->spin);
        if (filter->parameters == NULL)
        {
                filter->parameters = (FILTER_PARAMETER **)calloc(2, sizeof(FILTER_PARAMETER *));
		i = 0;
        }
        else
        {
                for (i = 0; filter->parameters[i]; i++)
                        ;
                filter->parameters = (FILTER_PARAMETER **)realloc(filter->parameters,
                                (i + 2) * sizeof(FILTER_PARAMETER *));
        }
	filter->parameters[i] = (FILTER_PARAMETER *)calloc(1, sizeof(FILTER_PARAMETER));
	filter->parameters[i]->name = strdup(name);
	filter->parameters[i]->value = strdup(value);
	filter->parameters[i+1] = NULL;
        spinlock_release(&filter->spin);
}

DOWNSTREAM *
filterApply(FILTER_DEF *filter, SESSION *session, DOWNSTREAM *downstream)
{
DOWNSTREAM	*me;

	if (filter->obj == NULL)
	{
		/* Filter not yet loaded */
		if ((filter->obj = load_module(filter->module,
					MODULE_FILTER)) == NULL)
		{
			return NULL;
		}
	}
	if (filter->filter == NULL)
		filter->filter = (filter->obj->createInstance)(filter->options,
					filter->parameters);
	if ((me = (DOWNSTREAM *)calloc(1, sizeof(DOWNSTREAM))) == NULL)
	{
		return NULL;
	}
	me->instance = filter->filter;
	me->routeQuery = filter->obj->routeQuery;
	me->session = filter->obj->newSession(me->instance, session);

	filter->obj->setDownstream(me->instance, me->session, downstream);

	return me;
}
