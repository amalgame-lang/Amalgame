/*
 * Amalgame Standard Library - Amalgame.Collections
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/BastienMOUGET/Amalgame
 *
 * Provides: List<T>, Map<K,V>, Set<T>
 */

#ifndef AMALGAME_COLLECTIONS_H
#define AMALGAME_COLLECTIONS_H

#include "_runtime.h"
#include <string.h>

/* GC-safe strdup */
static inline char* _am_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char*  r   = (char*) GC_MALLOC(len);
    memcpy(r, s, len);
    return r;
}

/* ================================================================
   List<T> - dynamic array (AmalgameList is defined in _runtime.h)
   ================================================================ */

static inline i64 AmalgameList_size(AmalgameList* l) {
    return l ? (i64) l->size : 0;
}

static inline code_bool AmalgameList_isEmpty(AmalgameList* l) {
    return !l || l->size == 0;
}

static inline void AmalgameList_clear(AmalgameList* l) {
    if (l) l->size = 0;
}

static inline code_bool AmalgameList_contains(AmalgameList* l, void* item) {
    if (!l) return false;
    for (int i = 0; i < l->size; i++)
        if (l->data[i] == item) return true;
    return false;
}

static inline void AmalgameList_addAll(AmalgameList* dst, AmalgameList* src) {
    if (!dst || !src) return;
    for (int i = 0; i < src->size; i++)
        AmalgameList_add(dst, src->data[i]);
}

static inline code_bool AmalgameList_remove(AmalgameList* l, void* item) {
    if (!l) return false;
    for (int i = 0; i < l->size; i++) {
        if (l->data[i] == item) {
            for (int j = i; j < l->size - 1; j++)
                l->data[j] = l->data[j + 1];
            l->size--;
            return true;
        }
    }
    return false;
}

static inline void AmalgameList_removeAt(AmalgameList* l, int index) {
    if (!l || index < 0 || index >= l->size) return;
    for (int j = index; j < l->size - 1; j++)
        l->data[j] = l->data[j + 1];
    l->size--;
}

static inline void AmalgameList_set(AmalgameList* l, int index, void* item) {
    if (!l || index < 0 || index >= l->size) return;
    l->data[index] = item;
}

static inline i64 AmalgameList_indexOf(AmalgameList* l, void* item) {
    if (!l) return -1;
    for (int i = 0; i < l->size; i++)
        if (l->data[i] == item) return (i64) i;
    return -1;
}

static inline AmalgameList* AmalgameList_copy(AmalgameList* l) {
    AmalgameList* r = AmalgameList_new();
    if (l) AmalgameList_addAll(r, l);
    return r;
}

static inline AmalgameList* AmalgameList_reverse(AmalgameList* l) {
    AmalgameList* r = AmalgameList_copy(l);
    if (!r) return r;
    for (int i = 0, j = r->size - 1; i < j; i++, j--) {
        void* tmp  = r->data[i];
        r->data[i] = r->data[j];
        r->data[j] = tmp;
    }
    return r;
}

static inline AmalgameList* AmalgameList_slice(AmalgameList* l,
                                                int start, int end) {
    AmalgameList* r = AmalgameList_new();
    if (!l) return r;
    if (start < 0) start = 0;
    if (end > l->size) end = l->size;
    for (int i = start; i < end; i++)
        AmalgameList_add(r, l->data[i]);
    return r;
}

static inline code_bool AmalgameList_any(AmalgameList* l, AmalgamePredicate fn) {
    if (!l) return false;
    for (int i = 0; i < l->size; i++)
        if (fn(l->data[i])) return true;
    return false;
}

static inline code_bool AmalgameList_all(AmalgameList* l, AmalgamePredicate fn) {
    if (!l) return true;
    for (int i = 0; i < l->size; i++)
        if (!fn(l->data[i])) return false;
    return true;
}

static inline i64 AmalgameList_countIf(AmalgameList* l, AmalgamePredicate fn) {
    if (!l) return 0;
    i64 count = 0;
    for (int i = 0; i < l->size; i++)
        if (fn(l->data[i])) count++;
    return count;
}

/* ================================================================
   Map<K,V> - open-addressing hash map (string keys)
   ================================================================ */

#define AMMAP_INITIAL_CAP 16
#define AMMAP_LOAD_MAX    0.70

typedef struct {
    code_string key;
    void*       value;
    code_bool   used;
} AmalgameMapEntry;

typedef struct _AmalgameMap {
    AmalgameMapEntry* entries;
    int               capacity;
    int               size;
} AmalgameMap;

static unsigned int _ammap_hash(code_string key, int cap) {
    unsigned int h = 2166136261u;
    for (const char* p = key; *p; p++)
        h = (h ^ (unsigned char)*p) * 16777619u;
    return h % (unsigned int) cap;
}

/* Forward declaration for mutual recursion */
static void _ammap_grow(AmalgameMap* m);

static AmalgameMap* AmalgameMap_new() {
    AmalgameMap* m   = (AmalgameMap*) GC_MALLOC(sizeof(AmalgameMap));
    m->capacity      = AMMAP_INITIAL_CAP;
    m->size          = 0;
    m->entries       = (AmalgameMapEntry*) GC_MALLOC(
                         sizeof(AmalgameMapEntry) * AMMAP_INITIAL_CAP);
    memset(m->entries, 0,
           sizeof(AmalgameMapEntry) * AMMAP_INITIAL_CAP);
    return m;
}

