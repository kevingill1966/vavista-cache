
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

/* Python */
#include <Python.h>
#include "structmember.h"

/* Cache */
#include "callin.h"

#define MAXMSG 2048     /* max length of a Cache message  - todo verify */
#define MAXVAL 1048576  /* max length of a value Cache can return - todo verify */

static char msgbuf[MAXMSG];
static int gInitialised=0;

static PyObject *CacheException;

/*--------------------------------------------------------------------------------*/
/* INOUT is a marker class which flag a parameter as being output                 */

typedef struct {
    PyObject_HEAD
    PyObject *value;
} INOUT;

static void
INOUT_dealloc(INOUT* self)
{
    if (self->value) {
        Py_XDECREF(self->value);
    }
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
INOUT_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    INOUT *self;

    self = (INOUT *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->value = PyString_FromString("");
        if (self->value == NULL)
        {
            Py_DECREF(self);
            return NULL;
        }
        return (PyObject *)self;
    }
    return NULL;
}

static int
INOUT_init(INOUT *self, PyObject *args, PyObject *kwds)
{
    PyObject *value=NULL, *tmp=NULL;

    if (!PyArg_ParseTuple(args, "O:init", &value))
        return -1;

    if (value) {
        tmp = self->value;
        Py_INCREF(value);
        self->value = value;
        Py_XDECREF(tmp);
    }

    return 0;
}


static PyMethodDef INOUT_methods[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef INOUT_members[] = {
    {"value", T_OBJECT_EX, offsetof(INOUT, value), 0, "value"},
    {NULL}  /* Sentinel */
};


static PyTypeObject INOUT_type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_gtm.INOUT",             /*tp_name*/
    sizeof(INOUT), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)INOUT_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "INOUT flags a parameter as being returned",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    INOUT_methods,             /* tp_methods */
    INOUT_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)INOUT_init,      /* tp_init */
    0,                         /* tp_alloc */
    INOUT_new,                 /* tp_new */
};

/*--------------------------------------------------------------------------------*/
int
is_error(int rv, char *module){
    if (rv == 0) {
        return 0;
    } else {
        CACHE_ASTR errmsg;
        CACHE_ASTR srcline;
        int offset;
        int rc;

        errmsg.len = 50;
        srcline.len = 100;
        if ((rc = CacheErrorA(&errmsg, &srcline, &offset)) != CACHE_SUCCESS) {
            printf("%s: failed to display error - rc = %d\n", module, rc);
            sprintf(msgbuf, "%s: Cache returned error %d", module, rv);
        } else {
            sprintf(msgbuf, "%s: %s", module, errmsg.str);
        }
        PyErr_SetString(CacheException, msgbuf);
        return 1;
    }
    
}

/*--------------------------------------------------------------------------------*/

void mstop(void) {
    int status = 0;
    if (gInitialised) {
        gInitialised = 0;
        status = CacheEnd();
    }
}


static void *
mstart(void) {
    int status = 0;
    if (! gInitialised) {
        CACHE_ASTR prinp, prout, exename;

        CacheSetDir("/opt/cache/mgr");
        /* CacheSetDir("/usr/local/etc/cachesys"); */

        prinp.len = prout.len = 0;
        prinp.str[0] = prout.str[0] = '\0';
        strcpy((char *)exename.str, "vavista.cache");
        exename.len = strlen((const char *)exename.str);

        status = CacheSecureStartA(
            NULL,                             /* User */
            NULL,                             /* Password */
            &exename,                          /* Application Id */
            CACHE_PROGMODE | CACHE_TTNEVER,   /* Flags */
            0,                                /* timeout */
            &prinp,                        /* Principal input device */
            &prout);                       /* Principal output device */

        if (is_error(status, "mstart")) {
            return NULL;
        }
        atexit(mstop);
        gInitialised = 1;
    }
    return (void *)1; /* Success */
}

static int
_push_global(PyObject *gl)
{
    /*
     * Push the global described in a List *gl.
     * return -1 for an exception.
    */
    int status;
    PyObject *item;
    int i, ss_count;
    unsigned char *str;

    /* Split out subscripts */
    status = PySequence_Check(gl);
    if (status == 0) {
        sprintf(msgbuf, "_cache.mget() : parameter must be a list");
        PyErr_SetString(CacheException, msgbuf);
        return -1;
    }

    item = PySequence_GetItem(gl, 0);
    str = (unsigned char *)PyString_AsString(item);

    status = CachePushGlobal(strlen((const char *)str), str);
    if (is_error(status, "mget")) {
        return -1;
    }
    ss_count = PySequence_Size(gl) -1;

    for (i=0; i< ss_count; i++) {
        item = PySequence_GetItem(gl, i+1);
        str = (unsigned char *)PyString_AsString(item);
        status = CachePushStr(strlen((const char *)str), (const unsigned char *)str);
        if (is_error(status, "mget")) {
            return -1;
        }
    }
    return ss_count;
}


static PyObject*
Cache_mget(PyObject *self, PyObject *args)
{
    int status = 0;
    int len;
    unsigned char *p;
    PyObject *gl;
    int ss_count;

    if (mstart() == NULL) return NULL;

    if (!PyArg_ParseTuple(args, "O", &gl)) {
        sprintf(msgbuf, "mget requires a mumps variable name");
        PyErr_SetString(CacheException, msgbuf);
        return NULL;
    }

    ss_count = status = _push_global(gl);
    if (status == -1) {
        return NULL;
    }

    status = CacheGlobalGet(ss_count, 0);
    if (is_error(status, "mget")) {
        return NULL;
    }

    status = CachePopStr(&len, &p);
    if (is_error(status, "mget")) {
        return NULL;
    }

    return Py_BuildValue("s#", p, len);
}

static PyObject*
Cache_mset(PyObject *self, PyObject *args)
{
    char *value;
    int status;
    PyObject *gl;
    int ss_count;

    if (mstart() == NULL) return NULL;

    if (!PyArg_ParseTuple(args, "Os", &gl, &value)) {
        sprintf(msgbuf, "mset requires a mumps variable name and a value");
        PyErr_SetString(CacheException, msgbuf);
        return NULL;
    }

    ss_count = status = _push_global(gl);
    if (status == -1) {
        return NULL;
    }
/*
    status = CachePushGlobal(strlen((const char *)varname), varname);
    if (is_error(status, "mset")) {
        return NULL;
    }
    */
    status = CachePushStr(strlen(value), (void *)value);
    if (is_error(status, "mset")) {
        return NULL;
    }
    status = CacheGlobalSet(ss_count);
    if (is_error(status, "mset")) {
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef CacheMethods[] = {
    {"mget",   Cache_mget,   METH_VARARGS, "Get a mumps variable."},
    {"mset",   Cache_mset,   METH_VARARGS, "Set a mumps variable."},
    {NULL,     NULL,         0,            NULL}        /* Sentinel */
};

PyMODINIT_FUNC
init_cache(void) {
    PyObject *m;
    m = Py_InitModule("_cache", CacheMethods);

    CacheException = PyErr_NewException("_cache.error", NULL, NULL);
    Py_INCREF(CacheException);
    PyModule_AddObject(m, "error", CacheException);

    INOUT_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&INOUT_type) < 0)
        return;
    Py_INCREF(&INOUT_type);
    PyModule_AddObject(m, "INOUT", (PyObject *)&INOUT_type);
}

