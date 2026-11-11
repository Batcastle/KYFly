/****************************************************************************
** Meta object code from reading C++ file 'flightinput.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/flightinput.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'flightinput.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FlightInput_t {
    QByteArrayData data[47];
    char stringdata0[520];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FlightInput_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FlightInput_t qt_meta_stringdata_FlightInput = {
    {
QT_MOC_LITERAL(0, 0, 11), // "FlightInput"
QT_MOC_LITERAL(1, 12, 11), // "axesChanged"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 19), // "activeSourceChanged"
QT_MOC_LITERAL(4, 45, 22), // "throttleSpringsChanged"
QT_MOC_LITERAL(5, 68, 15), // "actionTriggered"
QT_MOC_LITERAL(6, 84, 6), // "action"
QT_MOC_LITERAL(7, 91, 10), // "sourceLost"
QT_MOC_LITERAL(8, 102, 6), // "source"
QT_MOC_LITERAL(9, 109, 7), // "setAxes"
QT_MOC_LITERAL(10, 117, 3), // "yaw"
QT_MOC_LITERAL(11, 121, 8), // "throttle"
QT_MOC_LITERAL(12, 130, 4), // "roll"
QT_MOC_LITERAL(13, 135, 5), // "pitch"
QT_MOC_LITERAL(14, 141, 13), // "releaseSource"
QT_MOC_LITERAL(15, 155, 7), // "trigger"
QT_MOC_LITERAL(16, 163, 13), // "resetThrottle"
QT_MOC_LITERAL(17, 177, 12), // "activeSource"
QT_MOC_LITERAL(18, 190, 15), // "throttleSprings"
QT_MOC_LITERAL(19, 206, 6), // "Source"
QT_MOC_LITERAL(20, 213, 8), // "NoSource"
QT_MOC_LITERAL(21, 222, 5), // "Touch"
QT_MOC_LITERAL(22, 228, 8), // "Keyboard"
QT_MOC_LITERAL(23, 237, 7), // "Gamepad"
QT_MOC_LITERAL(24, 245, 8), // "Scripted"
QT_MOC_LITERAL(25, 254, 6), // "Action"
QT_MOC_LITERAL(26, 261, 7), // "TakeOff"
QT_MOC_LITERAL(27, 269, 4), // "Land"
QT_MOC_LITERAL(28, 274, 13), // "TakeOffOrLand"
QT_MOC_LITERAL(29, 288, 13), // "EmergencyStop"
QT_MOC_LITERAL(30, 302, 13), // "CalibrateGyro"
QT_MOC_LITERAL(31, 316, 14), // "ToggleHeadless"
QT_MOC_LITERAL(32, 331, 18), // "ToggleAltitudeHold"
QT_MOC_LITERAL(33, 350, 11), // "ToggleLight"
QT_MOC_LITERAL(34, 362, 7), // "ArmFlip"
QT_MOC_LITERAL(35, 370, 12), // "SwitchCamera"
QT_MOC_LITERAL(36, 383, 7), // "SpeedUp"
QT_MOC_LITERAL(37, 391, 9), // "SpeedDown"
QT_MOC_LITERAL(38, 401, 11), // "ToggleArmed"
QT_MOC_LITERAL(39, 413, 11), // "RotateVideo"
QT_MOC_LITERAL(40, 425, 12), // "TrimRollLeft"
QT_MOC_LITERAL(41, 438, 13), // "TrimRollRight"
QT_MOC_LITERAL(42, 452, 13), // "TrimPitchBack"
QT_MOC_LITERAL(43, 466, 16), // "TrimPitchForward"
QT_MOC_LITERAL(44, 483, 11), // "TrimYawLeft"
QT_MOC_LITERAL(45, 495, 12), // "TrimYawRight"
QT_MOC_LITERAL(46, 508, 11) // "ActionCount"

    },
    "FlightInput\0axesChanged\0\0activeSourceChanged\0"
    "throttleSpringsChanged\0actionTriggered\0"
    "action\0sourceLost\0source\0setAxes\0yaw\0"
    "throttle\0roll\0pitch\0releaseSource\0"
    "trigger\0resetThrottle\0activeSource\0"
    "throttleSprings\0Source\0NoSource\0Touch\0"
    "Keyboard\0Gamepad\0Scripted\0Action\0"
    "TakeOff\0Land\0TakeOffOrLand\0EmergencyStop\0"
    "CalibrateGyro\0ToggleHeadless\0"
    "ToggleAltitudeHold\0ToggleLight\0ArmFlip\0"
    "SwitchCamera\0SpeedUp\0SpeedDown\0"
    "ToggleArmed\0RotateVideo\0TrimRollLeft\0"
    "TrimRollRight\0TrimPitchBack\0"
    "TrimPitchForward\0TrimYawLeft\0TrimYawRight\0"
    "ActionCount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FlightInput[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       6,   86, // properties
       2,  110, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    0,   60,    2, 0x06 /* Public */,
       4,    0,   61,    2, 0x06 /* Public */,
       5,    1,   62,    2, 0x06 /* Public */,
       7,    1,   65,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
       9,    5,   68,    2, 0x02 /* Public */,
      14,    1,   79,    2, 0x02 /* Public */,
      15,    1,   82,    2, 0x02 /* Public */,
      16,    0,   85,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,

 // methods: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QReal, QMetaType::QReal, QMetaType::QReal, QMetaType::QReal,    8,   10,   11,   12,   13,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,

 // properties: name, type, flags
      17, QMetaType::Int, 0x00495001,
      10, QMetaType::QReal, 0x00495001,
      11, QMetaType::QReal, 0x00495001,
      12, QMetaType::QReal, 0x00495001,
      13, QMetaType::QReal, 0x00495001,
      18, QMetaType::Bool, 0x00495103,

 // properties: notify_signal_id
       1,
       0,
       0,
       0,
       0,
       2,

 // enums: name, alias, flags, count, data
      19,   19, 0x0,    5,  120,
      25,   25, 0x0,   21,  130,

 // enum data: key, value
      20, uint(FlightInput::NoSource),
      21, uint(FlightInput::Touch),
      22, uint(FlightInput::Keyboard),
      23, uint(FlightInput::Gamepad),
      24, uint(FlightInput::Scripted),
      26, uint(FlightInput::TakeOff),
      27, uint(FlightInput::Land),
      28, uint(FlightInput::TakeOffOrLand),
      29, uint(FlightInput::EmergencyStop),
      30, uint(FlightInput::CalibrateGyro),
      31, uint(FlightInput::ToggleHeadless),
      32, uint(FlightInput::ToggleAltitudeHold),
      33, uint(FlightInput::ToggleLight),
      34, uint(FlightInput::ArmFlip),
      35, uint(FlightInput::SwitchCamera),
      36, uint(FlightInput::SpeedUp),
      37, uint(FlightInput::SpeedDown),
      38, uint(FlightInput::ToggleArmed),
      39, uint(FlightInput::RotateVideo),
      40, uint(FlightInput::TrimRollLeft),
      41, uint(FlightInput::TrimRollRight),
      42, uint(FlightInput::TrimPitchBack),
      43, uint(FlightInput::TrimPitchForward),
      44, uint(FlightInput::TrimYawLeft),
      45, uint(FlightInput::TrimYawRight),
      46, uint(FlightInput::ActionCount),

       0        // eod
};

