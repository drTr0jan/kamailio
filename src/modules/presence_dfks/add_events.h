/*
 * presence_dfks module - add_event header file
 *
 * Copyright (C) 2014 Maja Stanislawska <maja.stanislawska@yahoo.com>
 * Copyright (C) 2024 Victor Seva <vseva@sipwise.com>
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _DFKS_ADD_EV_H_
#define _DFKS_ADD_EV_H_

int dfks_add_events(void);
int dfks_publ_handler(struct sip_msg *msg);
int dfks_subs_handler(struct sip_msg *msg);
#endif
