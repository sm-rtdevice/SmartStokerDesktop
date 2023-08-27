//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit1.h"
#include "Unit2.h"
#include "Unit3.h"

#include "include\CmdSystem.h"
#include "include\Stoker.h"
#include "include\DS-1820.h"

#include "include\CRC8.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

// глобальные переменные программы:
CSettings ProgramSettings;              // настройки программы
CComPort* ComPortMK;                    // ком порт для подключения контроллера
CmdSystem_AT32* KernelStoker;           // ядро выполнения команд смарт кочегара

cDS1820 DS1_Radiator;                   // датчик DS1820 на радиаторе
cDS1820 DS2_Caldron;                    // датчик DS1820 на котле

bool IconActive = false;                // индикаторы состояний приема\передачи команд
bool ComOK;

WORD GetTempTime = 5;                   // периодичность опроса температуры (с.) (настройки программы) todo: сделать сохранение в файл

// данные из EEPROM контроллера:
unsigned char MinTRadiator;             // температура включения насоса  45 (радиатор)
unsigned char MaxTRadiator;             // температура отключения насоса 55 (радиатор)
unsigned char MinTCaldron;              // минимальная температура воды в котле при котором возможно включение насоса
unsigned char PulseMode;                // импульсный режим для управления без использования датчиков DS1820; 0 - DS1820; 1 - импульсный; по умолчанию - 0,  (bool 0-1) bit!;
unsigned char PumpMode;                 // управление насосом: 0 - авто; 1 - принудительное включение; 2 - принудительное отключение
unsigned char WorkTime;                 // время работы 5-10 мин
unsigned char IdleTime;                 // время простоя 20-30 мин
unsigned char nStart;                   // счетчик перезапусков контроллера

unsigned char Radiator_SN[8];           // SN
unsigned char Caldron_SN[8];            // SN

// данные из RAM (текущие данные) контроллера:
char TempRadiator = 0;                  // текущее значение температуры радиатора 0 - начальное значение/ошибка
char TempCaldron  = 0;                  // текущее значение температуры котла     0 - начальное значение/ошибка

// Поток таймера:
HANDLE hTimerThread;
DWORD IDTimerThread;
bool exitThrd = false;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner):TForm(Owner){}
//---------------------------------------------------------------------------

/*
void __fastcall TForm1::Response(TWMDropFiles& Message) // перетаскивание
{
 HDROP hdropHandle = (HDROP)Message.Drop;
 char chName[MAX_PATH];  // Переменная для хранения имени файла

 // int viNumber = DragQueryFile(hdropHandle,-1,NULL,NULL);  // Число буксируемых файлов
 DragQueryFile(hdropHandle,0,chName,MAX_PATH);

 String strEx = ExtractFileExt(chName);                     // только .мес
 if(strEx != ".sit"){return;};

 //float OutI, OutU;
 //LoadMeasureData(chName,OutI,OutU,ArrCount,dataArr); // загрузить данные из файла

 Label4->Caption = chName;
}
//---------------------------------------------------------------------------
*/

void SaveDeviceError(char* msg, DWORD DevAddr, DevTYPE DeviceType)  // сохранение событий в лог файл
{
 if(ProgramSettings.SaveLog == true) {
 }
}
//---------------------------------------------------------------------------

void SaveEvent(char* msg)  // сохранение событий в лог файл
{

 if(ProgramSettings.SaveLog == true) // сохр. лог?
 {
/*  // if(mutex){return;}                                                 // идет запись
  // set mutex                                                            // занять

  __try
  {
   SaveLog((ProgramSettings.ExePath  + "Event.log").c_str(), msg);        // записать  try SaveLog finaly освободить
  }
  __finally
  {
   // reset mutex                                                         // освободить
  }

  // reset mutex                                                          // освободить
*/
 } // \сохр. лог?

}
//---------------------------------------------------------------------------

String GetTempDegreeStr(int Temp)
{
  String StrDegree = IntToStr(Temp);
  StrDegree += "\u00B0";
  StrDegree += "C";
  return StrDegree;
}

