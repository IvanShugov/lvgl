#ifndef LV_SYMBOL_DEF_H
#define LV_SYMBOL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

/*
 * With no UTF-8 support (192- 255) (192..241 is used)
 *
 * With UTF-8 support (in Supplemental Private Use Area-A): 0xF800 .. 0xF831
 * - Basic symbols:     0xE000..0xE01F
 * - File symbols:      0xE020..0xE03F
 * - Feedback symbols:  0xE040..0xE05F
 * - Reserved:          0xE060..0xE07F
 */

#if LV_TXT_UTF8 == 0
#define LV_SYMBOL_GLYPH_FIRST  0xC0
#define LV_SYMBOL_AUDIO           _LV_SYMBOL_VALUE1(C0)
#define LV_SYMBOL_VIDEO           _LV_SYMBOL_VALUE1(C1)
#define LV_SYMBOL_LIST            _LV_SYMBOL_VALUE1(C2)
#define LV_SYMBOL_OK              _LV_SYMBOL_VALUE1(C3)
#define LV_SYMBOL_CLOSE           _LV_SYMBOL_VALUE1(C4)
#define LV_SYMBOL_POWER           _LV_SYMBOL_VALUE1(C5)
#define LV_SYMBOL_SETTINGS        _LV_SYMBOL_VALUE1(C6)
#define LV_SYMBOL_TRASH           _LV_SYMBOL_VALUE1(C7)
#define LV_SYMBOL_HOME            _LV_SYMBOL_VALUE1(C8)
#define LV_SYMBOL_DOWNLOAD        _LV_SYMBOL_VALUE1(C9)
#define LV_SYMBOL_DRIVE           _LV_SYMBOL_VALUE1(CA)
#define LV_SYMBOL_REFRESH         _LV_SYMBOL_VALUE1(CB)
#define LV_SYMBOL_MUTE            _LV_SYMBOL_VALUE1(CC)
#define LV_SYMBOL_VOLUME_MID      _LV_SYMBOL_VALUE1(CD)
#define LV_SYMBOL_VOLUME_MAX      _LV_SYMBOL_VALUE1(CE)
#define LV_SYMBOL_IMAGE           _LV_SYMBOL_VALUE1(CF)
#define LV_SYMBOL_EDIT            _LV_SYMBOL_VALUE1(D0)
#define LV_SYMBOL_PREV            _LV_SYMBOL_VALUE1(D1)
#define LV_SYMBOL_PLAY            _LV_SYMBOL_VALUE1(D2)
#define LV_SYMBOL_PAUSE           _LV_SYMBOL_VALUE1(D3)
#define LV_SYMBOL_STOP            _LV_SYMBOL_VALUE1(D4)
#define LV_SYMBOL_NEXT            _LV_SYMBOL_VALUE1(D5)
#define LV_SYMBOL_EJECT           _LV_SYMBOL_VALUE1(D6)
#define LV_SYMBOL_LEFT            _LV_SYMBOL_VALUE1(D7)
#define LV_SYMBOL_RIGHT           _LV_SYMBOL_VALUE1(D8)
#define LV_SYMBOL_PLUS            _LV_SYMBOL_VALUE1(D9)
#define LV_SYMBOL_MINUS           _LV_SYMBOL_VALUE1(DA)
#define LV_SYMBOL_WARNING         _LV_SYMBOL_VALUE1(DB)
#define LV_SYMBOL_SHUFFLE         _LV_SYMBOL_VALUE1(DC)
#define LV_SYMBOL_UP              _LV_SYMBOL_VALUE1(DD)
#define LV_SYMBOL_DOWN            _LV_SYMBOL_VALUE1(DE)
#define LV_SYMBOL_LOOP            _LV_SYMBOL_VALUE1(DF)
#define LV_SYMBOL_DIRECTORY       _LV_SYMBOL_VALUE1(E0)
#define LV_SYMBOL_UPLOAD          _LV_SYMBOL_VALUE1(E1)
#define LV_SYMBOL_CALL            _LV_SYMBOL_VALUE1(E2)
#define LV_SYMBOL_CUT             _LV_SYMBOL_VALUE1(E3)
#define LV_SYMBOL_COPY            _LV_SYMBOL_VALUE1(E4)
#define LV_SYMBOL_SAVE            _LV_SYMBOL_VALUE1(E5)
#define LV_SYMBOL_CHARGE          _LV_SYMBOL_VALUE1(E6)
#define LV_SYMBOL_BELL            _LV_SYMBOL_VALUE1(E7)
#define LV_SYMBOL_KEYBOARD        _LV_SYMBOL_VALUE1(E8)
#define LV_SYMBOL_GPS             _LV_SYMBOL_VALUE1(E9)
#define LV_SYMBOL_FILE            _LV_SYMBOL_VALUE1(EA)
#define LV_SYMBOL_WIFI            _LV_SYMBOL_VALUE1(EB)
#define LV_SYMBOL_BATTERY_FULL    _LV_SYMBOL_VALUE1(EC)
#define LV_SYMBOL_BATTERY_3       _LV_SYMBOL_VALUE1(ED)
#define LV_SYMBOL_BATTERY_2       _LV_SYMBOL_VALUE1(EE)
#define LV_SYMBOL_BATTERY_1       _LV_SYMBOL_VALUE1(EF)
#define LV_SYMBOL_BATTERY_EMPTY   _LV_SYMBOL_VALUE1(F0)
#define LV_SYMBOL_BLUETOOTH       _LV_SYMBOL_VALUE1(F1)
#define LV_SYMBOL_GLYPH_LAST   0xF1
#define LV_SYMBOL_DUMMY           _LV_SYMBOL_VALUE1(FF)       /*Invalid symbol. If written before a string then `lv_img` will show it as a label*/

