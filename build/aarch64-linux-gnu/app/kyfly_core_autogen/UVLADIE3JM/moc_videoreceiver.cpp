/****************************************************************************
** Meta object code from reading C++ file 'videoreceiver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/videoreceiver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videoreceiver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VideoReceiver_t {
    QByteArrayData data[42];
    char stringdata0[366];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoReceiver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoReceiver_t qt_meta_stringdata_VideoReceiver = {
    {
QT_MOC_LITERAL(0, 0, 13), // "VideoReceiver"
QT_MOC_LITERAL(1, 14, 10), // "frameReady"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 5), // "image"
QT_MOC_LITERAL(4, 32, 10), // "urlChanged"
QT_MOC_LITERAL(5, 43, 13), // "useTcpChanged"
QT_MOC_LITERAL(6, 57, 12), // "stateChanged"
QT_MOC_LITERAL(7, 70, 14), // "runningChanged"
QT_MOC_LITERAL(8, 85, 12), // "statsChanged"
QT_MOC_LITERAL(9, 98, 11), // "startWorker"
QT_MOC_LITERAL(10, 110, 3), // "url"
QT_MOC_LITERAL(11, 114, 6), // "useTcp"
QT_MOC_LITERAL(12, 121, 10), // "stopWorker"
QT_MOC_LITERAL(13, 132, 13), // "onWorkerState"
QT_MOC_LITERAL(14, 146, 5), // "state"
QT_MOC_LITERAL(15, 152, 7), // "message"
QT_MOC_LITERAL(16, 160, 13), // "onWorkerStats"
QT_MOC_LITERAL(17, 174, 3), // "fps"
QT_MOC_LITERAL(18, 178, 5), // "rxFps"
QT_MOC_LITERAL(19, 184, 4), // "lost"
QT_MOC_LITERAL(20, 189, 7), // "dropped"
QT_MOC_LITERAL(21, 197, 7), // "skipped"
QT_MOC_LITERAL(22, 205, 1), // "w"
QT_MOC_LITERAL(23, 207, 1), // "h"
QT_MOC_LITERAL(24, 209, 8), // "decodeMs"
QT_MOC_LITERAL(25, 218, 6), // "rcvBuf"
QT_MOC_LITERAL(26, 225, 5), // "start"
QT_MOC_LITERAL(27, 231, 4), // "stop"
QT_MOC_LITERAL(28, 236, 7), // "restart"
QT_MOC_LITERAL(29, 244, 7), // "delayMs"
QT_MOC_LITERAL(30, 252, 6), // "status"
QT_MOC_LITERAL(31, 259, 7), // "running"
QT_MOC_LITERAL(32, 267, 10), // "frameWidth"
QT_MOC_LITERAL(33, 278, 11), // "frameHeight"
QT_MOC_LITERAL(34, 290, 10), // "lostPerSec"
QT_MOC_LITERAL(35, 301, 13), // "droppedPerSec"
QT_MOC_LITERAL(36, 315, 13), // "skippedPerSec"
QT_MOC_LITERAL(37, 329, 5), // "State"
QT_MOC_LITERAL(38, 335, 4), // "Idle"
QT_MOC_LITERAL(39, 340, 10), // "Connecting"
QT_MOC_LITERAL(40, 351, 7), // "Playing"
QT_MOC_LITERAL(41, 359, 6) // "Failed"

    },
    "VideoReceiver\0frameReady\0\0image\0"
    "urlChanged\0useTcpChanged\0stateChanged\0"
    "runningChanged\0statsChanged\0startWorker\0"
    "url\0useTcp\0stopWorker\0onWorkerState\0"
    "state\0message\0onWorkerStats\0fps\0rxFps\0"
    "lost\0dropped\0skipped\0w\0h\0decodeMs\0"
    "rcvBuf\0start\0stop\0restart\0delayMs\0"
    "status\0running\0frameWidth\0frameHeight\0"
    "lostPerSec\0droppedPerSec\0skippedPerSec\0"
    "State\0Idle\0Connecting\0Playing\0Failed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoReceiver[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
      14,  128, // properties
       1,  184, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,
       4,    0,   87,    2, 0x06 /* Public */,
       5,    0,   88,    2, 0x06 /* Public */,
       6,    0,   89,    2, 0x06 /* Public */,
       7,    0,   90,    2, 0x06 /* Public */,
       8,    0,   91,    2, 0x06 /* Public */,
       9,    2,   92,    2, 0x06 /* Public */,
      12,    0,   97,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    2,   98,    2, 0x08 /* Private */,
      16,    9,  103,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      26,    0,  122,    2, 0x02 /* Public */,
      27,    0,  123,    2, 0x02 /* Public */,
      28,    1,  124,    2, 0x02 /* Public */,
      28,    0,  127,    2, 0x22 /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   10,   11,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   14,   15,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Int, QMetaType::Int, QMetaType::Double, QMetaType::Int,   17,   18,   19,   20,   21,   22,   23,   24,   25,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,

 // properties: name, type, flags
      10, QMetaType::QString, 0x00495103,
      11, QMetaType::Bool, 0x00495103,
      14, QMetaType::Int, 0x00495001,
      30, QMetaType::QString, 0x00495001,
      31, QMetaType::Bool, 0x00495001,
      17, QMetaType::Double, 0x00495001,
      24, QMetaType::Double, 0x00495001,
      32, QMetaType::Int, 0x00495001,
      33, QMetaType::Int, 0x00495001,
      18, QMetaType::Double, 0x00495001,
      34, QMetaType::Double, 0x00495001,
      35, QMetaType::Double, 0x00495001,
      36, QMetaType::Double, 0x00495001,
      25, QMetaType::Int, 0x00495001,

 // properties: notify_signal_id
       1,
       2,
       3,
       3,
       4,
       5,
       5,
       5,
       5,
       5,
       5,
       5,
       5,
       5,

 // enums: name, alias, flags, count, data
      37,   37, 0x0,    4,  189,

 // enum data: key, value
      38, uint(VideoReceiver::Idle),
      39, uint(VideoReceiver::Connecting),
      40, uint(VideoReceiver::Playing),
      41, uint(VideoReceiver::Failed),

       0        // eod
};

