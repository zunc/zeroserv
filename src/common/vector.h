/*
 * Copyright (C) 2015
 *
 *
 * Written by Raphael S. Carvalho <raphael.scarv@gmail.com>
 * Custom by Khoai <dungcoivb@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct vector {
    void *data;
    float growth_factor;
    int type_size;
    int size;
    int capacity;
} vector;

#define ELEM(i) ((char *) (v->data) + (i) * (v->type_size))

static vector vector_create(int initial_capacity, float growth_factor) {
    assert(growth_factor > 1.0f);
    vector new = {
        .data = malloc(initial_capacity * sizeof (void*)),
        .growth_factor = growth_factor,
        .type_size = sizeof (void*),
        .size = 0,
        .capacity = initial_capacity,
    };
    assert(new.data != NULL);
    return new;
}

static void vector_destroy(vector *v) {
    free(v->data);
    memset((void *) v, 0, sizeof (vector));
}

static void *vector_access(vector *v, int i) {
    assert(i >= 0 && i < v->capacity);
    unsigned offset = i * v->type_size;
    void** p = v->data + offset;
    return *(void**) p;
}

static int vector_size(vector *v) {
    return v->size;
}

static void *vector_insert(vector *v, void *object) {
    assert(v->capacity > 0);
    assert(v->size <= v->capacity);
    if (v->size == v->capacity) {
        // increase vector capacity by growth factor.
        int new_capacity = (int) (v->capacity * v->growth_factor + 1);
        v->data = realloc(v->data, new_capacity * v->type_size);
        assert(v->data != NULL);
        v->capacity = new_capacity;
    }

    int type_size = v->type_size;
    unsigned offset = v->size * type_size;
    void *dest = (void *) ((char *) v->data + offset);
    assert(offset + type_size <= v->capacity * type_size);
    if (object) {
        *(void**) dest = object;
    }
    v->size++;
    return dest;
}

static int vector_remove(vector *v, void *object) {
    // remove by pointer
    assert(v);
    assert(v->capacity > 0);
    int idx;
    for (idx = 0; idx < v->size; idx++) {
        void *pointer = ELEM(idx);
        if (*(void**) pointer == object) {
            memmove(ELEM(idx), ELEM(idx + 1), (v->size-- - idx) * v->type_size);
            return 0;
        }
    }
    return 1;
}

static int vector_footprint(vector *v) {
    return v->size * v->type_size;
}

#endif