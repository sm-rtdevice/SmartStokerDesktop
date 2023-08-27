// Released by Sirkin I.S.: 29.10.14
// установить FILE_ATTRIBUTE_NORMAL - для Send, FILE_FLAG_OVERLAPPED - для Sinhrosend иначе виснет KERNEL

#ifndef CCOMPORT_H
#define CCOMPORT_H

// рпспиновка DB-9:
// DCD - (1) обнаружена несущая  (T+)(вход)
// RxD - (2) прием данных        (R+)(вход)
// TxD - (3) передача данных     (T-)(выход)
// DTR - (4) приемник готов          (выход)  Компьютер готов. Линия взаимодействия, которая показывает, что компьютер включен и готов к связи
// GND - (5) земля                   (общий)
// DSR - (6) передатчик готов    (R-)(вход)   Готовность внешнего устройства. Линия взаимодействия, с помощью которой внешнее устройство сообщает компьютеру, что оно включено и готово к связи.
// RTS - (7) запрос отправки данных  (выход)  Запрос на передачу. Линия взаимодействия, которая показывает, что компьютер готов к приему данных
// CTS - (8) готов к передаче        (вход)   Готовность к передаче. Линия взаимодействия, с помощью которой внешнее устройство сообщает компьютеру, что оно готово к передаче данных.
// RING -(9) обнаружен сигнал вызова (вход)   устройство вызывает хост

// baud: 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000

#pragma once
#include <windows.h>
#define MAX_WIN_PORT 255
#include <stdio.h>

// определение доступных портов в системе
struct CPort
{
 unsigned char Id; // Номер порта
 bool Available;   // состояние: (доступен | не используется)
};

/*
struct PortInfo
{
	CPort Port[MAX_WIN_PORT];   // bool AvailablePorts[MAX_WIN_PORT]
	unsigned char Count;        // количество найденных портов
};
*/

class CComPortFinder            // CComPortFinder PCPorts - com порты компьютера
{
public:
  void GetPortsInfo();          // определение доступных портов в системе
  // PortInfo *Ports;           // список портов

  CPort Port[MAX_WIN_PORT];     // bool AvailablePorts[MAX_WIN_PORT]
  unsigned char Count;          // количество найденных портов
  unsigned char CountAvailable; // количество найденных портов

  CComPortFinder();             // конструктор
  ~CComPortFinder();            // деструктор
};

