#include <Python.h>
#include <fbp.h>


static PyObject* FixedBytePair_encode_wrapper(
        PyObject* self,
        PyObject* args,
        PyObject* kwargs) {
    int len;
    PyObject* ret;
    const char* src;
    const char* name;
    uint8_t* dst = NULL;
    FixedBytePairState_t* state = NULL;
    static char* kwlist[] = {"src", "name", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|s", kwlist, &src, &name)) {
        Py_RETURN_NONE;
    }

    state = FixedBytePair_get(name);
    if (state == NULL) {
        Py_RETURN_NONE;
    }

    len = FixedBytePair_encode(state, dst, 0, src);
    if (len <= 0) {
        Py_RETURN_NONE;
    }

    dst = malloc(len);
    if (dst == NULL) {
        Py_RETURN_NONE;
    }

    len = FixedBytePair_encode(state, dst, 0, src);

    ret = PyBytes_FromString(dst);
    free(dst);
    return ret;
}

static PyObject* FixedBytePair_decode_wrapper(
        PyObject* self,
        PyObject* args,
        PyObject* kwargs) {
    int len;
    PyObject* ret;
    const char* src;
    const char* name;
    char* dst = NULL;
    FixedBytePairState_t* state = NULL;
    static char* kwlist[] = {"src", "name", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y|s", kwlist, &src, &name)) {
        Py_RETURN_NONE;
    }

    state = FixedBytePair_get(name);
    if (state == NULL) {
        Py_RETURN_NONE;
    }

    len = FixedBytePair_decode(state, dst, 0, src);
    if (len <= 0) {
        Py_RETURN_NONE;
    }

    dst = malloc(len);
    if (dst == NULL) {
        Py_RETURN_NONE;
    }
    
    len = FixedBytePair_decode(state, dst, 0, src);

    ret = PyUnicode_FromString(dst);
    free(dst);
    return ret;
}

static PyMethodDef FbpMethods[] = {
    {
        "encode", FixedBytePair_encode_wrapper, METH_VARARGS,
        "Encode a Fixed Byte Pair string from ascii\n"
        "\n"
        "@param string text to encode\n"
        "@return encoded bytestream or None on failure\n"
    },
    {
        "decode", FixedBytePair_decode_wrapper, METH_VARARGS,
        "Decode a Fixed Byte Pair encoded string to ascii\n"
        "\n"
        "@param bytestream data to decode\n"
        "@return decoded ascii string or None on failure\n"
    },
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initfbp(void) {
    (void) Py_InitModule("fbp", FbpMethods);
}
