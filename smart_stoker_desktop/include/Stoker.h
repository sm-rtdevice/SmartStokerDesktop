// Released by Sirkin I.S. 30.10.2014
#ifndef STOKER_H
#define STOKER_H

#include "CmdSystem.h"
#include "include\CRC8.h"

#define FF 1
#define oXFF 0xFF

bool CRC_OK(byte* RecvPacket, byte RecvPacketSize) // проверка контрольной суммы заглушка для команд в которых она не используется - использовать NULL - ТЕСТ
{
  return true;
};

bool CRC_8(byte* RecvPacket, byte RecvPacketSize) // проверка контрольной суммы заглушка для команд в которых она не используется - использовать NULL - ТЕСТ
{
  if( RecvPacket[RecvPacketSize-1] == crc8(RecvPacket, RecvPacketSize-1) ) {return true; } // вызов функции по указателю  // добавить -1 при 0xFF
  return false;
};

/*--- команда запрос температуры ---*/
class CMDGetTempers: public COMMANDS
{

public:

  void SetParam(int Delay = 500)           // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x01;          // 0x01 - запрос температуры
   Command->SendPacket[1] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDGetTempers(): COMMANDS(dtAT32,1+FF,3,CRC_8)    // конструктор 1,2:  1-отправлено, 2 - получено + CRC
  {
  };

  ~CMDGetTempers() // деструктор
  {
  };

  void GetTempers(char &T1, char &T2) // чтение температур
  {
   //int intT1 = Command->RecvPacket[0] | ((Command->RecvPacket[1] << 8) );
   //int intT2 = Command->RecvPacket[2] | ((Command->RecvPacket[3] << 8) );

   //T1 = (char)intT1;
   //T2 = (char)intT2;

   T1 = Command->RecvPacket[0];
   T2 = Command->RecvPacket[1];
      //Command->RecvPacket[2]; - CRC
  };

};
//---------------------------------------------------------------------------

/*--- команда запрос настроек ---*/
class CMDGetSettings: public COMMANDS
{

public:

  void SetParam(int Delay = 500) // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x02;          // 0x02 - запрос настроек
   Command->SendPacket[1] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDGetSettings(): COMMANDS(dtAT32,1+FF,9,CRC_8) // конструктор
  {
  };

  ~CMDGetSettings() // деструктор
  {
  };

  void GetSettings(unsigned char &MinT, unsigned char &MaxT, unsigned char &MinTCaldron, unsigned char &WorkTime, unsigned char &IdleTime, unsigned char &PulseMode, unsigned char &PumpMode, unsigned char &nStart) // чтение настроек
  {

   MinT = Command->RecvPacket[0];
   MaxT = Command->RecvPacket[1];
   MinTCaldron = Command->RecvPacket[2];
   WorkTime = Command->RecvPacket[3];
   IdleTime = Command->RecvPacket[4];
   PulseMode = Command->RecvPacket[5];
   PumpMode = Command->RecvPacket[6];
   nStart = Command->RecvPacket[7];
    // Command->RecvPacket[8]; - CRC
  };

};
//---------------------------------------------------------------------------

/*--- команда установка настроек ---*/
class CMDSetSettings: public COMMANDS
{

public:

  void SetParam(unsigned char MinT, unsigned char MaxT, unsigned char MinTCaldron, unsigned char WorkTime, unsigned char IdleTime, unsigned char PulseMode, int Delay = 500) // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x03;          // 0x03 - установка настроек
   Command->SendPacket[1] = MinT;          // температура включения насоса  45 (радиатор)
   Command->SendPacket[2] = MaxT;          // температура отключения насоса 55 (радиатор)
   Command->SendPacket[3] = MinTCaldron;   // минимальная температура воды в котле при котором возможно включение насоса
   Command->SendPacket[4] = WorkTime;      // время работы 5-10 мин
   Command->SendPacket[5] = IdleTime;      // время простоя 20-30 мин
   Command->SendPacket[6] = PulseMode;     // режим работы (от DS или импульсный)