void CComPortFinder::GetPortsInfo() // Поиск информации о портах
{
  DWORD  Ports_MemSize = 0;
  DWORD  Ports_Count   = 0;
  BYTE*  lpPorts       = NULL;

  // Getting Ports_MemSize value...
  EnumPorts(NULL, 1, lpPorts, 0, &Ports_MemSize, &Ports_Count);

  // Getting lpPorts...
  lpPorts = new BYTE[Ports_MemSize];
  EnumPorts(NULL, 1, lpPorts, Ports_MemSize, &Ports_MemSize, &Ports_Count);

  // Forming List Of Ports...
  DWORD dw;
  TCHAR str[8];
  char temp[4];
  int port = -1;
  PORT_INFO_1 *pPortInfo;
  pPortInfo = (PORT_INFO_1 *)lpPorts;
  int counter = 0;     // всего
  CountAvailable = 0;  // доступных
  bool av = false;

  for (dw = 0; dw < Ports_Count; dw++)
  {
    lstrcpyn(str, pPortInfo->pName, 4);
    str[4] = 0;
    if (lstrcmpi(str, "com") == 0)
    {
	 //printf("%s\n", pPortInfo->pName);
     memset(temp, '\0', sizeof(temp));
     temp[0] = pPortInfo->pName[3];
     if(pPortInfo->pName[4]!= ':' && pPortInfo->pName[4]!= '\0'){ temp[1] = pPortInfo->pName[4]; };
     if(pPortInfo->pName[5]!= ':' && pPortInfo->pName[5]!= '\0'){ temp[2] = pPortInfo->pName[5]; };
     port = atoi(temp);
		//printf("%d\n", port);

     char COM_string[20];
     sprintf(COM_string,"\\\\.\\COM%d", port);

     HANDLE h = CreateFile(COM_string, GENERIC_READ|GENERIC_WRITE, 0, NULL,
     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

     if(h == INVALID_HANDLE_VALUE){av = false;}
     else{av = true; CloseHandle(h); CountAvailable++ ;	}

     Port[counter].Id = port;
     Port[counter].Available = av;
     counter++;
    }

    pPortInfo++;
  }

  Count = counter;
  delete [] lpPorts;
}

CComPortFinder::CComPortFinder() {
}

CComPortFinder::~CComPortFinder() {
}
//===========================================================================

enum IN_LINES_FLAGS {CTS, DSR, RING, RLSD};       // флаги состояний: (готов к передаче, передатчик готов, ,  DCD)
enum OUT_LINES_FLAGS {DTR, RTS};                  //                  (приемник готов, запрос отправки данных)

class CComPort
{
public:
  CComPort();      // конструктор
  ~CComPort();     // деструктор

  DCB dcb;         // структура для общей инициализации порта DCB

  unsigned char PortCount; // количество портов
  unsigned char* Port;     // номера (id) доступных com портов

  void FindCom();

  bool Open(int port, int baud);                                              // открыть порт по номеру
  bool Open(char* port, int baud);                                            // открыть порт по имени

  bool Open(char* port, int baud, Byte ByteSize, Byte Parity, Byte StopBits); // открыть порт по имени c доп. параметрами
  bool Open(int port, int baud, Byte ByteSize, Byte Parity, Byte StopBits);   // открыть порт по номеру c доп. параметрами

  DWORD Send(char* buff, int szBuff);                                         // отправить данные, работает только с FILE_ATTRIBUTE_NORMAL
  DWORD Recv(char* buff, int szBuff);                                         // получить данные,  работает только с FILE_ATTRIBUTE_NORMAL

  DWORD RecvAsync(char* buff, int szBuff, OVERLAPPED* overlapped);            // асинхронный прием с доп. параметрами, FILE_FLAG_OVERLAPPED

  DWORD SendAsync(char* buff, int szBuff, OVERLAPPED* overlapped);            // асинхронная передача с доп. параметрами, FILE_FLAG_OVERLAPPED
  DWORD SendDataAsync(char* buff, int szBuff);                                // асинхронная передача, overlapped объявлен внутри метода, FILE_FLAG_OVERLAPPED

  DWORD RecvSync(char* buff, DWORD szBuff, DWORD TimeOut, DWORD &Result);     // прочитать из порта буфер buff размером szBuff за время TimeOut, Result - результат чтения из порта
  DWORD RecvDataSync(char* buff, DWORD szBuff, DWORD TimeOut);                // прочитать из порта буфер buff размером szBuff за время TimeOut, возвращает DWORD &Result - результат чтения из порта

  void SetLine(OUT_LINES_FLAGS ln, bool state);   // установить параметры линии
  bool GetLine(IN_LINES_FLAGS ln);                // получить параметры линии

  bool SetDCB(int baud);                          // установить параметры передачи COM-порта

  void Close(); // закрыть порт
  void Clean(); // очистить буферы порта
  bool Opened;  // состояние порта - открыт/закрыт

private:
  HANDLE COM_Handle;
};

bool CComPort::Open(char* COM_string, int baud) // открыть порт
{
  COMMTIMEOUTS CommTimeOuts;    // структура для установки таймаутов

  // FILE_ATTRIBUTE_NORMAL - для Send, FILE_FLAG_OVERLAPPED - для Sinhrosend иначе виснет KERNEL
  COM_Handle = CreateFile(COM_string /*"\\\\.\\Com10"*/,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/ FILE_FLAG_OVERLAPPED, NULL);  // до 10 - "Com9"; после 10 - "\\\\.\\Com10"
  // - portname.c_str() - имя порта в качестве имени файла
  // - GENERIC_READ | GENERIC_WRITE - доступ к порту на чтение/записть
  // - 0 - порт не может быть общедоступным (shared)
  // - NULL - дескриптор порта не наследуется, используется дескриптор безопасности по умолчанию
  // - OPEN_EXISTING - порт должен открываться как уже существующий файл
  // - FILE_FLAG_OVERLAPPED - использование асинхронных операций, FILE_ATTRIBUTE_NORMAL - синхро
  // - NULL - указатель на файл шаблона не используется при работе с портами

  if(COM_Handle == INVALID_HANDLE_VALUE){return false;} // не удалось открыть порт

  SetupComm(COM_Handle,4096,4096); // установить размеры очередей приёма и передачи, Размер передаваемого буфера не должен превышать размер очереди передачи!
  PurgeComm(COM_Handle, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR); // очистить все буферы порта

  if(!GetCommState(COM_Handle, &dcb)){return false;} // не удалось считать структуру DCB из порта

  // установить таймауты
  CommTimeOuts.ReadIntervalTimeout	   = 20;     // таймаут между двумя символами | MAXDWORD
  CommTimeOuts.ReadTotalTimeoutMultiplier  = 60; // общий таймаут операции чтения
  CommTimeOuts.ReadTotalTimeoutConstant    = 70; // константа для общего таймаута операции чтения
  CommTimeOuts.WriteTotalTimeoutMultiplier = 20; // общий таймаут операции записи
  CommTimeOuts.WriteTotalTimeoutConstant   = 30; // константа для общего таймаута операции записи

  if(!SetCommTimeouts(COM_Handle, &CommTimeOuts)) // записать структуру таймаутов в порт
  {
   return false; // Не удалось установить тайм-ауты
  }

  // установить параметры передачи  (по умолчанию в конструкторе)
  dcb.BaudRate = baud;                  // скорость передачи бит/с. (бод)
  dcb.ByteSize = 8;                     // установить 8 бит в байте
  dcb.Parity   = NOPARITY;              // отключить проверку чётности
  dcb.StopBits = ONESTOPBIT;            // установить один стоп-бит

  /*
  dcb.fBinary = TRUE;                   // включить двоичный режим обмена
  dcb.fOutxCtsFlow = FALSE;             // включить режим слежения за сигналом CTS  (отключить)
  dcb.fOutxDsrFlow = FALSE;             // включить режим слежения за сигналом DSR  (отключить)
  dcb.fDtrControl = DTR_CONTROL_DISABLE;// отключить использование линии DTR
  dcb.fDsrSensitivity = FALSE;          // отключить восприимчивость драйвера к состоянию линии DSR
  dcb.fNull = FALSE;                    // разрешить приём нулевых байтов (отключить)
  dcb.fRtsControl = RTS_CONTROL_DISABLE;// отключить использование линии RTS
  dcb.fAbortOnError = FALSE;            // отключить остановку всех операций чтения/записи при ошибке
  */

  dcb.fDtrControl = DTR_CONTROL_ENABLE; // включить использование линии DTR

  dcb.XonLim = 100;  // 2048
  dcb.XoffLim = 100; // 512

  if(!SetCommState(COM_Handle, &dcb)){return false;} // не удалось загрузить структуру DCB в порт

  this->Opened = true;
  return true;
}
//---------------------------------------------------------------------------

bool CComPort::Open(int port, int baud) // открыть порт
{
  char COM_string[10];
  if(port < 10){ sprintf(COM_string,"COM%d", port); } else { sprintf(COM_string,"\\\\.\\COM%d", port); } //... COM9, \\\\.\\COM10 ...
  return Open(COM_string, baud);
}
//---------------------------------------------------------------------------

bool CComPort::Open(char* COM_string, int baud, Byte ByteSize, Byte Parity, Byte StopBits) // открыть порт
{
  COMMTIMEOUTS CommTimeOuts;    // структура для установки таймаутов

  // FILE_ATTRIBUTE_NORMAL - для Send, FILE_FLAG_OVERLAPPED - для Sinhrosend иначе виснет KERNEL
  COM_Handle = CreateFile(COM_string/*"\\\\.\\Com10"*/,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/ FILE_FLAG_OVERLAPPED, NULL);

  if(COM_Handle == INVALID_HANDLE_VALUE){return false;} // не удалось открыть порт

  SetupComm(COM_Handle,4096,4096); // установить размеры очередей приёма и передачи, Размер передаваемого буфера не должен превышать размер очереди передачи!
  PurgeComm(COM_Handle, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR); // очистить все буферы порта

  if(!GetCommState(COM_Handle, &dcb)){return false;} // не удалось считать структуру DCB из порта

  /*
  // установить таймауты
  CommTimeOuts.ReadIntervalTimeout	   = MAXDWORD; // таймаут между двумя символами
  CommTimeOuts.ReadTotalTimeoutMultiplier  = 0;        // общий таймаут операции чтения
  CommTimeOuts.ReadTotalTimeoutConstant    = 0;        // константа для общего таймаута операции чтения
  CommTimeOuts.WriteTotalTimeoutMultiplier = 0;        // общий таймаут операции записи
  CommTimeOuts.WriteTotalTimeoutConstant   = 1000;     // константа для общего таймаута операции записи

  if(!SetCommTimeouts(m_hFile, &CommTimeOuts))         // записать структуру таймаутов в порт
  {
   return; // Не удалось установить тайм-ауты
  }
  */

  // установить параметры передачи  (по умолчанию в конструкторе)
  dcb.BaudRate = baud;                // скорость передачи бит/с. (бод)
  dcb.ByteSize = ByteSize;            // установить 8 бит в байте
  dcb.Parity   = Parity;              // отключить проверку чётности
  dcb.StopBits = StopBits;            // установить один стоп-бит

  /*
  dcb.fBinary = TRUE;                   // включить двоичный режим обмена
  dcb.fOutxCtsFlow = FALSE;             // включить режим слежения за сигналом CTS  (отключить)
  dcb.fOutxDsrFlow = FALSE;             // включить режим слежения за сигналом DSR  (отключить)
  dcb.fDtrControl = DTR_CONTROL_DISABLE;// отключить использование линии DTR
  dcb.fDsrSensitivity = FALSE;          // отключить восприимчивость драйвера к состоянию линии DSR
  dcb.fNull = FALSE;                    // разрешить приём нулевых байтов (отключить)
  dcb.fRtsControl = RTS_CONTROL_DISABLE;// отключить использование линии RTS
  dcb.fAbortOnError = FALSE;            // отключить остановку всех операций чтения/записи при ошибке
  */

  dcb.fDtrControl = DTR_CONTROL_ENABLE; // включить использование линии DTR

  dcb.XonLim = 100;     // 2048
  dcb.XoffLim = 100;    // 512

  if(!SetCommState(COM_Handle, &dcb)){return false;} // не удалось загрузить структуру DCB в порт

  this->Opened = true;
  return true;  // все ОК
}
//---------------------------------------------------------------------------

bool CComPort::Open(int port, int baud, Byte ByteSize, Byte Parity, Byte StopBits) // открыть порт
{
  char COM_string[10];
  sprintf(COM_string,"COM%d", port);  // sprintf(COM_string,"\\\\.\\COM%d", port);
  return Open(COM_string, baud, ByteSize, Parity, StopBits);
}
//---------------------------------------------------------------------------

CComPort::CComPort()
{
  Opened = false;
  this->PortCount = 0;
}

CComPort::~CComPort()
{
  Close();
  if(this->PortCount != 0){delete [](this->Port); }
}
//---------------------------------------------------------------------------

void CComPort::Close()
{
  if(Opened == false){return;} // если уже закрыт
  this->Clean();               // очистить буферы порта
  Opened = false;
  CloseHandle(COM_Handle);
}
//---------------------------------------------------------------------------

void CComPort::Clean()
{
  if(!Opened){return;};
  PurgeComm(COM_Handle, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR); // очистить принимающий буфер порта - PURGE_RXCLEAR, передающий буфер - PURGE_TXCLEAR, PURGE_TXABORT|PURGE_RXABORT - ?
 // PurgeComm(COM_Handle, PURGE_TXABORT|PURGE_RXABORT); // очистить
}
//---------------------------------------------------------------------------

DWORD CComPort::Send(char* buff, int szBuff)    // отправить данные в порт char строка + 00 + CF
{ // byte buff[1]; buff[0] = 0xFF;  - буфер для отправки
  if(!Opened) return 0;                         // порт закрыт
  if(buff == NULL || szBuff <= 0){return 0;}    // буфер пуст

  DWORD lpdwBytesWrittens = 0;                  // количество отправленных байтов
  WriteFile(COM_Handle, buff, szBuff, &lpdwBytesWrittens, NULL);

  return lpdwBytesWrittens;
}
//---------------------------------------------------------------------------

DWORD CComPort::Recv(char* buff, int szBuff)      // получить данные из порта
{
  if(!Opened)return 0;                          // порт закрыт
  if(buff == NULL || szBuff <= 0){return 0;}    // буфер пуст

  DWORD dwBytesRead = 0;
  ReadFile(COM_Handle, buff, szBuff, &dwBytesRead, NULL);

  return dwBytesRead;
}
//---------------------------------------------------------------------------

void CComPort::SetLine(OUT_LINES_FLAGS ln, bool state)
{ //EscapeCommFunction(m_hFile, SETRTS); || EscapeCommFunction(m_hFile, CLRRTS);
  if(ln == DTR)
  {
   if(state){EscapeCommFunction(COM_Handle, SETDTR);}else{EscapeCommFunction(COM_Handle, CLRDTR);};
  };

  if(ln == RTS)
  {
   if(state){EscapeCommFunction(COM_Handle, SETRTS);}else{EscapeCommFunction(COM_Handle, CLRRTS);};
  };
}
//---------------------------------------------------------------------------

bool CComPort::GetLine(IN_LINES_FLAGS ln)
{
  if(!Opened)return 0;
  unsigned long ul = 0;

  GetCommModemStatus(COM_Handle, &ul);

  if(ul == 0x10 && ln == CTS){return true;}
  if(ul == 0x20 && ln == DSR){return true;}
  if(ul == 0x40 && ln == RING){return true;}
  if(ul == 0x80 && ln == RLSD){return true;}

  return false;
}
//---------------------------------------------------------------------------

DWORD CComPort::SendAsync(char* buff, int szBuff, OVERLAPPED* overlapped)
{
 if(!Opened){/*SetEvent(overlapped->hEvent);*/ return 0;}                       // порт закрыт
 if(buff == NULL || szBuff <= 0){/*SetEvent(overlapped->hEvent);*/return 0;}    // буфер пуст

 DWORD lpdwBytesWrittens = 0;                           // количество отправленных байтов
 WriteFile(COM_Handle, buff, szBuff, &lpdwBytesWrittens, overlapped);

 return lpdwBytesWrittens;
}
//---------------------------------------------------------------------------

DWORD CComPort::SendDataAsync(char* buff, int szBuff)                           // Асинхронная запись в порт
{
 OVERLAPPED OverSend;
 OverSend.hEvent = CreateEvent(NULL, false/*true*/, /*true*/false, NULL);       // ожидание завершения чтения из порта (по умолчанию не установлено)
 //ResetEvent(OverSend.hEvent);
 DWORD RES = SendAsync(buff, szBuff, &OverSend);                                // отсылка пакета на устройство
 CloseHandle(OverSend.hEvent);                                                  // закрыть объект-событие (ожидание приема ответа)

 return RES;
}
//---------------------------------------------------------------------------

DWORD CComPort::RecvAsync(char* buff, int szBuff, OVERLAPPED* overlapped)
{
  if(!Opened){/*SetEvent(overlapped->hEvent);*/ return 0;}                      // порт закрыт
  if(buff == NULL || szBuff <= 0){/*SetEvent(overlapped->hEvent);*/ return 0;}  // буфер пуст

  DWORD dwBytesRead = 0;
  ReadFile(COM_Handle, buff, szBuff, &dwBytesRead, overlapped);

  return dwBytesRead;
}
//---------------------------------------------------------------------------

DWORD CComPort::RecvSync(char* buff, DWORD szBuff, DWORD TimeOut, DWORD &Result) // ожидать получение ответа в течение времени TimeOut и вернуть количество прогчитанных байт
{                                                                                // Result - результат ожидания
  if(!Opened || buff == NULL || szBuff <= 0){Result = WAIT_ABANDONED; return 0;} // порт закрыт или буфер пуст

  bool exit = false;                  // завершение ожидания
  DWORD dwBytesRead = 0;              // всего прочитано из порта
  DWORD BeginTime = GetTickCount();   // начало ожидания
  DWORD CurrentTime;                  // текущее время
  DWORD Wait;                         // остаток времени ожидания

  COMSTAT comstat;		      // структура текущего состояния порта используется для определения количества принятых в порт байтов
  DWORD btr, temp, mask, signal;      // переменная temp используется в качестве заглушки

  SetCommMask(COM_Handle, EV_RXCHAR); // установить маску на срабатывание по событию приёма байта в порт

  OVERLAPPED overlapped;
  overlapped.hEvent = CreateEvent(NULL, true, true, NULL); // создать сигнальный объект-событие для асинхронных операций (по умолчанию установлено)

  while(!exit)			                                    // пока поток не будет прерван, выполняем цикл
  {
   WaitCommEvent(COM_Handle, &mask, &overlapped);           // ожидать события приёма байта (это и есть перекрываемая операция)

   CurrentTime = GetTickCount();
   if(CurrentTime < BeginTime){BeginTime = GetTickCount(); CurrentTime = GetTickCount();}// переполнение GetTickCount()
   if(TimeOut > (CurrentTime - BeginTime) ){Wait = TimeOut - (CurrentTime - BeginTime);} // продолжить ожидание поступления байтов
   else{ Wait = 0; }                                                                     // время ожидания истекло
  // {
  //  Result = WAIT_TIMEOUT;
  //  exit = true;
    //break; // из while(1)
  // }

   signal = WaitForSingleObject(overlapped.hEvent, Wait);	  // приостановить поток до прихода байта

   if(signal == WAIT_OBJECT_0)				          // если событие прихода байта произошло
   {
    if(GetOverlappedResult(COM_Handle, &overlapped, &temp, true)) // проверяем, успешно ли завершилась перекрываемая операция WaitCommEvent
    {
     if((mask & EV_RXCHAR)!=0)					  // если произошло именно событие прихода байта
     {
      ClearCommError(COM_Handle, &temp, &comstat);		  // нужно заполнить структуру COMSTAT
      btr = comstat.cbInQue;                          	          // и получить из неё количество принятых байтов
      if(btr)                         			          // если действительно есть байты для чтения
      {
       if(dwBytesRead + btr > szBuff){Result = WAIT_ABANDONED; exit = true; break;} // Защита от переполнения буфера  !!!!!!!!!!!!!!!!!!!

       ReadFile(COM_Handle, buff + dwBytesRead, btr, &temp, &overlapped);           // прочитать байты из порта в буфер программы   !!!!!!!!!!!!!!!!!!!
       dwBytesRead += btr;                                        // увеличиваем счётчик байтов

       if(dwBytesRead == szBuff) // установленный блок прочитан из com порта dwBytesRead >= szBuff - читануть больше
       {
        Result = WAIT_OBJECT_0;  // ОК
        exit = true;
       }

      }
     }
    }
   }
   else{ Result = signal; exit = true; };  // нет ответа от устройства com порта

  } // while(1)

  CloseHandle(overlapped.hEvent); // закрыть объект-событие (ожидание приема ответа)
  return dwBytesRead;
}
//---------------------------------------------------------------------------

DWORD CComPort::RecvDataSync(char* buff, DWORD szBuff, DWORD TimeOut) // ожидать получение ответа в течение времени TimeOut и вернуть результат ожидания
{
 DWORD RES;
 RecvSync(buff, szBuff, TimeOut, RES); // ожидать получение ответа в течение времени TimeOut и вернуть результат ожидания
 return RES;
}
//---------------------------------------------------------------------------

void CComPort::FindCom()
{
 CComPortFinder PCPorts;
 PCPorts.GetPortsInfo(); // найти все порты

 if(this->PortCount != 0){delete [](this->Port); }

 this->Port = new unsigned char[PCPorts.CountAvailable];
 this->PortCount = PCPorts.CountAvailable;

 for(int i=0; i<PCPorts.Count; i++)  // выбрать доступные
 {
  if(PCPorts.Port[i].Available)
  {
   this->Port[i] = PCPorts.Port[i].Id;
  }
 }

}
//---------------------------------------------------------------------------

bool CComPort::SetDCB(int baud)       // установить параметры передачи COM-порта
{
  // установить параметры передачи  (по умолчанию в конструкторе)
  dcb.BaudRate = baud;                // скорость передачи бит/с. (бод)
//  dcb.ByteSize = ByteSize;            // установить 8 бит в байте
//  dcb.Parity   = Parity;              // отключить проверку чётности
//  dcb.StopBits = StopBits;            // установить один стоп-бит

  /*
  dcb.fBinary = TRUE;                   // включить двоичный режим обмена
  dcb.fOutxCtsFlow = FALSE;             // включить режим слежения за сигналом CTS  (отключить)
  dcb.fOutxDsrFlow = FALSE;             // включить режим слежения за сигналом DSR  (отключить)
  dcb.fDtrControl = DTR_CONTROL_DISABLE;// отключить использование линии DTR
  dcb.fDsrSensitivity = FALSE;          // отключить восприимчивость драйвера к состоянию линии DSR
  dcb.fNull = FALSE;                    // разрешить приём нулевых байтов (отключить)
  dcb.fRtsControl = RTS_CONTROL_DISABLE;// отключить использование линии RTS
  dcb.fAbortOnError = FALSE;            // отключить остановку всех операций чтения/записи при ошибке
  */

  dcb.fDtrControl = DTR_CONTROL_ENABLE; // включить использование линии DTR

  dcb.XonLim = 100;     // 2048
  dcb.XoffLim = 100;    // 512

  if(!SetCommState(COM_Handle, &dcb)){return false;} // не удалось загрузить структуру DCB в порт

  return true;
}

#endif
