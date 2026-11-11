/****************************************************************************
** Meta object code from reading C++ file 'rtspworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/rtspworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rtspworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_RtspWorker_t {
    QByteArrayData data[35];
    char stringdata0[325];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RtspWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RtspWorker_t qt_meta_stringdata_RtspWorker = {
    {
QT_MOC_LITERAL(0, 0, 10), // "RtspWorker"
QT_MOC_LITERAL(1, 11, 10), // "frameReady"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 5), // "image"
QT_MOC_LITERAL(4, 29, 12), // "stateChanged"
QT_MOC_LITERAL(5, 42, 5), // "state"
QT_MOC_LITERAL(6, 48, 7), // "message"
QT_MOC_LITERAL(7, 56, 12), // "statsUpdated"
QT_MOC_LITERAL(8, 69, 8), // "shownFps"
QT_MOC_LITERAL(9, 78, 5), // "rxFps"
QT_MOC_LITERAL(10, 84, 10), // "lostPerSec"
QT_MOC_LITERAL(11, 95, 13), // "droppedPerSec"
QT_MOC_LITERAL(12, 109, 13), // "skippedPerSec"
QT_MOC_LITERAL(13, 123, 5), // "width"
QT_MOC_LITERAL(14, 129, 6), // "height"
QT_MOC_LITERAL(15, 136, 8), // "decodeMs"
QT_MOC_LITERAL(16, 145, 6), // "rcvBuf"
QT_MOC_LITERAL(17, 152, 5), // "start"
QT_MOC_LITERAL(18, 158, 3), // "url"
QT_MOC_LITERAL(19, 162, 6), // "useTcp"
QT_MOC_LITERAL(20, 169, 4), // "stop"
QT_MOC_LITERAL(21, 174, 11), // "onConnected"
QT_MOC_LITERAL(22, 186, 14), // "onTcpReadyRead"
QT_MOC_LITERAL(23, 201, 10), // "onTcpError"
QT_MOC_LITERAL(24, 212, 14), // "onRtpReadyRead"
QT_MOC_LITERAL(25, 227, 15), // "onRtcpReadyRead"
QT_MOC_LITERAL(26, 243, 11), // "onKeepAlive"
QT_MOC_LITERAL(27, 255, 10), // "onWatchdog"
QT_MOC_LITERAL(28, 266, 11), // "onStatsTick"
QT_MOC_LITERAL(29, 278, 9), // "reconnect"
QT_MOC_LITERAL(30, 288, 5), // "State"
QT_MOC_LITERAL(31, 294, 4), // "Idle"
QT_MOC_LITERAL(32, 299, 10), // "Connecting"
QT_MOC_LITERAL(33, 310, 7), // "Playing"
QT_MOC_LITERAL(34, 318, 6) // "Failed"

    },
    "RtspWorker\0frameReady\0\0image\0stateChanged\0"
    "state\0message\0statsUpdated\0shownFps\0"
    "rxFps\0lostPerSec\0droppedPerSec\0"
    "skippedPerSec\0width\0height\0decodeMs\0"
    "rcvBuf\0start\0url\0useTcp\0stop\0onConnected\0"
    "onTcpReadyRead\0onTcpError\0onRtpReadyRead\0"
    "onRtcpReadyRead\0onKeepAlive\0onWatchdog\0"
    "onStatsTick\0reconnect\0State\0Idle\0"
    "Connecting\0Playing\0Failed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RtspWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       1,  126, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,
       4,    2,   87,    2, 0x06 /* Public */,
       7,    9,   92,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      17,    2,  111,    2, 0x0a /* Public */,
      20,    0,  116,    2, 0x0a /* Public */,
      21,    0,  117,    2, 0x08 /* Private */,
      22,    0,  118,    2, 0x08 /* Private */,
      23,    0,  119,    2, 0x08 /* Private */,
      24,    0,  120,    2, 0x08 /* Private */,
      25,    0,  121,    2, 0x08 /* Private */,
      26,    0,  122,    2, 0x08 /* Private */,
      27,    0,  123,    2, 0x08 /* Private */,
      28,    0,  124,    2, 0x08 /* Private */,
      29,    0,  125,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    5,    6,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Int, QMetaType::Int, QMetaType::Double, QMetaType::Int,    8,    9,   10,   11,   12,   13,   14,   15,   16,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   18,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // enums: name, alias, flags, count, data
      30,   30, 0x0,    4,  131,

 // enum data: key, value
      31, uint(RtspWorker::Idle),
      32, uint(RtspWorker::Connecting),
      33, uint(RtspWorker::Playing),
      34, uint(RtspWorker::Failed),

       0        // eod
};

void RtspWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RtspWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameReady((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 1: _t->stateChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->statsUpdated((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< int(*)>(_a[7])),(*reinterpret_cast< double(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        case 3: _t->start((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->stop(); break;
        case 5: _t->onConnected(); break;
        case 6: _t->onTcpReadyRead(); break;
        case 7: _t->onTcpError(); break;
        case 8: _t->onRtpReadyRead(); break;
        case 9: _t->onRtcpReadyRead(); break;
        case 10: _t->onKeepAlive(); break;
        case 11: _t->onWatchdog(); break;
        case 12: _t->onStatsTick(); break;
        case 13: _t->reconnect(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RtspWorker::*)(const QImage & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RtspWorker::frameReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (RtspWorker::*)(int , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RtspWorker::stateChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (RtspWorker::*)(double , double , double , double , double , int , int , double , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RtspWorker::statsUpdated)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject RtspWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_RtspWorker.data,
    qt_meta_data_RtspWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *RtspWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RtspWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RtspWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RtspWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
    return _id;
}

// SIGNAL 0
void RtspWorker::frameReady(const QImage & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RtspWorker::stateChanged(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RtspWorker::statsUpdated(double _t1, double _t2, double _t3, double _t4, double _t5, int _t6, int _t7, double _t8, int _t9)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t8))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t9))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