#else
#define LV_SYMBOL_GLYPH_FIRST  0xF800
#define LV_SYMBOL_AUDIO           _LV_SYMBOL_VALUE3(EF,A0,80)
#define LV_SYMBOL_VIDEO           _LV_SYMBOL_VALUE3(EF,A0,81)
#define LV_SYMBOL_LIST            _LV_SYMBOL_VALUE3(EF,A0,82)
#define LV_SYMBOL_OK              _LV_SYMBOL_VALUE3(EF,A0,83)
#define LV_SYMBOL_CLOSE           _LV_SYMBOL_VALUE3(EF,A0,84)
#define LV_SYMBOL_POWER           _LV_SYMBOL_VALUE3(EF,A0,85)
#define LV_SYMBOL_SETTINGS        _LV_SYMBOL_VALUE3(EF,A0,86)
#define LV_SYMBOL_TRASH           _LV_SYMBOL_VALUE3(EF,A0,87)
#define LV_SYMBOL_HOME            _LV_SYMBOL_VALUE3(EF,A0,88)
#define LV_SYMBOL_DOWNLOAD        _LV_SYMBOL_VALUE3(EF,A0,89)
#define LV_SYMBOL_DRIVE           _LV_SYMBOL_VALUE3(EF,A0,8A)
#define LV_SYMBOL_REFRESH         _LV_SYMBOL_VALUE3(EF,A0,8B)
#define LV_SYMBOL_MUTE            _LV_SYMBOL_VALUE3(EF,A0,8C)
#define LV_SYMBOL_VOLUME_MID      _LV_SYMBOL_VALUE3(EF,A0,8D)
#define LV_SYMBOL_VOLUME_MAX      _LV_SYMBOL_VALUE3(EF,A0,8E)
#define LV_SYMBOL_IMAGE           _LV_SYMBOL_VALUE3(EF,A0,8F)
#define LV_SYMBOL_EDIT            _LV_SYMBOL_VALUE3(EF,A0,90)
#define LV_SYMBOL_PREV            _LV_SYMBOL_VALUE3(EF,A0,91)
#define LV_SYMBOL_PLAY            _LV_SYMBOL_VALUE3(EF,A0,92)
#define LV_SYMBOL_PAUSE           _LV_SYMBOL_VALUE3(EF,A0,93)
#define LV_SYMBOL_STOP            _LV_SYMBOL_VALUE3(EF,A0,94)
#define LV_SYMBOL_NEXT            _LV_SYMBOL_VALUE3(EF,A0,95)
#define LV_SYMBOL_EJECT           _LV_SYMBOL_VALUE3(EF,A0,96)
#define LV_SYMBOL_LEFT            _LV_SYMBOL_VALUE3(EF,A0,97)
#define LV_SYMBOL_RIGHT           _LV_SYMBOL_VALUE3(EF,A0,98)
#define LV_SYMBOL_PLUS            _LV_SYMBOL_VALUE3(EF,A0,99)
#define LV_SYMBOL_MINUS           _LV_SYMBOL_VALUE3(EF,A0,9A)
#define LV_SYMBOL_WARNING         _LV_SYMBOL_VALUE3(EF,A0,9B)
#define LV_SYMBOL_SHUFFLE         _LV_SYMBOL_VALUE3(EF,A0,9C)
#define LV_SYMBOL_UP              _LV_SYMBOL_VALUE3(EF,A0,9D)
#define LV_SYMBOL_DOWN            _LV_SYMBOL_VALUE3(EF,A0,9E)
#define LV_SYMBOL_LOOP            _LV_SYMBOL_VALUE3(EF,A0,9F)
#define LV_SYMBOL_DIRECTORY       _LV_SYMBOL_VALUE3(EF,A0,A0)
#define LV_SYMBOL_UPLOAD          _LV_SYMBOL_VALUE3(EF,A0,A1)
#define LV_SYMBOL_CALL            _LV_SYMBOL_VALUE3(EF,A0,A2)
#define LV_SYMBOL_CUT             _LV_SYMBOL_VALUE3(EF,A0,A3)
#define LV_SYMBOL_COPY            _LV_SYMBOL_VALUE3(EF,A0,A4)
#define LV_SYMBOL_SAVE            _LV_SYMBOL_VALUE3(EF,A0,A5)
#define LV_SYMBOL_CHARGE          _LV_SYMBOL_VALUE3(EF,A0,A6)
#define LV_SYMBOL_BELL            _LV_SYMBOL_VALUE3(EF,A0,A7)
#define LV_SYMBOL_KEYBOARD        _LV_SYMBOL_VALUE3(EF,A0,A8)
#define LV_SYMBOL_GPS             _LV_SYMBOL_VALUE3(EF,A0,A9)
#define LV_SYMBOL_FILE            _LV_SYMBOL_VALUE3(EF,A0,AA)
#define LV_SYMBOL_WIFI            _LV_SYMBOL_VALUE3(EF,A0,AB)
#define LV_SYMBOL_BATTERY_FULL    _LV_SYMBOL_VALUE3(EF,A0,AC)
#define LV_SYMBOL_BATTERY_3       _LV_SYMBOL_VALUE3(EF,A0,AD)
#define LV_SYMBOL_BATTERY_2       _LV_SYMBOL_VALUE3(EF,A0,AE)
#define LV_SYMBOL_BATTERY_1       _LV_SYMBOL_VALUE3(EF,A0,AF)
#define LV_SYMBOL_BATTERY_EMPTY   _LV_SYMBOL_VALUE3(EF,A0,B0)
#define LV_SYMBOL_BLUETOOTH       _LV_SYMBOL_VALUE3(EF,A0,B1)
#define LV_SYMBOL_GLYPH_LAST   0xF831
#define LV_SYMBOL_DUMMY           _LV_SYMBOL_VALUE3(EF,A3,BF)       /*Invalid symbol at (U+F831). If written before a string then `lv_img` will show it as a label*/
#endif

