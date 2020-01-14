/****************************************************************************
** Meta object code from reading C++ file 'virtualmachineviewdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../virtualmachineviewdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'virtualmachineviewdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_VirtualMachineViewDialog_t {
    QByteArrayData data[8];
    char stringdata0[167];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VirtualMachineViewDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VirtualMachineViewDialog_t qt_meta_stringdata_VirtualMachineViewDialog = {
    {
QT_MOC_LITERAL(0, 0, 24), // "VirtualMachineViewDialog"
QT_MOC_LITERAL(1, 25, 21), // "on_stepButton_clicked"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 26), // "on_codeScroll_valueChanged"
QT_MOC_LITERAL(4, 75, 5), // "value"
QT_MOC_LITERAL(5, 81, 30), // "on_variableScroll_valueChanged"
QT_MOC_LITERAL(6, 112, 26), // "on_heapScroll_valueChanged"
QT_MOC_LITERAL(7, 139, 27) // "on_stackScroll_valueChanged"

    },
    "VirtualMachineViewDialog\0on_stepButton_clicked\0"
    "\0on_codeScroll_valueChanged\0value\0"
    "on_variableScroll_valueChanged\0"
    "on_heapScroll_valueChanged\0"
    "on_stackScroll_valueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VirtualMachineViewDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    1,   40,    2, 0x08 /* Private */,
       5,    1,   43,    2, 0x08 /* Private */,
       6,    1,   46,    2, 0x08 /* Private */,
       7,    1,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

void VirtualMachineViewDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        VirtualMachineViewDialog *_t = static_cast<VirtualMachineViewDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_stepButton_clicked(); break;
        case 1: _t->on_codeScroll_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_variableScroll_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_heapScroll_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->on_stackScroll_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject VirtualMachineViewDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_VirtualMachineViewDialog.data,
      qt_meta_data_VirtualMachineViewDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *VirtualMachineViewDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VirtualMachineViewDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_VirtualMachineViewDialog.stringdata0))
        return static_cast<void*>(const_cast< VirtualMachineViewDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int VirtualMachineViewDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
