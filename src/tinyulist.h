#ifndef TDS_ULIST_H_

#include "stdlib.h"
#include "string.h"

struct UListBase
{
    uint8_t* data;
    size_t count;
    size_t capacity_bytes;
    size_t used_bytes;
};

static int ulist_create(struct UListBase* ulist, size_t count, size_t element_size)
{
    ulist->used_bytes = 0;
    ulist->capacity_bytes = 0;
    ulist->data = NULL;

    if (count > 0)
    {
        ulist->data = malloc(count * element_size);
        if (ulist->data == NULL)
            return -1;
        ulist->capacity_bytes = count * element_size;
    }

    return 0;
}

static int ulist_destroy(struct UListBase* ulist)
{
    free(ulist->data);
    ulist->data = NULL;
    ulist->capacity_bytes = 0;
    ulist->used_bytes = 0;
    return 0;
}

static int ulist_reserve(struct UListBase* ulist, size_t capacity_bytes)
{
    void* new_data = realloc(ulist->data, capacity_bytes);
    if (new_data == NULL)
        return -1;
    ulist->data = new_data;
    ulist->capacity_bytes = capacity_bytes;
    ulist->used_bytes = capacity_bytes;
    return 0;
}

//static int ulist_add(struct UListBase* ulist, void* data, size_t count, size_t element_size)
//{
//    return 0;
//}

static int ulist_add_one(struct UListBase* ulist, void* data, size_t element_size)
{
    if (ulist->capacity_bytes <= ulist->used_bytes + element_size)
    {
        size_t new_size = ulist->capacity_bytes > 0 ? ulist->capacity_bytes * 2 : 1;
        int sts = ulist_reserve(ulist, new_size);
        if (sts != 0)
            return sts;
    }
    memcpy(ulist->data + ulist->used_bytes, data, element_size);
    ulist->used_bytes += element_size;
    ulist->count++;
    return 0;
}

static size_t ulist_count(struct UListBase* ulist, size_t element_size)
{
    return ulist->used_bytes / element_size;
}

static int ulist_remove_one(struct UListBase* ulist, size_t index, size_t element_size)
{
    void* dst = ulist->data + index * element_size;
    size_t count = ulist->used_bytes / element_size;
    memcpy(dst, ulist->data + (count - 1) * element_size, element_size);
    ulist->used_bytes -= element_size;
    ulist->count --;
    return 0;
}

static int ulist_relax(struct UListBase* ulist)
{
    size_t suggested_size = ulist->capacity_bytes / 2;
    if (suggested_size > 0 && ulist->used_bytes <= suggested_size)
    {
        return ulist_reserve(ulist, suggested_size);
    }
    return 0;
}

static int ulist_create_copy(struct UListBase* from, struct UListBase* to)
{
    ulist_create(to, from->used_bytes, 1);
    memcpy(from->data, to->data, from->used_bytes);
    to->used_bytes = from->used_bytes;
    to->count = from->count;
}

static int ulist_pop(struct UListBase* list, size_t element_size)
{
    if (list->used_bytes >= element_size)
    {
        list->used_bytes -= element_size;
        list->count--;
        return 0;
    }
    else
    {
        return -1;
    }
}

/*
#define TDS_ULIST_DECL(SOCKET, name)                                           \
    union UList_soc;                                                      \
    static int ulist_soc_create(union UList_soc* ulist, size_t count); \
    static int ulist_soc_destroy(union UList_soc* ulist);              \
    static int ulist_soc_add(union UList_soc* ulist, SOCKET data);       \
    static int ulist_soc_remove(union UList_soc* ulist, size_t index); \
    static int ulist_soc_resize(union UList_soc* ulist, size_t count); \
    static int ulist_soc_relax(union UList_soc* ulist);                \
    static size_t ulist_soc_count(union UList_soc* ulist);
// TODO: missing declarations
*/

typedef unsigned long long SOCKET;

#define TDS_ULIST_IMPL(name, type)

typedef union
{
    struct UListBase _base;
    unsigned int* data;
    size_t count; // Read-only
} UList_soc;

static inline int ulist_soc_create(UList_soc* ulist, size_t count)
{
    return ulist_create(&ulist->_base, count, sizeof(SOCKET));
}
static inline int ulist_soc_create_copy(UList_soc* from, UList_soc* to)
{
    return ulist_create_copy(&from->_base, &to->_base);
}
static inline int ulist_soc_destroy(UList_soc* ulist)
{
    return ulist_destroy(&ulist->_base);
}
static inline int ulist_soc_add_one(UList_soc* ulist, SOCKET data)
{
    return ulist_add_one(&ulist->_base, (void*)&data, sizeof(SOCKET));
}
//static inline int ulist_soc_add(UList_soc* ulist, SOCKET* data, size_t count)
//{
//    return ulist_add(&ulist->_base, (void*)data, count, sizeof(SOCKET));
//}
static inline int ulist_soc_remove_one(UList_soc* ulist, size_t index)
{
    return ulist_remove_one(&ulist->_base, index, sizeof(SOCKET));
}
static inline int ulist_soc_pop(UList_soc* list)
{
    return ulist_pop(&list->_base, sizeof(SOCKET));
}
static inline size_t ulist_soc_count(UList_soc* ulist)
{
    return ulist_count(&ulist->_base, sizeof(SOCKET));
}
static inline int ulist_soc_reserve(UList_soc* ulist, size_t count)
{
    return ulist_reserve(&ulist->_base, count * sizeof(SOCKET));
}
static inline int ulist_soc_relax(UList_soc* ulist)
{
    return ulist_relax(&ulist->_base);
}
#endif