void KeyPressFilter(TObject *Sender, char &Key)  // фильтр цифровых кнопок
{

 if((Key >= '0') && (Key <= '9'))
 {
  if(  ((TEdit*)Sender)->SelText.Length() != 0 ){((TEdit*)Sender)->Clear();} // если текст выделен очистить поле и начать ввод заново

  if(((TEdit*)Sender)->Text.Pos(DecimalSeparator) != 0) // есть разделитель
  {
   if( ((TEdit*)Sender)->Text.Length() - ((TEdit*)Sender)->Text.Pos(DecimalSeparator) >= 1 ) { Key = 0; }  // число знаков после разделителя: 1
  }                                                     // \есть разделитель
 /*return Key;*/
 }
 else if(Key == VK_BACK){/*return Key;*/}
 else {Key = 0;}
}
//---------------------------------------------------------------------------

void ShowCurrentTemperature() // построение графика текущей температуры
{
 int i=0;
 TListData2<CTermPoint> *CurrentItem;

// DS1_Radiator;                   // датчик DS1820 на радиаторе
// DS2_Caldron;

// if(DS1_Radiator.TPoints.GetCount() == 0){return;}
// if(DS2_Caldron.TPoints.GetCount() == 0){return;}

 Form3->Chart1->Series[0]->Clear();

 CurrentItem = DS1_Radiator.TPoints.First();
 while(CurrentItem != NULL)     // отрисовка графика температуры датчик 1
 {
  //if(DS1MaxLog < CurrentItem->Data.GetTemp()){DS1MaxLog = CurrentItem->Data.GetTemp();};                                        // Максимальное значение температуры DS1
  //if(DS1MinLog > CurrentItem->Data.GetTemp()){if(CurrentItem->Data.GetTemp()!= -90){DS1MinLog = CurrentItem->Data.GetTemp();}; }// Минимальное значение температуры DS1
  i++;
  Form3->Chart1->Series[0]->AddXY(i, CurrentItem->Data.Temperature, CurrentItem->Data.Time.TimeString(), clRed);  // Rows->Data; данные(значения) параметра //*FloatToStrF(SrInt[i],ffFixed,6,2)
  CurrentItem = CurrentItem->Next;
 }

 i=0;
 Form3->Chart1->Series[1]->Clear();

 CurrentItem = DS2_Caldron.TPoints.First();
 while(CurrentItem != NULL)     // отрисовка графика температуры датчик 1
 {
  //if(DS1MaxLog < CurrentItem->Data.GetTemp()){DS1MaxLog = CurrentItem->Data.GetTemp();};                                        // Максимальное значение температуры DS1
  //if(DS1MinLog > CurrentItem->Data.GetTemp()){if(CurrentItem->Data.GetTemp()!= -90){DS1MinLog = CurrentItem->Data.GetTemp();}; }// Минимальное значение температуры DS1
  i++;
  Form3->Chart1->Series[1]->AddXY(i, CurrentItem->Data.Temperature + 0.1, CurrentItem->Data.Time.TimeString(), clBlue);  // Rows->Data; данные(значения) параметра //*FloatToStrF(SrInt[i],ffFixed,6,2)
  CurrentItem = CurrentItem->Next;
 }

 AnsiString FormCap;
 FormCap = "Просмотр оперативных данных: ";

 float min, Min;
 float max, Max;

 DS1_Radiator.GetMinMaxT(min,max);
 FormCap += "   (Датчик 1 Min  = ";
 if(min == 125.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(min ,ffFixed,3,1); };
 FormCap += "; Max = ";
 if(max == -90.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(max ,ffFixed,3,1); };

 Min = min;
 Max = max;

 DS2_Caldron.GetMinMaxT(min,max);
 FormCap += " | Датчик 2 Min  = ";
 if(min == 125.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(min ,ffFixed,3,1); };
 FormCap += "; Max = ";
 if(max == -90.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(max ,ffFixed,3,1); };
 FormCap += ")";

 Form3->Caption = FormCap;

 if(ProgramSettings.AutoRange == true)  // автонастройка графика
 {
  if(Min > min){Min = min;};
  if(Max < max){Max = max;};
  Form3->Chart1->LeftAxis->SetMinMax(Min-0.5, Max+0.5);
 }

}
//---------------------------------------------------------------------------

