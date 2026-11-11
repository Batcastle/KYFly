/****************************************************************************
** Meta object code from reading C++ file 'dronelink.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/dronelink.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dronelink.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DroneLink_t {
    QByteArrayData data[59];
    char stringdata0[702];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DroneLink_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DroneLink_t qt_meta_stringdata_DroneLink = {
    {
QT_MOC_LITERAL(0, 0, 9), // "DroneLink"
QT_MOC_LITERAL(1, 10, 11), // "hostChanged"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 13), // "activeChanged"
QT_MOC_LITERAL(4, 37, 16), // "connectedChanged"
QT_MOC_LITERAL(5, 54, 14), // "modelIdChanged"
QT_MOC_LITERAL(6, 69, 13), // "formatChanged"
QT_MOC_LITERAL(7, 83, 12), // "armedChanged"
QT_MOC_LITERAL(8, 96, 17), // "speedLevelChanged"
QT_MOC_LITERAL(9, 114, 19), // "altitudeHoldChanged"
QT_MOC_LITERAL(10, 134, 15), // "headlessChanged"
QT_MOC_LITERAL(11, 150, 12), // "lightChanged"
QT_MOC_LITERAL(12, 163, 16), // "flipArmedChanged"
QT_MOC_LITERAL(13, 180, 13), // "flyingChanged"
QT_MOC_LITERAL(14, 194, 11), // "trimChanged"
QT_MOC_LITERAL(15, 206, 17), // "lastPacketChanged"
QT_MOC_LITERAL(16, 224, 16), // "lastReplyChanged"
QT_MOC_LITERAL(17, 241, 12), // "inputChanged"
QT_MOC_LITERAL(18, 254, 21), // "videoRestartRequested"
QT_MOC_LITERAL(19, 276, 7), // "delayMs"
QT_MOC_LITERAL(20, 284, 20), // "rotateVideoRequested"
QT_MOC_LITERAL(21, 305, 13), // "sendHeartbeat"
QT_MOC_LITERAL(22, 319, 11), // "onReadyRead"
QT_MOC_LITERAL(23, 331, 10), // "onSendTick"
QT_MOC_LITERAL(24, 342, 11), // "onLinkCheck"
QT_MOC_LITERAL(25, 354, 8), // "onAction"
QT_MOC_LITERAL(26, 363, 6), // "action"
QT_MOC_LITERAL(27, 370, 7), // "takeOff"
QT_MOC_LITERAL(28, 378, 4), // "land"
QT_MOC_LITERAL(29, 383, 13), // "takeOffOrLand"
QT_MOC_LITERAL(30, 397, 13), // "emergencyStop"
QT_MOC_LITERAL(31, 411, 13), // "calibrateGyro"
QT_MOC_LITERAL(32, 425, 10), // "toggleFlip"
QT_MOC_LITERAL(33, 436, 12), // "switchCamera"
QT_MOC_LITERAL(34, 449, 7), // "sendRaw"
QT_MOC_LITERAL(35, 457, 5), // "bytes"
QT_MOC_LITERAL(36, 463, 4), // "host"
QT_MOC_LITERAL(37, 468, 6), // "active"
QT_MOC_LITERAL(38, 475, 9), // "connected"
QT_MOC_LITERAL(39, 485, 7), // "modelId"
QT_MOC_LITERAL(40, 493, 14), // "formatOverride"
QT_MOC_LITERAL(41, 508, 12), // "packetFormat"
QT_MOC_LITERAL(42, 521, 5), // "armed"
QT_MOC_LITERAL(43, 527, 10), // "speedLevel"
QT_MOC_LITERAL(44, 538, 12), // "speedPercent"
QT_MOC_LITERAL(45, 551, 12), // "altitudeHold"
QT_MOC_LITERAL(46, 564, 18), // "altitudeHoldLocked"
QT_MOC_LITERAL(47, 583, 8), // "headless"
QT_MOC_LITERAL(48, 592, 5), // "light"
QT_MOC_LITERAL(49, 598, 9), // "flipArmed"
QT_MOC_LITERAL(50, 608, 13), // "flipSupported"
QT_MOC_LITERAL(51, 622, 6), // "flying"
QT_MOC_LITERAL(52, 629, 8), // "trimRoll"
QT_MOC_LITERAL(53, 638, 9), // "trimPitch"
QT_MOC_LITERAL(54, 648, 7), // "trimYaw"
QT_MOC_LITERAL(55, 656, 13), // "lastPacketHex"
QT_MOC_LITERAL(56, 670, 12), // "lastReplyHex"
QT_MOC_LITERAL(57, 683, 5), // "input"
QT_MOC_LITERAL(58, 689, 12) // "FlightInput*"

    },
    "DroneLink\0hostChanged\0\0activeChanged\0"
    "connectedChanged\0modelIdChanged\0"
    "formatChanged\0armedChanged\0speedLevelChanged\0"
    "altitudeHoldChanged\0headlessChanged\0"
    "lightChanged\0flipArmedChanged\0"
    "flyingChanged\0trimChanged\0lastPacketChanged\0"
    "lastReplyChanged\0inputChanged\0"
    "videoRestartRequested\0delayMs\0"
    "rotateVideoRequested\0sendHeartbeat\0"
    "onReadyRead\0onSendTick\0onLinkCheck\0"
    "onAction\0action\0takeOff\0land\0takeOffOrLand\0"
    "emergencyStop\0calibrateGyro\0toggleFlip\0"
    "switchCamera\0sendRaw\0bytes\0host\0active\0"
    "connected\0modelId\0formatOverride\0"
    "packetFormat\0armed\0speedLevel\0"
    "speedPercent\0altitudeHold\0altitudeHoldLocked\0"
    "headless\0light\0flipArmed\0flipSupported\0"
    "flying\0trimRoll\0trimPitch\0trimYaw\0"
    "lastPacketHex\0lastReplyHex\0input\0"
    "FlightInput*"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DroneLink[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
      22,  206, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      18,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  169,    2, 0x06 /* Public */,
       3,    0,  170,    2, 0x06 /* Public */,
       4,    0,  171,    2, 0x06 /* Public */,
       5,    0,  172,    2, 0x06 /* Public */,
       6,    0,  173,    2, 0x06 /* Public */,
       7,    0,  174,    2, 0x06 /* Public */,
       8,    0,  175,    2, 0x06 /* Public */,
       9,    0,  176,    2, 0x06 /* Public */,
      10,    0,  177,    2, 0x06 /* Public */,
      11,    0,  178,    2, 0x06 /* Public */,
      12,    0,  179,    2, 0x06 /* Public */,
      13,    0,  180,    2, 0x06 /* Public */,
      14,    0,  181,    2, 0x06 /* Public */,
      15,    0,  182,    2, 0x06 /* Public */,
      16,    0,  183,    2, 0x06 /* Public */,
      17,    0,  184,    2, 0x06 /* Public */,
      18,    1,  185,    2, 0x06 /* Public */,
      20,    0,  188,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      21,    0,  189,    2, 0x08 /* Private */,
      22,    0,  190,    2, 0x08 /* Private */,
      23,    0,  191,    2, 0x08 /* Private */,
      24,    0,  192,    2, 0x08 /* Private */,
      25,    1,  193,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      27,    0,  196,    2, 0x02 /* Public */,
      28,    0,  197,    2, 0x02 /* Public */,
      29,    0,  198,    2, 0x02 /* Public */,
      30,    0,  199,    2, 0x02 /* Public */,
      31,    0,  200,    2, 0x02 /* Public */,
      32,    0,  201,    2, 0x02 /* Public */,
      33,    0,  202,    2, 0x02 /* Public */,
      34,    1,  203,    2, 0x02 /* Public */,

 // signals: parameters
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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   26,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   35,

 // properties: name, type, flags
      36, QMetaType::QString, 0x00495103,
      37, QMetaType::Bool, 0x00495103,
      38, QMetaType::Bool, 0x00495001,
      39, QMetaType::Int, 0x00495001,
      40, QMetaType::Int, 0x00495103,
      41, QMetaType::Int, 0x00495001,
      42, QMetaType::Bool, 0x00495103,
      43, QMetaType::Int, 0x00495103,
      44, QMetaType::Int, 0x00495001,
      45, QMetaType::Bool, 0x00495103,
      46, QMetaType::Bool, 0x00495001,
      47, QMetaType::Bool, 0x00495103,
      48, QMetaType::Bool, 0x00495103,
      49, QMetaType::Bool, 0x00495001,
      50, QMetaType::Bool, 0x00495001,
      51, QMetaType::Bool, 0x00495001,
      52, QMetaType::Int, 0x00495103,
      53, QMetaType::Int, 0x00495103,
      54, QMetaType::Int, 0x00495103,
      55, QMetaType::QString, 0x00495001,
      56, QMetaType::QString, 0x00495001,
      57, 0x80000000 | 58, 0x0049510b,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,
       4,
       4,
       5,
       6,
       6,
       7,
       4,
       8,
       9,
      10,
       4,
      11,
      12,
      12,
      12,
      13,
      14,
      15,

       0        // eod
};

