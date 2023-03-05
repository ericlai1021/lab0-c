#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(*head));

    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l || list_empty(l)) {
        free(l);
        return;
    }

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        list_del(&entry->list);
        q_release_element(entry);
    }

    free(l);

    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(*node));
    if (!node)
        return false;

    size_t len = strlen(s) + 1;
    node->value = malloc(sizeof(char) * len);
    if (!node->value) {
        free(node);
        return false;
    }

    strncpy(node->value, s, len);
    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(*node));
    if (!node)
        return false;

    size_t len = strlen(s) + 1;
    node->value = malloc(sizeof(char) * len);
    if (!node->value) {
        free(node);
        return false;
    }

    strncpy(node->value, s, len);
    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_first_entry(head, element_t, list);
    list_del_init(&node->list);

    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_last_entry(head, element_t, list);
    list_del_init(&node->list);

    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *pos;

    list_for_each (pos, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *forward = head->next, *backward = head->prev;
    while (forward != backward && forward->next != backward) {
        forward = forward->next;
        backward = backward->prev;
    }
    list_del_init(forward);
    element_t *entry = list_entry(forward, element_t, list);
    q_release_element(entry);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head) || list_is_singular(head))
        return false;

    element_t *entry, *safe;
    element_t *del = NULL;
    list_for_each_entry_safe (entry, safe, head, list) {
        if ((&safe->list != head) && !strcmp(entry->value, safe->value)) {
            del = safe;
            list_del(&entry->list);
            q_release_element(entry);
        } else if (del) {
            list_del(&del->list);
            q_release_element(del);
            del = NULL;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        node->next = node->prev;
        node->prev = safe;
    }
    head->next = head->prev;
    head->prev = safe;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    LIST_HEAD(new_head);
    struct list_head *node, *safe, *cut = head;
    int cnt = 0;
    list_for_each_safe (node, safe, head) {
        if (++cnt == k) {
            list_cut_position(&new_head, cut, node);
            q_reverse(&new_head);
            list_splice_init(&new_head, cut);
            cut = safe->prev;
            cnt = 0;
        }
    }
}

static struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    if (!l1)
        return l2;
    if (!l2)
        return l1;

    if (strcmp(list_entry(l1, element_t, list)->value,
               list_entry(l2, element_t, list)->value) < 0) {
        l1->next = merge(l1->next, l2);
        return l1;
    } else {
        l2->next = merge(l1, l2->next);
        return l2;
    }
}

static struct list_head *mergeSortList(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head->next;
    struct list_head *slow = head;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    fast = slow->next;
    slow->next = NULL;

    struct list_head *l1 = mergeSortList(head);
    struct list_head *l2 = mergeSortList(fast);

    return merge(l1, l2);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    head->prev->next = NULL;
    head->next = mergeSortList(head->next);
    struct list_head *cur = head, *next = head->next;
    while (next) {
        next->prev = cur;
        cur = next;
        next = next->next;
    }
    cur->next = head;
    head->prev = cur;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *pos = head->prev;
    char *maxStr = "";
    while (pos != head) {
        element_t *entry = list_entry(pos, element_t, list);
        pos = pos->prev;
        if (strcmp(entry->value, maxStr) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else {
            maxStr = entry->value;
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    struct list_head *list = list_first_entry(head, queue_contex_t, chain)->q;
    queue_contex_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, chain) {
        if (entry->id == 0)
            continue;

        list_splice_init(entry->q, list);
    }
    q_sort(list);

    return q_size(list);
}