void VideoReceiver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoReceiver *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameReady((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 1: _t->urlChanged(); break;
        case 2: _t->useTcpChanged(); break;
        case 3: _t->stateChanged(); break;
        case 4: _t->runningChanged(); break;
        case 5: _t->statsChanged(); break;
        case 6: _t->startWorker((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: _t->stopWorker(); break;
        case 8: _t->onWorkerState((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 9: _t->onWorkerStats((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< int(*)>(_a[7])),(*reinterpret_cast< double(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 10: _t->start(); break;
        case 11: _t->stop(); break;
        case 12: _t->restart((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->restart(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoReceiver::*)(const QImage & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::frameReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::urlChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::useTcpChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::stateChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::runningChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::statsChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::startWorker)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (VideoReceiver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoReceiver::stopWorker)) {
                *result = 7;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<VideoReceiver *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->url(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->useTcp(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->state(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->status(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->running(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->fps(); break;
        case 6: *reinterpret_cast< double*>(_v) = _t->decodeMs(); break;
        case 7: *reinterpret_cast< int*>(_v) = _t->frameWidth(); break;
        case 8: *reinterpret_cast< int*>(_v) = _t->frameHeight(); break;
        case 9: *reinterpret_cast< double*>(_v) = _t->rxFps(); break;
        case 10: *reinterpret_cast< double*>(_v) = _t->lostPerSec(); break;
        case 11: *reinterpret_cast< double*>(_v) = _t->droppedPerSec(); break;
        case 12: *reinterpret_cast< double*>(_v) = _t->skippedPerSec(); break;
        case 13: *reinterpret_cast< int*>(_v) = _t->rcvBuf(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<VideoReceiver *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setUrl(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setUseTcp(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject VideoReceiver::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_VideoReceiver.data,
    qt_meta_data_VideoReceiver,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoReceiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoReceiver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoReceiver.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int VideoReceiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 14;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void VideoReceiver::frameReady(const QImage & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VideoReceiver::urlChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void VideoReceiver::useTcpChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void VideoReceiver::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void VideoReceiver::runningChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void VideoReceiver::statsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void VideoReceiver::startWorker(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void VideoReceiver::stopWorker()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
