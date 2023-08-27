// Released by Sirkin I.S. 27.05.2014
#ifndef CMDSYSTEM_H
#define CMDSYSTEM_H

#include "Identifiers.h"
#include "DynamicData.h"

/*--- коммандный пакет для отправки ---*/
class CmdPacket
{
  public:
  byte* SendPacket;     // командный пакет
  byte* RecvPacket;     // возвращаемые данные
  byte  SendPacketSize; // размер командного пакета
  byte  RecvPacketSize; // размер возвращаемых данных
  DevTYPE DeviceType;   // тип устройства

  CMDRES Result;        // результат выполнения команды

  unsigned long Delay;  // задержка на выполнение комманды
  OVERLAPPED EndCmd;    // ожидание завершения комманды

  pCRCFunk CRCFunk;     // указатель на функцию проверки CRC

  bool CRC()            // расчет CRC для любого устройства
  {
   if(CRCFunk == NULL){return false;};
   return (*CRCFunk)(RecvPacket, RecvPacketSize);  // вызов функции по указателю
  }

  void CmdPacket::ExecSuccess() // когда получен ответ на команду (выполнена удачно)
  {
   Result = crSuccess;       // команда выполнена удачно
   SetEvent(EndCmd.hEvent);  // событие произошло
  };

  void CmdPacket::ExecError()// (неудачное выполнение)
  {
   Result = crError;         // команда выполнена не удачно
   SetEvent(EndCmd.hEvent);  // событие произошло
  };

  void CmdPacket::ExecTimeOut() // (таймаут выполнения)
  {
   Result = crTimeOut;
   SetEvent(EndCmd.hEvent); // событие произошло
  };

  void CmdPacket::ExecCRCError() //(ошибка в контрольной сумме)
  {
   Result = crCRCError;
   SetEvent(EndCmd.hEvent); // событие произошло
  };

  CmdPacket::CmdPacket(DevTYPE devtype, byte SendByfSize, byte RecvByfSize, pCRCFunk inCRCFunk) // пакеты для всех типов устройств
  {
   DeviceType = devtype;                // тип устройства
   CRCFunk = inCRCFunk;                 // установить указатель на функцию

   SendPacket = new byte[SendByfSize];  // динамический размер отсылаемого пакета
   SendPacketSize = SendByfSize;

   RecvPacket = new byte[RecvByfSize];  // динамический размер принимаемого пакета
   RecvPacketSize = RecvByfSize;

   //for(int i = 0; i<RecvByfSize; i++){RecvPacket[i] = 0xFF;};  // "обнуление" тест
  };

  CmdPacket::~CmdPacket()
  {
    delete[](SendPacket);  // освобождение памяти
    delete[](RecvPacket);  // освобождение памяти
  };
};
//---------------------------------------------------------------------------

/*--- базовое описание всех синхронных команд ---*/
class COMMANDS
{
  public:
  CmdPacket *Command;   // указатель на выполняемую команду

  CMDRES COMMANDS::ExecCmd(CQueue <CmdPacket*> *CmdBuf) // выполнить команду (синхронно)
  {
    ResetEvent(Command->EndCmd.hEvent); // событие не наступило

    if(CmdBuf->InsR(this->Command) == false) // добавить команду в буфер команд
    {
     return crBufError; // не добавилось (буфер команд переполнен), повторить команду позже
    }

    DWORD signal = 0;  // результат ожидания
    signal = WaitForSingleObject(Command->EndCmd.hEvent,5000); // ждать выполнения команды  5000 INFINITE - аварийный таймаут (если буфер команд переполнен)

    if(signal == WAIT_OBJECT_0) // команда выполнена во время
    {
      // RecvPacket уст. в процедуре дешифраторе команд(возвращаемые данные командой)
      // Result = crSuccess || crError

     return Command->Result;  // RecvPacket - обрабатывать результат команды если crSuccess
    }
    else if(signal == WAIT_TIMEOUT) // превышено время ожидания, нет ответа от устройства
    {
     return crInfiniteTimeOut; // RecvPacket не учитывать  crInfiniteTimeOut - превышен аварийный таймаут
    }

   return crError; // непредвиденное событие
  };

  COMMANDS::COMMANDS(DevTYPE devtype, byte SendByfSize, byte RecvByfSize, pCRCFunk CRCFunk)
  {
   Command = new CmdPacket(devtype, SendByfSize, RecvByfSize, CRCFunk); // тип устройства, размар посылаемого пакета, размер принимаемого пакета
   Command->EndCmd.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // ожидание завершения очередной комманды (создать объект событие)
  };

  COMMANDS::~COMMANDS() // деструктор
  {
   CloseHandle(Command->EndCmd.hEvent);
   delete(Command);
   Command = NULL;
  };

};

