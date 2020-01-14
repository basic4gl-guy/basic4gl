/****************************************************************************
** Meta object code from reading C++ file 'plugindialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../plugindialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plugindialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PluginDialog_t {
    QByteArrayData data[8];
    char stringdata0[112];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PluginDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PluginDialog_t qt_meta_stringdata_PluginDialog = {
    {
QT_MOC_LITERAL(0, 0, 12), // "PluginDialog"
QT_MOC_LITERAL(1, 13, 25), // "on_pluginList_itemChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(4, 57, 4), // "item"
QT_MOC_LITERAL(5, 62, 32), // "on_pluginList_currentItemChanged"
QT_MOC_LITERAL(6, 95, 7), // "current"
QT_MOC_LITERAL(7, 103, 8) // "previous"

    },
    "PluginDialog\0on_pluginList_itemChanged\0"
    "\0QListWidgetItem*\0item\0"
    "on_pluginList_currentItemChanged\0"
    "current\0previous"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PluginDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x08 /* Private */,
       5,    2,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    6,    7,

       0        // eod
};

void PluginDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PluginDialog *_t = static_cast<PluginDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_pluginList_itemChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 1: _t->on_pluginList_currentItemChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QListWidgetItem*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject PluginDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PluginDialog.data,
      qt_meta_data_PluginDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PluginDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PluginDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PluginDialog.stringdata0))
        return static_cast<void*>(const_cast< PluginDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PluginDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
