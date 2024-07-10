/****************************************************************************
** Meta object code from reading C++ file 'x_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "x_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'x_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_x_window_t {
    QByteArrayData data[14];
    char stringdata0[268];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_x_window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_x_window_t qt_meta_stringdata_x_window = {
    {
QT_MOC_LITERAL(0, 0, 8), // "x_window"
QT_MOC_LITERAL(1, 9, 25), // "on_x_close_button_clicked"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 32), // "on_horizontalSlider_valueChanged"
QT_MOC_LITERAL(4, 69, 5), // "value"
QT_MOC_LITERAL(5, 75, 23), // "on_lineEdit_textChanged"
QT_MOC_LITERAL(6, 99, 4), // "arg1"
QT_MOC_LITERAL(7, 104, 21), // "on_get_button_clicked"
QT_MOC_LITERAL(8, 126, 22), // "on_lose_button_clicked"
QT_MOC_LITERAL(9, 149, 26), // "on_step_set_button_clicked"
QT_MOC_LITERAL(10, 176, 21), // "on_pushButton_pressed"
QT_MOC_LITERAL(11, 198, 22), // "on_pushButton_released"
QT_MOC_LITERAL(12, 221, 22), // "on_left_button_clicked"
QT_MOC_LITERAL(13, 244, 23) // "on_right_button_clicked"

    },
    "x_window\0on_x_close_button_clicked\0\0"
    "on_horizontalSlider_valueChanged\0value\0"
    "on_lineEdit_textChanged\0arg1\0"
    "on_get_button_clicked\0on_lose_button_clicked\0"
    "on_step_set_button_clicked\0"
    "on_pushButton_pressed\0on_pushButton_released\0"
    "on_left_button_clicked\0on_right_button_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_x_window[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x08 /* Private */,
       3,    1,   65,    2, 0x08 /* Private */,
       5,    1,   68,    2, 0x08 /* Private */,
       7,    0,   71,    2, 0x08 /* Private */,
       8,    0,   72,    2, 0x08 /* Private */,
       9,    0,   73,    2, 0x08 /* Private */,
      10,    0,   74,    2, 0x08 /* Private */,
      11,    0,   75,    2, 0x08 /* Private */,
      12,    0,   76,    2, 0x08 /* Private */,
      13,    0,   77,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void x_window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<x_window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_x_close_button_clicked(); break;
        case 1: _t->on_horizontalSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_lineEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_get_button_clicked(); break;
        case 4: _t->on_lose_button_clicked(); break;
        case 5: _t->on_step_set_button_clicked(); break;
        case 6: _t->on_pushButton_pressed(); break;
        case 7: _t->on_pushButton_released(); break;
        case 8: _t->on_left_button_clicked(); break;
        case 9: _t->on_right_button_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject x_window::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_x_window.data,
    qt_meta_data_x_window,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *x_window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *x_window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_x_window.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int x_window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
