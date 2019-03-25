/*
 * Remove cmpxchg !!!
 */
/*
 * Lock-less NULL terminated single linked list
 *
 * The basic atomic operation of this list is cmpxchg on long.  On
 * architectures that don't have NMI-safe cmpxchg implementation, the
 * list can NOT be used in NMI handlers.  So code that uses the list in
 * an NMI handler should depend on CONFIG_ARCH_HAVE_NMI_SAFE_CMPXCHG.
 *
 * Copyright 2010,2011 Intel Corp.
 *   Author: Huang Ying <ying.huang@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "llist.h"


/**
 * llist_add_batch - add several linked entries in batch
 * @new_first:    first entry in batch to be added
 * @new_last:    last entry in batch to be added
 * @head:    the head for your lock-less list
 *
 * Return whether list is empty before adding.
 */
bool llist_add_batch(struct llist_node *new_first, struct llist_node *new_last,
             struct llist_head *head)
{
    struct llist_node *first;

    new_last->next = first = head->first;
    head->first = new_first;

    return !first;
}

/**
 * llist2_add_batch - add several linked entries in batch
 * @new_first:    first entry in batch to be added
 * @new_last:    last entry in batch to be added
 * @head:    the head for your lock-less list
 *
 * Return whether list is empty before adding.
 */
bool llist2_add_batch(struct llist_node *new_first, struct llist_node *new_last,
             struct llist2_head *head)
{
    if (head->first == NULL) {
        head->first = new_first;
        head->last = new_last;
        return true;
    }

    head->last->next = new_first;
    head->last = new_last;
    return false;
}

/**
 * llist_del_first - delete the first entry of lock-less list
 * @head:    the head for your lock-less list
 *
 * If list is empty, return NULL, otherwise, return the first entry
 * deleted, this is the newest added one.
 *
 * Only one llist_del_first user can be used simultaneously with
 * multiple llist_add users without lock.  Because otherwise
 * llist_del_first, llist_add, llist_add (or llist_del_all, llist_add,
 * llist_add) sequence in another user may change @head->first->next,
 * but keep @head->first.  If multiple consumers are needed, please
 * use llist_del_all or use lock between consumers.
 */
struct llist_node *llist_del_first(struct llist_head *head)
{
    struct llist_node *entry;

    entry = head->first;
    if (entry == NULL)
        return NULL;
    head->first = entry->next;
    entry->next = NULL;

    return entry;
}

/**
 * llist_del_first - delete the first entry of lock-less list
 * @head:    the head for your lock-less list
 *
 * If list is empty, return NULL, otherwise, return the first entry
 * deleted, this is the newest added one.
 *
 * Only one llist_del_first user can be used simultaneously with
 * multiple llist_add users without lock.  Because otherwise
 * llist_del_first, llist_add, llist_add (or llist_del_all, llist_add,
 * llist_add) sequence in another user may change @head->first->next,
 * but keep @head->first.  If multiple consumers are needed, please
 * use llist_del_all or use lock between consumers.
 */
struct llist_node *llist2_del_first(struct llist2_head *head)
{
    struct llist_node *entry;

    entry = head->first;
    if (entry == NULL)
        return NULL;
    head->first = entry->next;
    if (head->first == NULL)
        head->last = NULL;
    entry->next = NULL;

    return entry;
}

/**
 * llist_reverse_order - reverse order of a llist chain
 * @head:    first item of the list to be reversed
 *
 * Reverse the order of a chain of llist entries and return the
 * new first entry.
 */
struct llist_node *llist_reverse_order(struct llist_node *head)
{
    struct llist_node *new_head = NULL;

    while (head) {
        struct llist_node *tmp = head;
        head = head->next;
        tmp->next = new_head;
        new_head = tmp;
    }

    return new_head;
}
