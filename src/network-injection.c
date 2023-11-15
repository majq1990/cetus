/* $%BEGINLICENSE%$
 Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; version 2 of the
 License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA

 $%ENDLICENSE%$ */

#include <string.h>

#include "network-injection.h"

#include "network-mysqld-proto.h"
#include "network-mysqld-packet.h"
#include "glib-ext.h"
#include "chassis-timings.h"
#include "cetus-util.h"

#define TIME_DIFF_US(t2, t1) \
    ((t2.tv_sec - t1.tv_sec) * 1000000.0 + (t2.tv_usec - t1.tv_usec))

/**
 * Initialize an injection struct.
 */
injection *
injection_new(int id, GString *query)
{
    injection *i;

    i = g_new0(injection, 1);
    i->id = id;
    i->query = query;
    i->resultset_is_needed = 0; /* don't buffer the resultset */
    i->ts_read_query = 0;
    i->ts_read_query_result_last = 0;

    return i;
}

/**
 * Free an injection struct
 */
void
injection_free(injection *i)
{
    if (!i)
        return;

    if (i->query) {
        g_string_free(i->query, TRUE);
        i->query = NULL;
    }

    g_free(i);
}

network_injection_queue *
network_injection_queue_new()
{
    return g_queue_new();
}

void
network_injection_queue_free(network_injection_queue *q)
{
    if (!q)
        return;

    network_injection_queue_reset(q);

    g_queue_free(q);
}

void
network_injection_queue_reset(network_injection_queue *q)
{
    injection *inj;
    if (!q)
        return;

    while ((inj = g_queue_pop_head(q))) {
        injection_free(inj);
    }
}

void
network_injection_queue_append(network_injection_queue *q, injection *inj)
{
    g_queue_push_tail(q, inj);
}

void
network_injection_queue_prepend(network_injection_queue *q, injection *inj)
{
    g_queue_push_head(q, inj);
}

proxy_resultset_t *
proxy_resultset_new()
{
    proxy_resultset_t *res;

    res = g_new0(proxy_resultset_t, 1);

    return res;
}

/**
 * Free a resultset struct
 */
void
proxy_resultset_free(proxy_resultset_t *res)
{
    if (!res)
        return;

    if (res->fields) {
        network_mysqld_proto_fielddefs_free(res->fields);
    }

    g_free(res);
}