   Command->SendPacket[7] = crc8(Command->SendPacket,7);     // CRC 7-1
   Command->SendPacket[8] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDSetSettings(): COMMANDS(dtAT32,8+FF,1,CRC_OK) // конструктор
  {
  };

  ~CMDSetSettings() // деструктор
  {
  };

  byte Result(){return Command->RecvPacket[0];} // если ОК то возвращает - 0xAA

};
//---------------------------------------------------------------------------

/*--- команда запрос серийных номеров датчиков DS1820 ---*/
class CMDGetSN: public COMMANDS
{

public:

  void SetParam(int Delay = 500) // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x04;          // 0x04 - запрос SN
   Command->SendPacket[1] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDGetSN(): COMMANDS(dtAT32,1+FF,17,CRC_8)        // конструктор
  {
  };

  ~CMDGetSN() // деструктор
  {
  };

  void GetSN(byte* SN1, byte* SN2)
  {

   for(int i = 0; i < 8; i++)
   {
    SN1[i] = Command->RecvPacket[i+i];    // радиатор
    SN2[i] = Command->RecvPacket[i+i+1];  // котел
    //Sleep(10);
   }

   // Command->RecvPacket[16] - CRC

   /*
   for(int i = 0; i < 8; i++)
   {
    SN1[i] = Command->RecvPacket[i]; // радиатор
   }

   for(int i = 8; i < 16; i++)
   {
    SN2[i-8] = Command->RecvPacket[i]; // радиатор
   }
   */

  };

};
//---------------------------------------------------------------------------

/*--- команда установка серийных номеров датчиков DS1820 ---*/
class CMDSetSN: public COMMANDS
{

public:

  void SetParam(byte* SN1, byte* SN2, int Delay = 500) // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x05;          // 0x05 - установка SN

   for(int i = 0; i < 8; i++)
   {
    Command->SendPacket[i+i+1] = SN1[i];   // радиатор
    Command->SendPacket[i+i+1+1] = SN2[i]; // котел
   }

   Command->SendPacket[17] = crc8(Command->SendPacket, 17);     // CRC  17-1 - проверить  17!
   Command->SendPacket[18] = oXFF;       // конец команды

   /*
   for(int i = 0; i < 8; i++)
   {
    Command->SendPacket[i+1] = SN1[i]; // радиатор
   }

   for(int i = 8; i < 16; i++)
   {
    Command->SendPacket[i+1] = SN2[i-8]; // радиатор
   }
   */

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDSetSN(): COMMANDS(dtAT32,18+FF,1,CRC_OK)        // конструктор
  {
  };

  ~CMDSetSN() // деструктор
  {
  };

  byte Result(){return Command->RecvPacket[0];} // если ОК то возвращает - 0xAA
};
//---------------------------------------------------------------------------

/*--- команда сброс счетчика перезапусков ---*/
class CMDResetCounter: public COMMANDS
{

public:

  void SetParam(int Delay = 500)         // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x06;        // 0x06 - сброс счетчика
   Command->SendPacket[1] = oXFF;        // конец команды

   Command->Delay = Delay;               // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDResetCounter(): COMMANDS(dtAT32,1+FF,1,CRC_OK) // конструктор
  {
  };

  ~CMDResetCounter() // деструктор
  {
  };

  byte Result(){return Command->RecvPacket[0];} // если ОК то возвращает - 0xAA
};
//---------------------------------------------------------------------------

/*--- команда установка режима работы насоса ---*/  // - команда больше не нужна!
/*class CMDSetPumpMode: public COMMANDS
{

public:

  void SetParam(byte Mode, int Delay = 500)// установка параметров комманды:
  {
   Command->SendPacket[0] = 0x07;          // 0x07 - установка режима работы насоса
   Command->SendPacket[1] = Mode;          // 0 - авто; 1 - принудительное включение; 2 - принудительное отключение
   Command->SendPacket[2] = crc8(Command->SendPacket,2);     // CRC
   Command->SendPacket[3] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);     //
  };

  CMDSetPumpMode(): COMMANDS(dtAT32,3+FF,1,CRC_OK)   // конструктор 1...9
  {
  };

  ~CMDSetPumpMode() // деструктор
  {
  };

  byte Result(){return Command->RecvPacket[0];} // если ОК то возвращает - 0xAA

}; */
//---------------------------------------------------------------------------

