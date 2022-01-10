/****************************************************************************
** Meta object code from reading C++ file 'funcbox.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Documents/GitHub/Speed-Box/src/funcbox.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'funcbox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VARBOX_t {
    const uint offsetsAndSize[16];
    char stringdata0[72];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_VARBOX_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_VARBOX_t qt_meta_stringdata_VARBOX = {
    {
QT_MOC_LITERAL(0, 6), // "VARBOX"
QT_MOC_LITERAL(7, 3), // "MSG"
QT_MOC_LITERAL(11, 0), // ""
QT_MOC_LITERAL(12, 11), // "const char*"
QT_MOC_LITERAL(24, 4), // "text"
QT_MOC_LITERAL(29, 5), // "title"
QT_MOC_LITERAL(35, 28), // "QMessageBox::StandardButtons"
QT_MOC_LITERAL(64, 7) // "buttons"

    },
    "VARBOX\0MSG\0\0const char*\0text\0title\0"
    "QMessageBox::StandardButtons\0buttons"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VARBOX[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   32,    2, 0x0a,    1 /* Public */,
       1,    2,   39,    2, 0x2a,    5 /* Public | MethodCloned */,
       1,    1,   44,    2, 0x2a,    8 /* Public | MethodCloned */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, 0x80000000 | 6,    4,    5,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void VARBOX::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VARBOX *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->MSG((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2])),(*reinterpret_cast< QMessageBox::StandardButtons(*)>(_a[3]))); break;
        case 1: _t->MSG((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2]))); break;
        case 2: _t->MSG((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject VARBOX::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_VARBOX.offsetsAndSize,
    qt_meta_data_VARBOX,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_VARBOX_t
, QtPrivate::TypeAndForceComplete<VARBOX, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>, QtPrivate::TypeAndForceComplete<QMessageBox::StandardButtons, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>


>,
    nullptr
} };


const QMetaObject *VARBOX::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VARBOX::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VARBOX.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int VARBOX::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