void FlightInput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FlightInput *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->axesChanged(); break;
        case 1: _t->activeSourceChanged(); break;
        case 2: _t->throttleSpringsChanged(); break;
        case 3: _t->actionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->sourceLost((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->setAxes((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2])),(*reinterpret_cast< qreal(*)>(_a[3])),(*reinterpret_cast< qreal(*)>(_a[4])),(*reinterpret_cast< qreal(*)>(_a[5]))); break;
        case 6: _t->releaseSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->trigger((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->resetThrottle(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FlightInput::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FlightInput::axesChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FlightInput::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FlightInput::activeSourceChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FlightInput::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FlightInput::throttleSpringsChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (FlightInput::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FlightInput::actionTriggered)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (FlightInput::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FlightInput::sourceLost)) {
                *result = 4;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FlightInput *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->activeSource(); break;
        case 1: *reinterpret_cast< qreal*>(_v) = _t->yaw(); break;
        case 2: *reinterpret_cast< qreal*>(_v) = _t->throttle(); break;
        case 3: *reinterpret_cast< qreal*>(_v) = _t->roll(); break;
        case 4: *reinterpret_cast< qreal*>(_v) = _t->pitch(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->throttleSprings(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<FlightInput *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 5: _t->setThrottleSprings(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject FlightInput::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_FlightInput.data,
    qt_meta_data_FlightInput,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FlightInput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FlightInput::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FlightInput.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FlightInput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void FlightInput::axesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void FlightInput::activeSourceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void FlightInput::throttleSpringsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void FlightInput::actionTriggered(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void FlightInput::sourceLost(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
