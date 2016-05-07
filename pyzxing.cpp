#include <Python.h>
#include "cli/src/ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>
using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;
vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
    Ref<Reader> reader(new MultiFormatReader);
    return vector<Ref<Result> >(1, reader->decode(image, hints));
}
vector<Ref<Result> > decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
    MultiFormatReader delegate;
    GenericMultipleBarcodeReader reader(delegate);
    return reader.decodeMultiple(image, hints);
}
static PyObject *_zxing_error;
static PyObject * decode(PyObject *self, PyObject *args) {
    const char *data;
    int width, height, comps, len;
    bool hybrid = false;
    bool search_multi = false;
    if (!PyArg_ParseTuple(
                args, "iiis#", &width, &height, &comps, &data, &len)) {
        return NULL;
    }
    if (len != width * height * comps) {
        PyErr_SetString(_zxing_error, "Invalid data size. Expected width * height * channels.");
        return NULL;
    }
    zxing::ArrayRef<char> image = zxing::ArrayRef<char>(len);
    memcpy(&image[0], &data[0], len);
    Ref<LuminanceSource> source(new ImageReaderSource(image, width, height, comps));
    vector<Ref<Result> > results;
    string cell_result;
    try {
        Ref<Binarizer> binarizer;
        if (hybrid) {
            binarizer = new HybridBinarizer(source);
        } else {
            binarizer = new GlobalHistogramBinarizer(source);
        }
        DecodeHints hints(DecodeHints::DEFAULT_HINT);
        // hints.setTryHarder(try_harder);
        Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
        if (search_multi) {
            results = decode_multi(binary, hints);
        } else {
            results = decode(binary, hints);
        }
        if (results.size()) {
            const string& s = results.front()->getText()->getText();
            return Py_BuildValue("s#", s.c_str(), s.size());
        }
    } catch (const ReaderException& e) {
        cell_result = "zxing::ReaderException: " + string(e.what());
        PyErr_SetString(_zxing_error, cell_result.c_str());
        return NULL;
    } catch (const zxing::IllegalArgumentException& e) {
        cell_result = "zxing::IllegalArgumentException: " + string(e.what());
        PyErr_SetString(_zxing_error, cell_result.c_str());
        return NULL;
    } catch (const zxing::Exception& e) {
        cell_result = "zxing::Exception: " + string(e.what());
        PyErr_SetString(_zxing_error, cell_result.c_str());
        return NULL;
    } catch (const std::exception& e) {
        cell_result = "std::exception: " + string(e.what());
        PyErr_SetString(_zxing_error, cell_result.c_str());
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}
static PyMethodDef methods[] = {
    {
        "decode",  decode, METH_VARARGS,
        "decode(width, height, channels, data).\n"
        "Decode QR code."
    },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};
PyMODINIT_FUNC
initzxing(void) {
    PyObject* m;
    m = Py_InitModule("zxing", methods);
    if (!m)
        return;
    _zxing_error = PyErr_NewException("zxing.error", NULL, NULL);
    Py_INCREF(_zxing_error);
    PyModule_AddObject(m, "error", _zxing_error);
}