#include "CComPort.h"
#include "Settings.h"

/*---Глобальные процедуры---*/
extern void SaveEvent(char* msg);
extern void SaveDeviceError(char* msg, DWORD DevAddr, DevTYPE DeviceType); // сохранение ошибок устройств в лог файл

/*======================================== CPU ATMega32 USART-ядро =========================================================*/

class CmdSystem_AT32
{
 public:
  CQueue <CmdPacket*> UserCmdBuf;       // буффер команд пользователя  // создать 3 буфер для обновления параметров устройств, для надежности, не использовать UserCmdBuf
  CQueue <CmdPacket*> SystemCmdBuf;     // буффер команд системный
  CQueue <CmdPacket*> CurrentCmdBuf;    // буффер для текущих команд

  CmdPacket *CurrentCmd;                // указатель на команду из буфера todo: объявить локально
  CComPort* ComPort;                    // указатель на ком порт класс
  CSettings* Settings;                  // таймауты, пути... (настройки программы)

  // Управление потоками
  DWORD IDKernelThread;                 // id
  HANDLE hKernelThread;                 // дескриптор процесса (ядро выполнения команд)
  bool TerminateKernelProc/* = false*/; // норм завершение потока

  DWORD KernelTime;                     // время выполнения команды
  OVERLAPPED OverSend;                  // асинхронная запись в порт

  // статистика:
  bool CMDOK;                // успешное выполнение команды
  DWORD LastPing;            // вемя последнего обращения к ядру
  DWORD CurrentPing;
  bool Engine();             // выполнение ядра (работает/нет)
  bool StartKernel;          // запуск ядра (запущено/нет)

  DWORD CountCMDOK;          // количество успешно выполненных команд
  DWORD CountCMDERR;         // количество ошибок выполнения команд

  DWORD ErrTimeOut;          // ошибки таймаутов
  DWORD ErrCritical;         // ошибки критические
  DWORD ErrCRC;              // ошибки контрольной суммы
  DWORD ErrSend;             // ошибки отправки данных
  DWORD ErrRecv;             // ошибки получения данных

 protected:
  DWORD KernelProc();        // ядро выполнения команд
//void ReloadKernel();       // перезапуск ядра при возникновении критической ошибки

  // статистика:
  void IteratorCMDOK() {CMDOK = true;  CountCMDOK  = CountCMDOK  %0xFFFFFFFF + 1; }; // счетчик успешно выполненных команд
  void IteratorCMDERR(){CMDOK = false; CountCMDERR = CountCMDERR %0xFFFFFFFF + 1; }; // счетчик команд выполненных с ошибками

 public:
  CmdSystem_AT32();  // конструктор
  ~CmdSystem_AT32(); // деструктор

  void Run();      // запустить ядро
  void Stop();     // остановить

  /*функции потоков*/
  static DWORD WINAPI KernelThreadProc(LPVOID lParam){return ((CmdSystem_AT32*)lParam)->KernelProc(); } // функция потока ядра

  void StartThreads(); // создать и запустить потоки
  void StopThreads();  // завершить потоки нормально
};

/*
void CmdSystem_FK::ReloadKernel()
{
 StopThreads();  // завершить потоки нормально
 StartThreads(); // создать и запустить потоки
}
*/

bool CmdSystem_AT32::Engine()
{
 if(LastPing == CurrentPing){return false;}
 else{LastPing = CurrentPing; return true;};
}

CmdSystem_AT32::CmdSystem_AT32()
{
 ComPort = NULL;          // указатель на ком порт класс

 LastPing = 0;
 CurrentPing = 0;

 CountCMDOK = 0;          // количество успешно выполненных команд
 CountCMDERR = 0;         // количество ошибок выполнения команд
 ErrTimeOut = 0;          // ошибки таймаутов
 ErrCritical = 0;         // ошибки критические
 ErrCRC = 0;              // ошибки контрольной суммы
 ErrSend = 0;             // ошибки отправки данных
 ErrRecv = 0;             // ошибки получения данных

 KernelTime = 999;        // время выполнения команды

 StartKernel = false;     // ядро не запущено

 UserCmdBuf.SetLimit(5);  // предельный размер буфера
 SystemCmdBuf.SetLimit(5);
 CurrentCmdBuf.SetLimit(5);
}

CmdSystem_AT32::~CmdSystem_AT32()
{
 UserCmdBuf.Free();                // очистка буферов команд
 SystemCmdBuf.Free();
 CurrentCmdBuf.Free();
}

