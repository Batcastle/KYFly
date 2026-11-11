/****************************************************************************
** Meta object code from reading C++ file 'videoitem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/videoitem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videoitem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VideoItem_t {
    QByteArrayData data[15];
    char stringdata0[165];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoItem_t qt_meta_stringdata_VideoItem = {
    {
QT_MOC_LITERAL(0, 0, 9), // "VideoItem"
QT_MOC_LITERAL(1, 10, 15), // "receiverChanged"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 17), // "sourceSizeChanged"
QT_MOC_LITERAL(4, 45, 15), // "fillModeChanged"
QT_MOC_LITERAL(5, 61, 15), // "hasFrameChanged"
QT_MOC_LITERAL(6, 77, 8), // "setFrame"
QT_MOC_LITERAL(7, 86, 5), // "image"
QT_MOC_LITERAL(8, 92, 5), // "clear"
QT_MOC_LITERAL(9, 98, 8), // "receiver"
QT_MOC_LITERAL(10, 107, 14), // "VideoReceiver*"
QT_MOC_LITERAL(11, 122, 8), // "fillMode"
QT_MOC_LITERAL(12, 131, 8), // "hasFrame"
QT_MOC_LITERAL(13, 140, 11), // "sourceWidth"
QT_MOC_LITERAL(14, 152, 12) // "sourceHeight"

    },
    "VideoItem\0receiverChanged\0\0sourceSizeChanged\0"
    "fillModeChanged\0hasFrameChanged\0"
    "setFrame\0image\0clear\0receiver\0"
    "VideoReceiver*\0fillMode\0hasFrame\0"
    "sourceWidth\0sourceHeight"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoItem[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       5,   52, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,
       4,    0,   46,    2, 0x06 /* Public */,
       5,    0,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   48,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
       8,    0,   51,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QImage,    7,

 // methods: parameters
    QMetaType::Void,

 // properties: name, type, flags
       9, 0x80000000 | 10, 0x0049510b,
      11, QMetaType::Bool, 0x00495103,
      12, QMetaType::Bool, 0x00495001,
      13, QMetaType::Int, 0x00495001,
      14, QMetaType::Int, 0x00495001,

 // properties: notify_signal_id
       0,
       2,
       3,
       1,
       1,

       0        // eod
};

void VideoItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoItem *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->receiverChanged(); break;
        case 1: _t->sourceSizeChanged(); break;
        case 2: _t->fillModeChanged(); break;
        case 3: _t->hasFrameChanged(); break;
        case 4: _t->setFrame((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 5: _t->clear(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoItem::receiverChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoItem::sourceSizeChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VideoItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoItem::fillModeChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (VideoItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoItem::hasFrameChanged)) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< VideoReceiver* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<VideoItem *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< VideoReceiver**>(_v) = _t->receiver(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->fillMode(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->hasFrame(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->sourceWidth(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->sourceHeight(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<VideoItem *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setReceiver(*reinterpret_cast< VideoReceiver**>(_v)); break;
        case 1: _t->setFillMode(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject VideoItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_meta_stringdata_VideoItem.data,
    qt_meta_data_VideoItem,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoItem.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int VideoItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void VideoItem::receiverChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void VideoItem::sourceSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void VideoItem::fillModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void VideoItem::hasFrameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
