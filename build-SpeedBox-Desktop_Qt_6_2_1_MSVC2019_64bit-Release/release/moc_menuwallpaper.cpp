/****************************************************************************
** Meta object code from reading C++ file 'menuwallpaper.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Documents/GitHub/Speed-Box/src/menuwallpaper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'menuwallpaper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MenuWallpaper_t {
    const uint offsetsAndSize[8];
    char stringdata0[37];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_MenuWallpaper_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_MenuWallpaper_t qt_meta_stringdata_MenuWallpaper = {
    {
QT_MOC_LITERAL(0, 13), // "MenuWallpaper"
QT_MOC_LITERAL(14, 11), // "previousPic"
QT_MOC_LITERAL(26, 0), // ""
QT_MOC_LITERAL(27, 9) // "removePic"

    },
    "MenuWallpaper\0previousPic\0\0removePic"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MenuWallpaper[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x0a,    1 /* Public */,
       3,    0,   27,    2, 0x0a,    2 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MenuWallpaper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MenuWallpaper *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->previousPic(); break;
        case 1: _t->removePic(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject MenuWallpaper::staticMetaObject = { {
    QMetaObject::SuperData::link<Wallpaper::staticMetaObject>(),
    qt_meta_stringdata_MenuWallpaper.offsetsAndSize,
    qt_meta_data_MenuWallpaper,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_MenuWallpaper_t
, QtPrivate::TypeAndForceComplete<MenuWallpaper, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *MenuWallpaper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MenuWallpaper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MenuWallpaper.stringdata0))
        return static_cast<void*>(this);
    return Wallpaper::qt_metacast(_clname);
}

int MenuWallpaper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Wallpaper::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