void CmdSystem_AT32::StartThreads() // создать и запустить потоки
{
  if(StartKernel){return;};       // если уже запущено
  StartKernel = true;             // ядро запущено

  TerminateKernelProc = false;    // сигнал завершения потока ядра
  hKernelThread = CreateThread(0, 0, &KernelThreadProc, this, 0, &IDKernelThread); // создать и запустить поток (ядро)
}

void CmdSystem_AT32::StopThreads()  // остановка потоков ядра
{
  if(!StartKernel){return;};      // если уже остановлено
  StartKernel = false;            // ядро не запущено

  // завершение ядра...
  DWORD TerminateKernelResult;
  TerminateKernelProc = true;     // сигнал на завершение потока ядра

  TerminateKernelResult = WaitForSingleObject(hKernelThread,1000); // ждать завершения потока ядра
  if(TerminateKernelResult != WAIT_OBJECT_0)                       // не удалось нормально завершить потоки
  {
   //MessageBoxA(NULL,"Принудительное завершение потока ядра","не удалось нормально завершить поток",0); // TEST MESSAGE
   SaveEvent("Не удалось нормально завершить поток, Принудительное завершение потока ядра"); // сохранить ошибку

   TerminateThread(hKernelThread, 0);       // принудительное завершение
  };

  CloseHandle(hKernelThread);
}

void CmdSystem_AT32::Run()   // запустить ядро
{
  CountCMDOK = 0;          // количество успешно выполненных команд
  CountCMDERR = 0;         // количество ошибок выполнения команд
  ErrTimeOut = 0;          // ошибки таймаутов
  ErrCritical = 0;         // ошибки критические
  ErrCRC = 0;              // ошибки контрольной суммы
  ErrSend = 0;             // ошибки отправки данных
  ErrRecv = 0;             // ошибки получения данных
  StartThreads();          // запустить потоки
}

void CmdSystem_AT32::Stop() // остановить ядро
{
  StopThreads();           // остановить потоки
}

