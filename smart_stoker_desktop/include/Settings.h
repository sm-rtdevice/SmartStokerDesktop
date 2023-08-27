//---------------------------------------------------------------------------
#ifndef SETTINGS_H
#define SETTINGS_H

#include <IniFiles.hpp>

class CSettings                 // настройки программы
{
  public:
     AnsiString ExePath;

// COM-порты:
     int ComPort;                       // номер порта 1,2...
     int ComPortSpeed;                  // скорость обмена

     int MinT;                          // пределы графика
     int MaxT;
     bool AutoRange;

     AnsiString DS1Name;                // подписи датчиков
     AnsiString DS2Name;

     bool SaveLog;                      // сохранение лог файла (событий, ошибок)

  public:
     bool LoadSettings(); // загрузка параметров
     bool SaveSettings(); // сохранение

     CSettings();
     ~CSettings();
};
//---------------------------------------------------------------------------

CSettings::CSettings()
{
  ExePath = ExtractFilePath( ParamStr( 0 ) );
 //  Connected = false;
 // SaveLog = false;         // ведение логов выключено
 // SaveLog = true;         // ведение логов включено
};

CSettings::~CSettings()
{
};

bool CSettings::LoadSettings() // загрузка параметров
{
 TIniFile *ini;
 ini = new Inifiles::TIniFile( ExePath + "Settings.ini" ); // файл с настройками

 bool result = true; // результат загрузки

 try
 {
  ComPort = ini->ReadInteger( "COM", "ComPort", 10 );                           // номер порта
  ComPortSpeed = ini->ReadInteger( "COM", "ComPortSpeed", 19200 );              // скорость обмена
  MinT = ini->ReadInteger( "GRAF", "MinT", -55 );   // пределы графика
  MaxT = ini->ReadInteger( "GRAF", "MaxT", 125 );
  AutoRange = ini->ReadBool( "GRAF", "AutoRange", false);
  DS1Name = ini->ReadString( "DS", "DS1Name", "радиатор" );
  DS2Name = ini->ReadString( "DS", "DS2Name", "котел" );
 }
 catch(...)
 {
  result = false;
 }

 ini->~TIniFile();
 return result;
};

bool CSettings::SaveSettings() // сохранение
{
 TIniFile *ini;
 ini = new Inifiles::TIniFile( ExePath + "Settings.ini" ); // файл с настройками

 bool result = true; // результат сохранения

 try
 {
  ini->WriteInteger( "COM", "ComPort", ComPort );                               // номер порта
  ini->WriteInteger( "COM", "ComPortSpeed", ComPortSpeed );                     // скорость обмена
  ini->WriteInteger( "GRAF", "MinT", MinT );                                    // пределы графика
  ini->WriteInteger( "GRAF", "MaxT", MaxT );
  ini->WriteBool( "GRAF", "AutoRange", AutoRange);
  ini->WriteString( "DS", "DS1Name", DS1Name );
  ini->WriteString( "DS", "DS2Name", DS2Name );
 }
 catch(...)
 {
  result = false;
 }

 ini->~TIniFile();
 return result;
};
#endif
