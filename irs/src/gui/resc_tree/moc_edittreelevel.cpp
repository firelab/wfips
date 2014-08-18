/****************************************************************************
** Meta object code from reading C++ file 'edittreelevel.h'
**
** Created: Fri Apr 4 13:13:04 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "edittreelevel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'edittreelevel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EditTreeLevel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      31,   14,   14,   14, 0x08,
      49,   43,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_EditTreeLevel[] = {
    "EditTreeLevel\0\0changeChecked()\0"
    "editLayer()\0value\0setDOIAgencies(int)\0"
};

void EditTreeLevel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        EditTreeLevel *_t = static_cast<EditTreeLevel *>(_o);
        switch (_id) {
        case 0: _t->changeChecked(); break;
        case 1: _t->editLayer(); break;
        case 2: _t->setDOIAgencies((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData EditTreeLevel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EditTreeLevel::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_EditTreeLevel,
      qt_meta_data_EditTreeLevel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EditTreeLevel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EditTreeLevel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EditTreeLevel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EditTreeLevel))
        return static_cast<void*>(const_cast< EditTreeLevel*>(this));
    return QDialog::qt_metacast(_clname);
}

int EditTreeLevel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