DWORD CmdSystem_AT32::KernelProc() // Синхронное выполнение команд (отправка и получение)
{
  DWORD signal;                  // результат ожидания
  TerminateKernelProc = false;   // завершение потока (не установлено)
  DWORD BeginTime;               // расчет времени обработки команды

  byte NumCmd;                   // номер входящей команды

  DWORD CurrentDevAddr;          // адрес устройства, опрашиваемого в текущим момент, CurrentCmd->DevAddr сохранение
  DevTYPE CurrentDeviceType;     // тип устройства, опрашиваемого в текущим момент, CurrentCmd->DeviceType сохранение

  OverSend.hEvent = CreateEvent(NULL, false, false, NULL); // ожидание завершения чтения из порта (по умолчанию не установлено)

  while(!TerminateKernelProc)    // ядро выполнения команд
  {
   // ErrSend++;  // TEST
   // Sleep();

   BeginTime = GetTickCount();
   CurrentPing = BeginTime;    // признак работы

   // выборка команды из буфера...              // приоритет высший -> UserCmdBuf - SystemCmdBuf - CurrentCmdBuf -> низший
   if(UserCmdBuf.GetCount() != 0)               // проверить буфер команд пользователя
   {
    CurrentCmd = UserCmdBuf.GetL();             // выбрать команду из буфера пользователя
   }
   else if(SystemCmdBuf.GetCount() != 0)        // проверить системный буфер команд
   {
    CurrentCmd = SystemCmdBuf.GetL();           // выбрать команду из системного буфера
   }
   else if(CurrentCmdBuf.GetCount() != 0)
   {
    CurrentCmd = CurrentCmdBuf.GetL();          // выбрать команду из буфера
   }
   else // холостой ход
   {
    CurrentCmd = NULL;
    Sleep(1); // Yield(); // 1 квант ~16 ms.
    //KernelTime = GetTickCount() - BeginTime;  // не показывать время холостого хода
    continue; // while(1) снова
   }

   ResetEvent(OverSend.hEvent);
   ComPort->Clean();            // очистить буферы порта перед отправкой сообщения
   //Sleep(65);                 // 70-80мс синхронизация, проверить timeout в настройках устройства

   try // отправка команды устройству на выполнение...
   {
    ComPort->SendAsync(CurrentCmd->SendPacket, CurrentCmd->SendPacketSize, &OverSend); // отсылка пакета в устройство
    //ComPort->SendDataAsync(CurrentCmd->SendPacket, CurrentCmd->SendPacketSize);
   }
   catch(...)
   {
    if(CurrentCmd != NULL){CurrentCmd->ExecError();}    // CurrentCmd->ExecComError(); разблокировка выполнения команды
    //SaveDeviceError("ошибка отправки команды в COM порт", CurrentDevAddr, CurrentDeviceType); //SaveEvent("ошибка отправки команды в COM порт"); // регистрация ошибки в журнале
    IteratorCMDERR(); ErrSend++;                        // ошибка выполнения команды
    Sleep(1);                                           // Sleep(2000); // аварийный таймаут
    KernelTime = GetTickCount() - BeginTime;            // время итерации ядра
    continue;                                           // while(1) снова
   }

   if(CurrentCmd->RecvPacketSize == 0)                  // нет возвращаемых данных
   {
    CurrentCmd->ExecSuccess();                          // результат команды ОК
    IteratorCMDOK();                                    // считать что команда выполнена успешно
    Sleep(1);                                           // Sleep(2000); // аварийный таймаут
    KernelTime = GetTickCount() - BeginTime;            // время итерации ядра
    continue;                                           // while(1) снова
   }

   try // получение ответа от устройства
   { // if(CurrentCmd != NULL){ Form1->ComPort1->Read( CurrentCmd->RecvPacket, CurrentCmd->RecvPacketSize); }
    ComPort->RecvSync(CurrentCmd->RecvPacket,CurrentCmd->RecvPacketSize, CurrentCmd->Delay, signal); // попытка получить пакет из порта
   }
   catch(...) // COM-порт недоступен
   {// MessageBoxA(NULL,"Ошибка получения данных.\n Проверьте настройки порта.","Ошибка при работе с COM-портом",0);
    if(CurrentCmd != NULL){CurrentCmd->ExecError();}                            // CurrentCmd->ExecComError(); разблокировка выполнения команды
    //ComPort->Clean();                                                         // очистить буфер com порта, глючит на скорости 1200 bod!
    //SaveDeviceError("ошибка получения команды из COM порта", CurrentDevAddr, CurrentDeviceType); //SaveEvent("ошибка получения команды из COM порта"); // регистрация ошибки в журнале
    IteratorCMDERR(); ErrRecv++;                                                // ошибка выполнения команды
    Sleep(1);                                                                   // Sleep(2000); // аварийный таймаут
    KernelTime = GetTickCount() - BeginTime;                                    // время итерации ядра
    continue;                                                                   // (while снова)
   }

   if(signal == WAIT_OBJECT_0) // команда выполнена, получен ответ
   {
    //MessageBoxA(NULL,"ОК","команда выполнена",0);                            // TEST MSG
    if(CurrentCmd->CRC() != true)                                               // ошибка в контрольной сумме во входящем пакете
    {
     if(CurrentCmd != NULL){CurrentCmd->ExecCRCError();}                        // результат команды crCRCError
     ComPort->Clean();                                                          // очистить буфер com порта глючит на скорости 1200 bod!
     //KernelTime = GetTickCount() - BeginTime;
     //SaveDeviceError("ошибка контрольной суммы", CurrentDevAddr, CurrentDeviceType);
     IteratorCMDERR(); ErrCRC++;                                                // ошибка выполнения команды
     Sleep(1);
     KernelTime = GetTickCount() - BeginTime;                                   // время итерации ядра
     continue; // (while снова)
    }

    CurrentCmd->ExecSuccess();                                                  // результат команды ОК
    IteratorCMDOK();                                                            // счетчик удачно выполненных команд
   }
   else if(signal == WAIT_TIMEOUT)                                              // превышено время ожидания
   {
    //MessageBoxA(NULL,"нет ответа от устройства","команда выбрана из буфера",0); // TEST MSG
    if(CurrentCmd != NULL){CurrentCmd->ExecTimeOut();}                          // нет ответа от устройства (обработка ошибок)
    //SaveDeviceError("нет связи", CurrentDevAddr, CurrentDeviceType);
    ComPort->Clean();                                                           // очистить буфер com порта глючит на скорости 1200 bod!
    IteratorCMDERR(); ErrTimeOut++;                                             // ошибка выполнения команды
   }
   else // Критическая ошибка событие не инициализированно
   {
    //MessageBoxA(NULL,"критическая ошибка","команда не выполнена",0); // TEST MSG
    if(CurrentCmd != NULL){CurrentCmd->ExecTimeOut();}
    //SaveDeviceError("*критическая ошибка", CurrentDevAddr, CurrentDeviceType);
    ComPort->Clean();                                                           // очистить буфер com порта глючит на скорости 1200 bod!
    Sleep(1500);                                                                // время на исправление ошибки
    IteratorCMDERR(); ErrCritical++;                                            // ошибка выполнения команды
   }

   Sleep(1); // Yield();                                                        // передача управления другим потокам
   KernelTime = GetTickCount() - BeginTime;                                     // время итерации ядра
  } // while(1)

  CloseHandle(OverSend.hEvent); // закрыть объект-событие (ожидание приема ответа)
  //hKernelThread = NULL;       // норм. завершение потока только после CloseHandle(hKernelThread);
  return 0;
}

#endif
