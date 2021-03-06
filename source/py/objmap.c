/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include "mp_assert.h"

#include "py/runtime.h"

typedef struct _mp_obj_map_t {
    mp_obj_base_t base;
    size_t n_iters;
    mp_obj_t fun;
    mp_obj_t iters[];
} mp_obj_map_t;

STATIC mp_obj_t map_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, MP_OBJ_FUN_ARGS_MAX, false);
    RETURN_ON_EXCEPTION(MP_OBJ_NULL)
    mp_obj_map_t *o = m_new_obj_var(mp_obj_map_t, mp_obj_t, n_args - 1);
    RETURN_ON_EXCEPTION(MP_OBJ_NULL)
    o->base.type = type;
    o->n_iters = n_args - 1;
    o->fun = args[0];
    m_rs_push_ptr(o);
    for (size_t i = 0; i < n_args - 1; i++) {
        o->iters[i] = mp_getiter(args[i + 1], NULL);
        RETURN_ON_EXCEPTION(MP_OBJ_NULL)
    }
    m_rs_pop_ptr(o);
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t map_iternext(mp_obj_t self_in) {
    mp_check_self(MP_OBJ_IS_TYPE(self_in, &mp_type_map));
    mp_obj_map_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t *nextses = m_new(mp_obj_t, self->n_iters);
    RETURN_ON_EXCEPTION(MP_OBJ_NULL)
    m_rs_push_ptr(nextses);

    for (size_t i = 0; i < self->n_iters; i++) {
        mp_obj_t next = mp_iternext(self->iters[i]);
        RETURN_ON_EXCEPTION(MP_OBJ_NULL)
        if (next == MP_OBJ_STOP_ITERATION) {
            m_del(mp_obj_t, nextses, self->n_iters);
            m_rs_pop_ptr(nextses);
            return MP_OBJ_STOP_ITERATION;
        }
        nextses[i] = next;
    }
    mp_obj_t o = mp_call_function_n_kw(self->fun, self->n_iters, 0, nextses);
    m_rs_pop_ptr(nextses);
    return o;
}

const mp_obj_type_t mp_type_map = {
    { &mp_type_type },
    .name = MP_QSTR_map,
    .make_new = map_make_new,
    .getiter = mp_identity_getiter,
    .iternext = map_iternext,
};
