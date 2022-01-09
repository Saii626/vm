
// https://raw.githubusercontent.com/eteran/c-vector/master/cvector.h


#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <assert.h> /* for assert */
#include <stdlib.h> /* for malloc/realloc/free */
#include <string.h> /* for memcpy/memmove */

/**
 * @brief vector_type - The vector type used in this library
 */
#define vector_type(type) type *

/**
 * @brief vector_capacity - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define vector_capacity(vec)                                                                                          \
    ((vec) ? ((size_t *) (vec))[-1] : (size_t) 0)                                                                      \

/**
 * @brief vector_size - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define vector_size(vec)                                                                                              \
    ((vec) ? ((size_t *) (vec))[-2] : (size_t) 0)                                                                      \

/**
 * @brief vector_empty - returns non-zero if the vector is empty
 * @param vec - the vector
 * @return non-zero if empty, zero if non-empty
 */
#define vector_empty(vec)                                                                                             \
  (vector_size(vec) == 0)                                                                                             \

/**
 * @brief vector_erase - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define vector_erase(vec, i)                                                                                          \
    do {                                                                                                               \
        if ((vec)) {                                                                                                   \
            const size_t cv_sz = vector_size(vec);                                                                    \
            if ((i) < cv_sz) {                                                                                         \
                vector_set_size((vec), cv_sz - 1);                                                                    \
                memmove((vec) + (i), (vec) + (i) + 1, sizeof(*(vec)) * (cv_sz - 1 - (i)));                             \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)                                                                                                        \

/**
 * @brief vector_free - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define vector_free(vec)                                                                                              \
    do {                                                                                                               \
        if ((vec)) {                                                                                                   \
            size_t *p1 = &((size_t *) (vec))[-2];                                                                      \
            free(p1);                                                                                                  \
        }                                                                                                              \
    } while (0)                                                                                                        \

/**
 * @brief vector_begin - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element (or NULL)
 */
#define vector_begin(vec)                                                                                             \
    (vec)                                                                                                              \

/**
 * @brief vector_end - returns an iterator to one past the last element of the vector
 * @param vec - the vector
 * @return a pointer to one past the last element (or NULL)
 */
#define vector_end(vec)                                                                                               \
    ((vec) ? &((vec)[vector_size(vec)]): NULL)                                                                        \

/* user request to use logarithmic growth algorithm */
#ifdef VECTOR_LOGARITHMIC_GROWTH

/**
 * @brief vector_push_back - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define vector_push_back(vec, value)                                                                                  \
    do {                                                                                                               \
        size_t cv_cap = vector_capacity(vec);                                                                         \
        if (cv_cap <= vector_size(vec)) {                                                                             \
            vector_grow((vec), cv_cap ? (cv_cap << 1) : 1);                                                           \
        }                                                                                                              \
        (vec)[vector_size(vec)] = (value);                                                                              \
        vector_set_size((vec), vector_size(vec) + 1);                                                                \
    } while (0)                                                                                                        \

/**
 * @brief vector_insert - insert element at position pos to the vector
 * @param vec - the vector
 * @param pos - position in the vector where the new elements are inserted.
 * @param val - value to be copied (or moved) to the inserted elements.
 * @return void
 */
#define vector_insert(vec, pos, val)                                                                                  \
    do {                                                                                                               \
        if (vector_capacity(vec) <= vector_size(vec) + 1) {                                                          \
            vector_grow((vec), vector_capacity(vec) ? (vector_capacity(vec) << 1) : 1);                             \
        }                                                                                                              \
        if (pos < vector_size(vec)) {                                                                                 \
            memmove((vec) + (pos) + 1, (vec) + (pos), sizeof(*(vec)) * ((vector_size(vec) + 1) - (pos)));             \
        }                                                                                                              \
        (vec)[(pos)] = (val);                                                                                          \
        vector_set_size((vec), vector_size(vec) + 1);                                                                \
    } while (0)                                                                                                        \

#else

