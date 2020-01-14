/****************************************************************************
** Meta object code from reading C++ file 'basic4gltextedit.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../basic4gltextedit.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'basic4gltextedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Basic4GLTextEdit_t {
    QByteArrayData data[8];
    char stringdata0[122];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Basic4GLTextEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Basic4GLTextEdit_t qt_meta_stringdata_Basic4GLTextEdit = {
    {
QT_MOC_LITERAL(0, 0, 16), // "Basic4GLTextEdit"
QT_MOC_LITERAL(1, 17, 25), // "updateLineNumberAreaWidth"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 13), // "newBlockCount"
QT_MOC_LITERAL(4, 58, 20), // "updateLineNumberArea"
QT_MOC_LITERAL(5, 79, 14), // "HighlightLines"
QT_MOC_LITERAL(6, 94, 16), // "insertCompletion"
QT_MOC_LITERAL(7, 111, 10) // "completion"

    },
    "Basic4GLTextEdit\0updateLineNumberAreaWidth\0"
    "\0newBlockCount\0updateLineNumberArea\0"
    "HighlightLines\0insertCompletion\0"
    "completion"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Basic4GLTextEdit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x08 /* Private */,
       4,    2,   37,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    1,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    2,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,

       0        // eod
};

void Basic4GLTextEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Basic4GLTextEdit *_t = static_cast<Basic4GLTextEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateLineNumberAreaWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->updateLineNumberArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->HighlightLines(); break;
        case 3: _t->insertCompletion((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Basic4GLTextEdit::staticMetaObject = {
    { &QPlainTextEdit::staticMetaObject, qt_meta_stringdata_Basic4GLTextEdit.data,
      qt_meta_data_Basic4GLTextEdit,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Basic4GLTextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Basic4GLTextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Basic4GLTextEdit.stringdata0))
        return static_cast<void*>(const_cast< Basic4GLTextEdit*>(this));
    return QPlainTextEdit::qt_metacast(_clname);
}

int Basic4GLTextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