void DroneLink::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DroneLink *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->hostChanged(); break;
        case 1: _t->activeChanged(); break;
        case 2: _t->connectedChanged(); break;
        case 3: _t->modelIdChanged(); break;
        case 4: _t->formatChanged(); break;
        case 5: _t->armedChanged(); break;
        case 6: _t->speedLevelChanged(); break;
        case 7: _t->altitudeHoldChanged(); break;
        case 8: _t->headlessChanged(); break;
        case 9: _t->lightChanged(); break;
        case 10: _t->flipArmedChanged(); break;
        case 11: _t->flyingChanged(); break;
        case 12: _t->trimChanged(); break;
        case 13: _t->lastPacketChanged(); break;
        case 14: _t->lastReplyChanged(); break;
        case 15: _t->inputChanged(); break;
        case 16: _t->videoRestartRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->rotateVideoRequested(); break;
        case 18: _t->sendHeartbeat(); break;
        case 19: _t->onReadyRead(); break;
        case 20: _t->onSendTick(); break;
        case 21: _t->onLinkCheck(); break;
        case 22: _t->onAction((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->takeOff(); break;
        case 24: _t->land(); break;
        case 25: _t->takeOffOrLand(); break;
        case 26: _t->emergencyStop(); break;
        case 27: _t->calibrateGyro(); break;
        case 28: _t->toggleFlip(); break;
        case 29: _t->switchCamera(); break;
        case 30: _t->sendRaw((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::hostChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::activeChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::connectedChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::modelIdChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::formatChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::armedChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::speedLevelChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::altitudeHoldChanged)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::headlessChanged)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::lightChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::flipArmedChanged)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::flyingChanged)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::trimChanged)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::lastPacketChanged)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::lastReplyChanged)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::inputChanged)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::videoRestartRequested)) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (DroneLink::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DroneLink::rotateVideoRequested)) {
                *result = 17;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 21:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< FlightInput* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DroneLink *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->host(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->active(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->connected(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->modelId(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->formatOverride(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->packetFormat(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->armed(); break;
        case 7: *reinterpret_cast< int*>(_v) = _t->speedLevel(); break;
        case 8: *reinterpret_cast< int*>(_v) = _t->speedPercent(); break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->altitudeHold(); break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->altitudeHoldLocked(); break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->headless(); break;
        case 12: *reinterpret_cast< bool*>(_v) = _t->light(); break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->flipArmed(); break;
        case 14: *reinterpret_cast< bool*>(_v) = _t->flipSupported(); break;
        case 15: *reinterpret_cast< bool*>(_v) = _t->flying(); break;
        case 16: *reinterpret_cast< int*>(_v) = _t->trimRoll(); break;
        case 17: *reinterpret_cast< int*>(_v) = _t->trimPitch(); break;
        case 18: *reinterpret_cast< int*>(_v) = _t->trimYaw(); break;
        case 19: *reinterpret_cast< QString*>(_v) = _t->lastPacketHex(); break;
        case 20: *reinterpret_cast< QString*>(_v) = _t->lastReplyHex(); break;
        case 21: *reinterpret_cast< FlightInput**>(_v) = _t->input(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DroneLink *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setHost(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setActive(*reinterpret_cast< bool*>(_v)); break;
        case 4: _t->setFormatOverride(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setArmed(*reinterpret_cast< bool*>(_v)); break;
        case 7: _t->setSpeedLevel(*reinterpret_cast< int*>(_v)); break;
        case 9: _t->setAltitudeHold(*reinterpret_cast< bool*>(_v)); break;
        case 11: _t->setHeadless(*reinterpret_cast< bool*>(_v)); break;
        case 12: _t->setLight(*reinterpret_cast< bool*>(_v)); break;
        case 16: _t->setTrimRoll(*reinterpret_cast< int*>(_v)); break;
        case 17: _t->setTrimPitch(*reinterpret_cast< int*>(_v)); break;
        case 18: _t->setTrimYaw(*reinterpret_cast< int*>(_v)); break;
        case 21: _t->setInput(*reinterpret_cast< FlightInput**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject DroneLink::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DroneLink.data,
    qt_meta_data_DroneLink,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DroneLink::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DroneLink::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DroneLink.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DroneLink::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 31;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 22;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 22;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 22;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 22;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 22;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void DroneLink::hostChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DroneLink::activeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DroneLink::connectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DroneLink::modelIdChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DroneLink::formatChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DroneLink::armedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DroneLink::speedLevelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DroneLink::altitudeHoldChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void DroneLink::headlessChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void DroneLink::lightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void DroneLink::flipArmedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void DroneLink::flyingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void DroneLink::trimChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void DroneLink::lastPacketChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void DroneLink::lastReplyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void DroneLink::inputChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void DroneLink::videoRestartRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void DroneLink::rotateVideoRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