/**
 * @brief vector_push_back - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define vector_push_back(vec, value)                                                                                  \
    do {                                                                                                               \
        size_t cv_cap = vector_capacity(vec);                                                                         \
        if (cv_cap <= vector_size(vec)) {                                                                             \
            vector_grow((vec), cv_cap + 1);                                                                           \
        }                                                                                                              \
        (vec)[vector_size(vec)] = (value);                                                                              \
        vector_set_size((vec), vector_size(vec) + 1);                                                                \
    } while (0)                                                                                                        \

/**
 * @brief vector_insert - insert element at position pos to the vector
 * @param vec - the vector
 * @param pos - position in the vector where the new elements are inserted.
 * @param val - value to be copied (or moved) to the inserted elements.
 * @return void
 */
#define vector_insert(vec, pos, val)                                                                                  \
    do {                                                                                                               \
        if (vector_capacity(vec) <= vector_size(vec) + 1) {                                                          \
            vector_grow((vec), vector_size(vec) + 1);                                                                \
        }                                                                                                              \
        if (pos < vector_size(vec)) {                                                                                 \
            memmove((vec) + (pos) + 1, (vec) + (pos), sizeof(*(vec)) * ((vector_size(vec) + 1) - (pos)));             \
        }                                                                                                              \
        (vec)[(pos)] = (val);                                                                                          \
        vector_set_size((vec), vector_size(vec) + 1);                                                                \
    } while (0)                                                                                                        \

#endif /* VECTOR_LOGARITHMIC_GROWTH */

/**
 * @brief vector_pop_back - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define vector_pop_back(vec)                                                                                          \
    do {                                                                                                               \
        vector_set_size((vec), vector_size(vec) - 1);                                                                \
    } while (0)                                                                                                        \

/**
 * @brief vector_copy - copy a vector
 * @param from - the original vector
 * @param to - destination to which the function copy to
 * @return void
 */
#define vector_copy(from, to)                                                                                         \
    do {                                                                                                               \
        if ((from)) {                                                                                                  \
            vector_grow(to, vector_size(from));                                                                      \
            vector_set_size(to, vector_size(from));                                                                  \
            memcpy((to), (from), vector_size(from) * sizeof(*(from)));                                                \
        }                                                                                                              \
    } while (0)                                                                                                        \

/**
 * @brief vector_set_capacity - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_capacity(vec, size)                                                                                \
    do {                                                                                                               \
        if ((vec)) {                                                                                                   \
            ((size_t *) (vec))[-1] = (size);                                                                           \
        }                                                                                                              \
    } while (0)                                                                                                        \

/**
 * @brief vector_set_size - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_size(vec, size)                                                                                    \
    do {                                                                                                               \
        if ((vec)) {                                                                                                   \
            ((size_t *) (vec))[-2] = (size);                                                                           \
        }                                                                                                              \
    } while (0)                                                                                                        \

/**
 * @brief vector_grow - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param count - the new capacity to set
 * @return void
 */
#define vector_grow(vec, count)                                                                                       \
    do {                                                                                                               \
        const size_t cv_sz = (count) * sizeof(*(vec)) + (sizeof(size_t) * 2);                                          \
        if ((vec)) {                                                                                                   \
            size_t *cv_p1 = &((size_t *) (vec))[-2];                                                                   \
            size_t *cv_p2 = realloc(cv_p1, (cv_sz));                                                                   \
            assert(cv_p2);                                                                                             \
            (vec) = (void *) (&cv_p2[2]);                                                                              \
            vector_set_capacity((vec), (count));                                                                      \
        } else {                                                                                                       \
            size_t *cv_p = malloc(cv_sz);                                                                              \
            assert(cv_p);                                                                                              \
            (vec) = (void *) (&cv_p[2]);                                                                               \
            vector_set_capacity((vec), (count));                                                                      \
            vector_set_size((vec), 0);                                                                                \
        }                                                                                                              \
    } while (0)                                                                                                        \

#endif /* __VECTOR_H__ */

