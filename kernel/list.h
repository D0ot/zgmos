#ifndef __LIST_H_
#define __LIST_H_


struct list_head {
  struct list_head *prev, *next;
};

static inline void list_init(struct list_head *list) {
  list->next = list;
  list->prev = list;
}

static inline void __list_add(struct list_head *newlist, struct list_head *prev, struct list_head *next) {
  newlist->prev = prev;
  newlist->next = next;
  prev->next = newlist;
  next->prev = newlist;
}

static inline void list_add(struct list_head *newlist, struct list_head *head) {
  __list_add(newlist, head, head->next);
}

static inline void list_add_tail(struct list_head *newlist, struct list_head *head) {
  __list_add(newlist, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next) {
  next->prev = prev;
  next->next = next;
}

static inline void list_del(struct list_head *entry) {
  __list_del(entry->prev, entry->next);
}

static inline void list_del_init(struct list_head *entry) {
  list_del(entry);
  list_init(entry);
}

static inline void list_replace(struct list_head *oldlist, struct list_head *newlist) {
  newlist->next = oldlist->next;
  newlist->next->prev = newlist;

  newlist->prev = oldlist->prev;
  newlist->prev->next = newlist;
}

static inline int list_empty(struct list_head *head) {
  return head->next == head;
}

#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); \
    pos = n, n = pos->next)

#define container_of(ptr, type, member) \
  (type*) ((uint8_t*)ptr - offsetof(type, member))

#endif // __LIST_H_