#define _LV_SYMBOL_VALUE1(x) (0x ## x)
#define _LV_SYMBOL_VALUE3(x, y, z) (0x ## z ## y ## x)
#define _LV_SYMBOL_NUMSTR(sym) LV_ ## sym ## _NUMSTR = sym

enum
{
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_AUDIO),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_VIDEO),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_LIST),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_OK),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_CLOSE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_POWER),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_SETTINGS),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_TRASH),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_HOME),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_DOWNLOAD),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_DRIVE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_REFRESH),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_MUTE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_VOLUME_MID),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_VOLUME_MAX),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_IMAGE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_EDIT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_PREV),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_PLAY),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_PAUSE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_STOP),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_NEXT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_EJECT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_LEFT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_RIGHT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_PLUS),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_MINUS),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_WARNING),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_SHUFFLE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_UP),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_DOWN),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_LOOP),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_DIRECTORY),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_UPLOAD),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_CALL),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_CUT),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_COPY),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_SAVE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_CHARGE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BELL),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_KEYBOARD),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_GPS),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_FILE),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_WIFI),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BATTERY_FULL),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BATTERY_3),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BATTERY_2),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BATTERY_1),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BATTERY_EMPTY),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_BLUETOOTH),
    _LV_SYMBOL_NUMSTR(LV_SYMBOL_DUMMY),
};

#undef _LV_SYMBOL_VALUE1
#undef _LV_SYMBOL_VALUE3

#define _LV_SYMBOL_STR_(x) #x
#define _LV_SYMBOL_STR(x) _LV_SYMBOL_STR_(x)
#define _LV_SYMBOL_CHAR(c) \x ## c
#define _LV_SYMBOL_VALUE1(x) _LV_SYMBOL_STR(_LV_SYMBOL_CHAR(x))
#define _LV_SYMBOL_VALUE3(x, y, z) _LV_SYMBOL_STR(_LV_SYMBOL_CHAR(x)_LV_SYMBOL_CHAR(y)_LV_SYMBOL_CHAR(z))

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /*LV_LV_SYMBOL_DEF_H*/
