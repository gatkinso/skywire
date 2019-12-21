#include <string>
#include <Python.h>

static PyObject *generatorError;
static PyObject* libgenerator_setcfg(PyObject *self, PyObject *args);
static PyObject* libgenerator_request(PyObject *self, PyObject *args);
static PyObject* libgenerator_response(PyObject *self, PyObject *args);

static std::string cfgfilename("generator.json");

static PyMethodDef GeneratorMethods[] = {
    {"set_config", libgenerator_setcfg, METH_VARARGS, "Set the configuration"},
    {nullptr, nullptr, 0, nullptr} /* Sentinel */
};

static struct PyModuleDef generatormodule = {
    PyModuleDef_HEAD_INIT,
    "libgenerator", /* name of module */
    nullptr,        /* module documentation, may be nullptr */
    -1,             /* size of per-interpreter state of the module,
                       or -1 if the module keeps state in global variables. */
    GeneratorMethods
};

/////////////////////////////////////////////////////////////

PyMODINIT_FUNC PyInit_libgenerator(void)
{
    PyObject *module;

    module = PyModule_Create(&generatormodule);
    if (module == nullptr)
        return nullptr;

    generatorError = PyErr_NewException((char*)"libgenerator.error", nullptr, nullptr);
    Py_INCREF(generatorError);
    PyModule_AddObject(module, "error", generatorError);

    return module;
}

static PyObject* libgenerator_setcfg(PyObject *self, PyObject *args)
{
    const char* filename = nullptr;

    if (!PyArg_ParseTuple(args, "s", &filename))
        Py_RETURN_NONE;
    
    if (nullptr != filename)
        cfgfilename = std::string(filename);
    else
        Py_RETURN_NONE;

    Py_RETURN_NONE;
}

// static PyObject* libgenerator_generate(PyObject *self, PyObject *args)
// {
//     std::lock_guard<std::mutex> lock(mtx_in);
//     position_t position;

//     if (last_image.empty())
//     {
//         Py_RETURN_NONE;
//     }

//     PyObject *val = Py_BuildValue("s",
//         last_image.c_str());

//     last_image.clear();

//     return val;
// }
