/****************************************************************************
** Meta object code from reading C++ file 'translater.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Documents/GitHub/Speed-Box/src/translater.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'translater.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Translater_t {
    const uint offsetsAndSize[20];
    char stringdata0[89];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Translater_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Translater_t qt_meta_stringdata_Translater = {
    {
QT_MOC_LITERAL(0, 10), // "Translater"
QT_MOC_LITERAL(11, 6), // "msgBox"
QT_MOC_LITERAL(18, 0), // ""
QT_MOC_LITERAL(19, 11), // "const char*"
QT_MOC_LITERAL(31, 8), // "finished"
QT_MOC_LITERAL(40, 6), // "setFix"
QT_MOC_LITERAL(47, 7), // "checked"
QT_MOC_LITERAL(55, 12), // "copyTranlate"
QT_MOC_LITERAL(68, 11), // "startEnToZh"
QT_MOC_LITERAL(80, 8) // "getReply"

    },
    "Translater\0msgBox\0\0const char*\0finished\0"
    "setFix\0checked\0copyTranlate\0startEnToZh\0"
    "getReply"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Translater[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,
       4,    1,   53,    2, 0x06,    3 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    1,   56,    2, 0x08,    5 /* Private */,
       7,    0,   59,    2, 0x08,    7 /* Private */,
       8,    1,   60,    2, 0x08,    8 /* Private */,
       9,    1,   63,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::QByteArray,    2,

       0        // eod
};

void Translater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Translater *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->msgBox((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        case 1: _t->finished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->setFix((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->copyTranlate(); break;
        case 4: _t->startEnToZh((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->getReply((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Translater::*)(const char * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Translater::msgBox)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Translater::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Translater::finished)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject Translater::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Translater.offsetsAndSize,
    qt_meta_data_Translater,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Translater_t
, QtPrivate::TypeAndForceComplete<Translater, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const char *, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>


>,
    nullptr
} };


const QMetaObject *Translater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Translater::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Translater.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Translater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Translater::msgBox(const char * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Translater::finished(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