/*--- команда запрос режима работы насоса ---*/
/*
class CMDGetPumpMode: public COMMANDS
{ // 0 - авто; 1 - принудительное включение; 2 - принудительное отключение

public:

  void SetParam(int Delay = 500)           // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x08;          // 0x08 - запрос режима работы насоса

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);     //
  };

  CMDGetPumpMode(): COMMANDS(dtAT32,1,1)   //
  {
  };

  ~CMDGetPumpMode() // деструктор
  {
  };

  void PumpMode(unsigned char &Mode)       // установленный режим работы
  {
   Mode = Command->RecvPacket[0];
  }

};
*/
//---------------------------------------------------------------------------

/*--- команда проверка связи ---*/
class CMDTestLink: public COMMANDS
{

public:

  void SetParam(Delay = 3000)              // установка параметров комманды:
  {
   Command->SendPacket[0] = 0x09;          //
   Command->SendPacket[1] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDTestLink(): COMMANDS(dtAT32,1+FF,14,CRC_OK) // конструктор
  {
  };

  ~CMDTestLink() // деструктор
  {
  };

  void Request(byte* TestStr) // =SMART-DEVICE=
  {
   TestStr[0] = Command->RecvPacket[0];
   TestStr[1] = Command->RecvPacket[1];
   TestStr[2] = Command->RecvPacket[2];
   TestStr[3] = Command->RecvPacket[3];
   TestStr[4] = Command->RecvPacket[4];
   TestStr[5] = Command->RecvPacket[5];
   TestStr[6] = Command->RecvPacket[6];
   TestStr[7] = Command->RecvPacket[7];
   TestStr[8] = Command->RecvPacket[8];
   TestStr[9] = Command->RecvPacket[9];
   TestStr[10] = Command->RecvPacket[10];
   TestStr[11] = Command->RecvPacket[11];
   TestStr[12] = Command->RecvPacket[12];
   TestStr[13] = Command->RecvPacket[13];
  }

};
//---------------------------------------------------------------------------

class CMDGetLogTemp: public COMMANDS
{

public:

  void SetParam(Delay = 5000)              // 0x0A - получение температуры (Архив ~12 часов):
  {
   Command->SendPacket[0] = 0x0A;
   Command->SendPacket[1] = oXFF;          // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDGetLogTemp(): COMMANDS(dtAT32,1+FF,750,CRC_OK) // конструктор CRC_OK
  {
  };

  ~CMDGetLogTemp() // деструктор
  {
  };

  void Request(byte* TestStr)
  {
   /*
   TestStr[0] = Command->RecvPacket[0]; // T1
   TestStr[1] = Command->RecvPacket[1]; // T2..750
   */
  }

};

class CMDResetCmd: public COMMANDS
{
 private:
  static const int CountoXFF = 18;         // максимальная длина команд протокола передачи

public:

  void SetParam( Delay = 0)                // oXFF - сброс цикла парсинга команд контроллера (холостая команда)
  {                                        // возврат к ожиданию команды при сбое предыдущей команды
   for(int i = 0; i < CountoXFF; i++){Command->SendPacket[i] = oXFF;}  // конец команды

   Command->Delay = Delay;                 // мс
   ResetEvent(Command->EndCmd.hEvent);
  };

  CMDResetCmd(): COMMANDS(dtAT32, CountoXFF, 0, CRC_OK) // конструктор CRC_OK, нет возвращаемых данных
  {
  };

  ~CMDResetCmd() // деструктор
  {
  };

  //void Request(byte* TestStr)              // нет возвращаемых данных
  //{
  //}

};

#endif // STOKER_H
