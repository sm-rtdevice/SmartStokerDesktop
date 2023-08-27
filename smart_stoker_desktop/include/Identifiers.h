#ifndef IDENTIFIERS_H  // 08_11_2012
#define IDENTIFIERS_H

typedef /*__cdecl*/ /*__stdcall*/ /*__fastcall*/ bool (*pCRCFunk)(byte*, byte);  // объявление типа указателя на функцию проверки CRC

enum ERRSTATE {sNormal, sTimeOut, sError, sNotGas,  sUnknown, sWait, sCRC, sWarning};  // состояние устройства, наличие газа, sWait - режим ожидания (данные не готовы)
enum CMDRES{crSuccess, crTimeOut, crError, crCRCError, crInfiniteTimeOut, crBufError}; // результат выполнения команды
enum DevTYPE{dtRRG_12,dtRM_5,dtFK,dtAT32};                                             // тип устройства

#endif