static void AmalgameMap_set(AmalgameMap* m, code_string key, void* value) {
    if (!m || !key) return;
    if ((double)(m->size + 1) / m->capacity > AMMAP_LOAD_MAX)
        _ammap_grow(m);

    unsigned int idx = _ammap_hash(key, m->capacity);
    for (int i = 0; i < m->capacity; i++) {
        unsigned int slot = (idx + (unsigned)i) % (unsigned)m->capacity;
        AmalgameMapEntry* e = &m->entries[slot];
        if (!e->used) {
            e->key   = _am_strdup(key);
            e->value = value;
            e->used  = true;
            m->size++;
            return;
        }
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
    }
}

static void _ammap_grow(AmalgameMap* m) {
    int               oldCap     = m->capacity;
    AmalgameMapEntry* oldEntries = m->entries;
    m->capacity *= 2;
    m->entries   = (AmalgameMapEntry*) GC_MALLOC(
                     sizeof(AmalgameMapEntry) * m->capacity);
    memset(m->entries, 0, sizeof(AmalgameMapEntry) * m->capacity);
    m->size = 0;
    for (int i = 0; i < oldCap; i++)
        if (oldEntries[i].used)
            AmalgameMap_set(m, oldEntries[i].key, oldEntries[i].value);
}

static inline void* AmalgameMap_get(AmalgameMap* m, code_string key) {
    if (!m || !key) return NULL;
    unsigned int idx = _ammap_hash(key, m->capacity);
    for (int i = 0; i < m->capacity; i++) {
        unsigned int slot = (idx + (unsigned)i) % (unsigned)m->capacity;
        AmalgameMapEntry* e = &m->entries[slot];
        if (!e->used) return NULL;
        if (strcmp(e->key, key) == 0) return e->value;
    }
    return NULL;
}

static inline code_bool AmalgameMap_has(AmalgameMap* m, code_string key) {
    if (!m || !key) return false;
    unsigned int idx = _ammap_hash(key, m->capacity);
    for (int i = 0; i < m->capacity; i++) {
        unsigned int slot = (idx + (unsigned)i) % (unsigned)m->capacity;
        AmalgameMapEntry* e = &m->entries[slot];
        if (!e->used) return false;
        if (strcmp(e->key, key) == 0) return true;
    }
    return false;
}

static inline code_bool AmalgameMap_remove(AmalgameMap* m, code_string key) {
    if (!m || !key) return false;
    unsigned int idx = _ammap_hash(key, m->capacity);
    for (int i = 0; i < m->capacity; i++) {
        unsigned int slot = (idx + (unsigned)i) % (unsigned)m->capacity;
        AmalgameMapEntry* e = &m->entries[slot];
        if (!e->used) return false;
        if (strcmp(e->key, key) == 0) {
            e->used = false;
            m->size--;
            return true;
        }
    }
    return false;
}

static inline i64 AmalgameMap_size(AmalgameMap* m) {
    return m ? (i64) m->size : 0;
}

static inline code_bool AmalgameMap_isEmpty(AmalgameMap* m) {
    return !m || m->size == 0;
}

static inline AmalgameList* AmalgameMap_keys(AmalgameMap* m) {
    AmalgameList* l = AmalgameList_new();
    if (!m) return l;
    for (int i = 0; i < m->capacity; i++)
        if (m->entries[i].used)
            AmalgameList_add(l, (void*) m->entries[i].key);
    return l;
}

static inline AmalgameList* AmalgameMap_values(AmalgameMap* m) {
    AmalgameList* l = AmalgameList_new();
    if (!m) return l;
    for (int i = 0; i < m->capacity; i++)
        if (m->entries[i].used)
            AmalgameList_add(l, m->entries[i].value);
    return l;
}

/* ================================================================
   Set<T> - unique string values (backed by AmalgameMap)
   ================================================================ */

typedef struct {
    AmalgameMap* map;
} AmalgameSet;

static inline AmalgameSet* AmalgameSet_new() {
    AmalgameSet* s = (AmalgameSet*) GC_MALLOC(sizeof(AmalgameSet));
    s->map = AmalgameMap_new();
    return s;
}

static inline code_bool AmalgameSet_add(AmalgameSet* s, code_string item) {
    if (!s || !item) return false;
    if (AmalgameMap_has(s->map, item)) return false;
    AmalgameMap_set(s->map, item, (void*) 1);
    return true;
}

static inline code_bool AmalgameSet_contains(AmalgameSet* s, code_string item) {
    return s && AmalgameMap_has(s->map, item);
}

static inline code_bool AmalgameSet_remove(AmalgameSet* s, code_string item) {
    return s && AmalgameMap_remove(s->map, item);
}

static inline i64 AmalgameSet_size(AmalgameSet* s) {
    return s ? AmalgameMap_size(s->map) : 0;
}

static inline code_bool AmalgameSet_isEmpty(AmalgameSet* s) {
    return !s || AmalgameMap_isEmpty(s->map);
}

static inline AmalgameList* AmalgameSet_toList(AmalgameSet* s) {
    return s ? AmalgameMap_keys(s->map) : AmalgameList_new();
}

#endif /* AMALGAME_COLLECTIONS_H */