DWORD WINAPI TimerProc(LPVOID lParam) // процедура потока таймера
{
 CMDGetTempers GetTempers;
 CMDRES res;
 GetTempers.SetParam(1500);                              // 0x01 - установка настроек

 while(!exitThrd) // вечный цикл
 {
  if(ComOK)
  {
   // запрос температуры с МК
   res = GetTempers.ExecCmd(&KernelStoker->CurrentCmdBuf);   // if( res!= crSuccess){MessageDlg("Не удалось получить температуру", mtWarning, TMsgDlgButtons() << mbOK, 0);};
   GetTempers.GetTempers(TempRadiator, TempCaldron);

   if(res == crSuccess) // ОК
   {
    DS1_Radiator.SetTPoint(TempRadiator);                   // датчик DS1820 на радиаторе
    DS2_Caldron.SetTPoint(TempCaldron);                     // на котле
    SaveTermPoint(DS1_Radiator.GetTPoint(), DS2_Caldron.GetTPoint());
   }
   else // ошибка получения
   {
    DS1_Radiator.SetTPoint(-100);                            // ошибк получения данных
    DS2_Caldron.SetTPoint(-100);                             //
    SaveTermPoint(DS1_Radiator.GetTPoint(), DS2_Caldron.GetTPoint());
   }

   if( (Form3 != NULL) && (Form3->Visible == true) && (Form3->Tag == 0) ) {ShowCurrentTemperature();} // обновление отображения текущей температуры
  }

  Sleep(GetTempTime*1000);       // период таймера 1000-60000
  // \процедура таймера
 } // while

 CloseHandle(hTimerThread);
 return 0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)    // периодический опрос датчиков
{
 Form1->StatusBar1->Panels->Items[1]->Text = KernelStoker->KernelTime;        // время выполнения команды

 // индикатор состояния работы...
 if(KernelStoker->Engine()) // ядро запущено, работает
 {
  if(KernelStoker->CMDOK)  // команда выполнена сост ОК
  {
   IconActive = !IconActive; // мигать
   ImageList1->Draw(Form1->StatusBar1->Canvas,120,2,0,IconActive); // сост ОК индикатор мигает зеленым
  }
  else  // ошибка выполнения команды, нет связи, таймаут...
  {
   ImageList1->Draw(Form1->StatusBar1->Canvas,120,2,1,true); // индикатор горит красным
  }
 }
 else // ядро не работает
 {
  ImageList1->Draw(Form1->StatusBar1->Canvas,120,2,1,false); // индикатор отключен
 }

 Form1->StatusBar1->Panels->Items[3]->Text = IntToStr(KernelStoker->CountCMDOK);
 Form1->StatusBar1->Panels->Items[4]->Text = IntToStr(KernelStoker->CountCMDERR);

 AnsiString str;
 //str = FloatToStrF(DS1_Radiator.GetTemperature(), ffFixed,3,1) + "'C"; //
 str = GetTempDegreeStr(DS1_Radiator.GetTemperature());
 if(DS1_Radiator.GetTemperature() == -100){str = "N/A 'C";};
 StaticText1->Caption = str;

 //str = FloatToStrF(DS2_Caldron.GetTemperature(), ffFixed,3,1) + "'C";
 str = GetTempDegreeStr(DS2_Caldron.GetTemperature());
 if(DS1_Radiator.GetTemperature() == -100){str = "N/A 'C";};
 StaticText2->Caption = str;
}
//---------------------------------------------------------------------------
void GetStokerSettings()
{
 // получение текущих настроек...
 CMDGetSettings GetSettings;
 CMDRES res;

 GetSettings.SetParam(2000); // 0x03 - установка настроек
 res = GetSettings.ExecCmd(&KernelStoker->UserCmdBuf);
 if( res != crSuccess){MessageDlg("Не удалось получить настройки МК", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};

 GetSettings.GetSettings(MinTRadiator,MaxTRadiator,MinTCaldron,WorkTime,IdleTime,PulseMode,PumpMode,nStart);

 if(MinTRadiator == 0xFF || MaxTRadiator == 0xFF ||  MinTCaldron == 0xFF ||  WorkTime == 0xFF ||  IdleTime == 0xFF ){MessageDlg("EEPROM контроллера не инициализирована\nПроизведите настройку уставок", mtWarning, TMsgDlgButtons() << mbOK, 0);};

 Form1->Edit1->Text = IntToStr(MinTRadiator);             // температура включения насоса  45 (радиатор)
 Form1->Edit2->Text = IntToStr(MaxTRadiator);             // температура отключения насоса 55 (радиатор)
 Form1->Edit3->Text = IntToStr(MinTCaldron);              // минимальная температура воды в котле при котором возможно включение насоса
 Form1->Edit4->Text = IntToStr(WorkTime);                 // время работы 5-10 мин
 Form1->Edit5->Text = IntToStr(IdleTime);                 // время простоя

 switch(PulseMode)                                 // 0 - DS1820; 1 - импульсный;
 {
  case 0: Form1->RadioGroup1->ItemIndex = 0;
  break;
  case 1: Form1->RadioGroup1->ItemIndex = 1;
  break;
  case 2: Form1->RadioGroup1->ItemIndex = 2;
  break;
  case 3: Form1->RadioGroup1->ItemIndex = 3;
  break;
 }

 Form1->StatusBar1->Panels->Items[5]->Text = StrToInt(nStart);          // счетчик перезапусков

 // получение SN датчиков ...
 CMDGetSN GetSN;

 GetSN.SetParam(2000);
 res = GetSN.ExecCmd(&KernelStoker->UserCmdBuf);
 if( res != crSuccess){MessageDlg("Не удалось получить SN датчиков", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};

 GetSN.GetSN(Radiator_SN, Caldron_SN);

 AnsiString StrSN;
 ParceSNtoStr(StrSN,Radiator_SN); Form1->Edit6->Text = StrSN;    // парсинг sn
 ParceSNtoStr(StrSN,Caldron_SN);  Form1->Edit7->Text = StrSN;
}

void __fastcall TForm1::FormCreate(TObject *Sender)     // подключение к устройству получение актуальных данных
{
 ComPortMK = new CComPort();

 if( ProgramSettings.LoadSettings() == false ) // Загрузка настроек программы
 { // only true!!!!!!!!!
  ProgramSettings.ComPortSpeed = 9600;                     // по умолчанию
  ProgramSettings.ComPort = 10;
  MessageDlg("Не удалось загрузить настройки программы", mtWarning, TMsgDlgButtons() << mbOK, 0);
 }

 KernelStoker = new CmdSystem_AT32;
 KernelStoker->ComPort = ComPortMK;
 KernelStoker->Settings = &ProgramSettings;                // указатель на параметры

 DS1_Radiator.Description = ProgramSettings.DS1Name;       // датчик DS1820 на радиаторе
 DS2_Caldron.Description = ProgramSettings.DS2Name;

 //DragAcceptFiles(Handle,true);  // включить буксировку файлов

 // попытка открыть порт контроллера...
 try
 {
  if(!ComPortMK->Open(ProgramSettings.ComPort,ProgramSettings.ComPortSpeed) )  // 2, 9600
  {
   ComOK = false;
   Form1->StatusBar1->Panels->Items[0]->Text = "COM ERR";
   MessageDlg("Невозможно открыть COM-порт\nПроверьте настройки COM-порта", mtWarning, TMsgDlgButtons() << mbOK, 0); return;
  }
  else
  {
   // OK
   ComOK = true;
   Form1->StatusBar1->Panels->Items[0]->Text = "COM " + IntToStr(ProgramSettings.ComPort);
  }
 }
 catch(...) // невозможно открыть COM-порт
 {
  // отключить опрос контроллера
  ComOK = false;
  Form1->StatusBar1->Panels->Items[0]->Text = "COM ERR";
  MessageDlg("Ошибка открытия порта\nПроверьте настройки COM-порта", mtWarning, TMsgDlgButtons() << mbOK, 0); return;
 }

 // запуск ядра и потоков...
 KernelStoker->Run();                              // запустить ядро
 hTimerThread = CreateThread(0, 0, &TimerProc, 0, 0 , &IDTimerThread); // Запуск обновления данных в отдельном потоке (чтобы таймер работал)
 Timer1->Enabled = true;

 GetStokerSettings();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormDestroy(TObject *Sender)    // отключение, закрытие формы
{
 exitThrd = true;                   // завершрить поток таймера

 ProgramSettings.SaveSettings();
 ComPortMK->Close();
 delete(ComPortMK);

 KernelStoker->Stop(); // завершить ядро
 delete(KernelStoker);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender)   // настройка уставок
{
 //PulseMode = 2;
 if(RadioGroup1->ItemIndex == -1){ MessageDlg("не выбран режим управления", mtWarning, TMsgDlgButtons() << mbOK, 0); return;}

 switch (RadioGroup1->ItemIndex)
 { // импульсный режим для управления без использования датчиков DS1820; 0 - DS1820; 1 - импульсный; по умолчанию - 0,  (bool 0-1) bit!;
  case 0: PulseMode = 0;
  break;
  case 1: PulseMode = 1;
  break;
  case 2: PulseMode = 2;
  break;
  case 3: PulseMode = 3;
  break;
 }

 try
 {
  MinTRadiator = StrToInt(Edit1->Text);             // температура включения насоса  45 (радиатор)
  MaxTRadiator = StrToInt(Edit2->Text);             // температура отключения насоса 55 (радиатор)
  MinTCaldron = StrToInt(Edit3->Text);              // минимальная температура воды в котле при котором возможно включение насоса
  WorkTime = StrToInt(Edit4->Text);                 // время работы 5-10 мин
  IdleTime = StrToInt(Edit5->Text);                 // время простоя 20-30 мин
 }
 catch(...) // ловить Все ошибки
 {
  MessageDlg("Введен неправильный тип данных", mtWarning, TMsgDlgButtons() << mbOK, 0);
  return;
 }

 // todo: проверка на диапазоны значений и != 255

 if(MinTRadiator >= MaxTRadiator){ MessageDlg("температура включения насоса <= температура отключения насоса", mtWarning, TMsgDlgButtons() << mbOK, 0); return; }

 // отправка уставок на МК
Button1->Enabled = false;              // заблочить кнопку
 CMDSetSettings SetSettings;
 CMDRES res;

 SetSettings.SetParam(MinTRadiator,MaxTRadiator,MinTCaldron,WorkTime,IdleTime,PulseMode,2000); //0x03 - установка настроек
 res = SetSettings.ExecCmd(&KernelStoker->UserCmdBuf);
Button1->Enabled = true;              // разблокировать кнопку
 if( res!= crSuccess){MessageDlg("Не удалось установить настройки", mtWarning, TMsgDlgButtons() << mbOK, 0);};
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1KeyPress(TObject *Sender, char &Key)
{
 KeyPressFilter(Sender, Key);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit2KeyPress(TObject *Sender, char &Key)
{
 KeyPressFilter(Sender, Key);        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit3KeyPress(TObject *Sender, char &Key)
{
 KeyPressFilter(Sender, Key);        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit4KeyPress(TObject *Sender, char &Key)
{
 KeyPressFilter(Sender, Key);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit5KeyPress(TObject *Sender, char &Key)
{
 KeyPressFilter(Sender, Key);
}
//---------------------------------------------------------------------------

void DialogSetSN()
{
 if(!Form1->OpenDialog1->Execute()){ return; }

 Form2->ListBox1->Items->Clear(); // очистить список

 //sDS1820 device;
 AnsiString StrSN;

 CTwoLinkList <sDS1820> DevList;  // DS1820
 TListData2<sDS1820> *CurrentItem;

 LoadSNf(Form1->OpenDialog1->FileName, DevList);

 CurrentItem = DevList.First();   // установить указатель на первый элемент
 while(CurrentItem != NULL)       // обход списка
 {
     StrSN = "";
     StrSN += IntToHex(CurrentItem->Data.SN[0] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[1] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[2] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[3] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[4] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[5] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[6] ,2); StrSN += " ";
     StrSN += IntToHex(CurrentItem->Data.SN[7] ,2);
     Form2->ListBox1->Items->Add(StrSN);

  CurrentItem = CurrentItem->Next;
 }

 if(DevList.GetCount() > 0){Form2->Show();};

 DevList.Free();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender) // выбор sn радиатора
{
 Form2->Tag = 0; // sn 1 (радиатор)
 DialogSetSN();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender) // выбор sn котла
{
 Form2->Tag = 1; // sn 2 (котел)
 DialogSetSN();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender) // отправка sn на МК
{
Button4->Enabled = false;              // заблочить кнопку

 CMDSetSN SetSN;
 CMDRES res;

 unsigned char sn1[8]/* = {0x10, 0xBA, 0x12, 0x1E, 0x01, 0x08, 0x00, 0x3C}*/;  // радиатор, массив для хранения SN - уникальных кодов датчиков, 8 байт на датчик  28AB5F7
 unsigned char sn2[8]/* = {0x10, 0x05, 0x39, 0xDA, 0x00, 0x08, 0x00, 0x0B}*/;  // TEST

 ParceSNtoByte(Edit6->Text, sn1);  // string --> byte[]
 ParceSNtoByte(Edit7->Text, sn2);  //

 SetSN.SetParam(sn1,sn2,2000);

 res = SetSN.ExecCmd(&KernelStoker->UserCmdBuf);
Button4->Enabled = true;               // разблокировать кнопку
 if( res != crSuccess){MessageDlg("Не удалось установить SN датчиков", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender) // ручное управление насосом
{
/*
 if(RadioGroup2->ItemIndex == -1){MessageDlg("Не выбран режим управления насосом", mtWarning, TMsgDlgButtons() << mbOK, 0); return;}

Button5->Enabled = false;              // заблочить кнопку
 CMDSetPumpMode SetPumpMode;
 CMDRES res;

 SetPumpMode.SetParam(RadioGroup2->ItemIndex,1500);

 res = SetPumpMode.ExecCmd(&KernelStoker->CurrentCmdBuf);
Button5->Enabled = true;              // разблокировать кнопку
 if( res != crSuccess){MessageDlg("Не удалось установить режим управления насосом", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};
 */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender)
{
Button7->Enabled = false;              // заблочить кнопку
 CMDRES res;
 CMDGetTempers GetTempers;
 GetTempers.SetParam(2000);
 res = GetTempers.ExecCmd(&KernelStoker->UserCmdBuf);
 GetTempers.GetTempers(TempRadiator, TempCaldron);

 switch(res)
 {
   case crSuccess:
   //ShowMessage("crSuccess");
   ShowMessage("TempRadiator");
   ShowMessage("TempCaldron");
   break;

   case crTimeOut: ShowMessage("crTimeOut");
   break;

   case crError: ShowMessage("crError");
   break;

   case crCRCError: ShowMessage("crCRCError");
   break;

   case crInfiniteTimeOut: ShowMessage("crInfiniteTimeOut");
   break;

   case crBufError: ShowMessage("crBufError");
   break;
 }

Button7->Enabled = true;              // разблокировать кнопку
 /*
 CMDGetSettings GetSettings;
 GetSettings.SetParam();
 res = GetSettings.ExecCmd(&KernelStoker->UserCmdBuf);

 if( res != crSuccess){MessageDlg("Не удалось получить настройки МК", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};
 GetSettings.GetSettings(MinTRadiator,MaxTRadiator,MinTCaldron,WorkTime,IdleTime,PulseMode,PumpMode,nStart);
 */

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
 ShowCurrentTemperature();
 Form3->Tag = 0;           // режим оперативного просмотра

 try
 {
  int ScrollPosMax;                                                                          // настройка скроллбара
  if(Form3->Series1->MaxXValue() < 10){ScrollPosMax = 10;}
  else if(Form3->Series1->MaxXValue() > 50){ScrollPosMax = Form3->Series1->MaxXValue() - 50;}
  else{ScrollPosMax = Form3->Series1->MaxXValue();};
  Form3->ScrollBar1->Max = ScrollPosMax;
  Form3->ScrollBar1->Position = 0;
  Form3->Chart1->BottomAxis->Minimum = 0;
  Form3->Chart1->BottomAxis->Maximum = 60;
 }
 catch(...)
 {
  Form3->ScrollBar1->Max = 60;
  Form3->ScrollBar1->Position =0;
 }

 Form3->Show();
}
//---------------------------------------------------------------------------
void StatusBarShowHint(int X)  // статус бар всплывающая подсказка/статистика
{
  AnsiString Hint;
 //int day,hour,min;

 if(X<84){Form1->StatusBar1->Hint = "настройки COM-порта";return;}

 if(X<119)
 {
  Hint = "время выполнения команды";
  //ProcessTime( IntervalMinutes(StartDataTime.CurrentDateTime(), StartDataTime), day,hour,min);
  //Hint += "\nвремя работы дней: "; Hint += IntToStr(day);
  //Hint += "; Часов: "; Hint += IntToStr(hour);
  //Hint += "; Минут: "; Hint += IntToStr(min);
  Form1->StatusBar1->Hint = Hint; return;
 }

 if(X<140)
 {
  Hint = "индикатор активности:";
  Hint += "\nтаймаутов: "; Hint += IntToStr(KernelStoker->ErrTimeOut);
  Hint += "\nCRC: "; Hint += IntToStr(KernelStoker->ErrCRC);
  Hint += "\nкритические: "; Hint += IntToStr(KernelStoker->ErrCritical);
  Hint += "\nотправка(COM): "; Hint += IntToStr(KernelStoker->ErrSend);
  Hint += "\nполучение(COM): "; Hint += IntToStr(KernelStoker->ErrRecv);
  Form1->StatusBar1->Hint = Hint; return;
 }

 if(X<190){Form1->StatusBar1->Hint = "ОК";return;}
 if(X<240){Form1->StatusBar1->Hint = "Ошибки";return;}
 if(X<290){Form1->StatusBar1->Hint = "счетчик перезапусков контроллера";return;}
 if(X<350){Form1->StatusBar1->Hint = "Версия ПО";return;}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StatusBar1MouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
 StatusBarShowHint(X);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StatusBar1MouseDown(TObject *Sender,TMouseButton Button, TShiftState Shift, int X, int Y)
{
 StatusBarShowHint(X);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button8Click(TObject *Sender) // получение SN датчиков ...
{
Button8->Enabled = false;              // заблочить кнопку
 CMDGetSN GetSN;
 CMDRES res;

 GetSN.SetParam(2000);
 res = GetSN.ExecCmd(&KernelStoker->UserCmdBuf);
Button8->Enabled = true;              // разблокировать кнопку
 if( res != crSuccess){MessageDlg("Не удалось получить SN датчиков", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};

 GetSN.GetSN(Radiator_SN, Caldron_SN);

 AnsiString StrSN;
 ParceSNtoStr(StrSN,Radiator_SN); Edit6->Text = StrSN;    // парсинг sn
 ParceSNtoStr(StrSN,Caldron_SN);  Edit7->Text = StrSN;    //
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button9Click(TObject *Sender)
{
Button9->Enabled = false;              // заблочить кнопку

 CMDTestLink TestLink;
 CMDRES res;

 char TestStr[15] = "-NOT-REQUEST!-";  // TestStr[14] = '\n'

 TestLink.SetParam();
 res = TestLink.ExecCmd(&KernelStoker->UserCmdBuf);
Button9->Enabled = true;               // разблокировать кнопку
 if( res != crSuccess){MessageDlg("Нет связи с МК!", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};

 TestLink.Request(TestStr);
 ShowMessage(TestStr);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button10Click(TObject *Sender)
{
Button10->Enabled = false;              // заблочить кнопку
 CMDResetCounter ResetCounter;
 CMDRES res;

 ResetCounter.SetParam(1000);
 res = ResetCounter.ExecCmd(&KernelStoker->UserCmdBuf);
Button10->Enabled = true;               // разблокировать кнопку
if( res != crSuccess){MessageDlg("Не удалось сбросить счетчик перезапусков", mtWarning, TMsgDlgButtons() << mbOK, 0); return;};
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button12Click(TObject *Sender)
{
 // StaticText1->Caption = GetTempDegreeStr(-25);
 unsigned char BufCRC[5];

 BufCRC[0] = 10;
 BufCRC[1] = 126;
 BufCRC[2] = 1;
 BufCRC[3] = 1;

 BufCRC[4] = crc8(BufCRC, 1);

 //ShowMessage(BufCRC[4]);
/*
 char byf = -1;
 unsigned char byf2 = byf;
 */
// ShowMessage((int)byf2);
ShowMessage(BufCRC[4]);  //  (byte)

if(BufCRC[1] == crc8(BufCRC, 1))
{
 ShowMessage("CRC OK");  //  (byte)
}

/*
 for(int i = 0; i<256; i++)
 {
  BufCRC[0] = i;
  StringGrid1->Cells[0][i] = crc8(BufCRC, 1);
 }
*/

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button13Click(TObject *Sender)
{
 GetStokerSettings();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button14Click(TObject *Sender)
{
 CMDResetCmd ResetCmd;
 ResetCmd.SetParam(); // oxFF - сброс
 ResetCmd.ExecCmd(&KernelStoker->UserCmdBuf);
}
//---------------------------------------------------------------------------
