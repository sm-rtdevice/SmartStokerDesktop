//----------------------Просмотр графика-----------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit3.h"
#include "Unit4.h"

#include "include\DS-1820.h"
#include "include\Settings.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm3 *Form3;

extern cDS1820 DS1_Radiator; // датчик DS1820 на радиаторе
extern cDS1820 DS2_Caldron;  // датчик DS1820 на котле

extern CSettings ProgramSettings;
const int MaxVisableX = 60; // Маленькое значение относительно графика

//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner): TForm(Owner){}
//---------------------------------------------------------------------------

void LoadLogs(AnsiString Path) // загрузка логов, отрисовка графиков
{
 LoadTermPoints(DS1_Radiator, DS2_Caldron, Path);

 int i=0;
 TListData2<CTermPoint> *CurrentItem;

 Form3->Chart1->Series[0]->Clear();

 CurrentItem = DS1_Radiator.TPointsLog.First();
 while(CurrentItem != NULL)     // отрисовка графика температуры датчик 1
 {
  i++;
  Form3->Chart1->Series[0]->AddXY(i, CurrentItem->Data.Temperature, CurrentItem->Data.Time.TimeString(), clRed);  // Rows->Data; данные(значения) параметра //*FloatToStrF(SrInt[i],ffFixed,6,2)
  CurrentItem = CurrentItem->Next;
 }

 i=0;
 Form3->Chart1->Series[1]->Clear();

 CurrentItem = DS2_Caldron.TPointsLog.First();
 while(CurrentItem != NULL)     // отрисовка графика температуры датчик 1
 {
  i++;
  Form3->Chart1->Series[1]->AddXY(i, CurrentItem->Data.Temperature + 0.1, CurrentItem->Data.Time.TimeString(), clBlue);  // Rows->Data; данные(значения) параметра //*FloatToStrF(SrInt[i],ffFixed,6,2)
  CurrentItem = CurrentItem->Next;
 }

 // int ScrollPosMax = Form5->Series1->MaxXValue() - 60;                // максимальная позиция скроллбара
// if(ScrollPosMax < 0){ScrollPosMax = Form5->Series1->MaxXValue();};
// if(ScrollPosMax < 0){ScrollPosMax = 0;};

 /*настройка скроллбара*/
 try
 {
  int ScrollPosMax;
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
  Form3->ScrollBar1->Position = 0;
 }
 /*/настройка скроллбара*/

 AnsiString FormCap;
 FormCap = "Просмотр логов: " + ExtractFileName(Path);

 FormCap += "   (Датчик 1 Min  = ";
 if(DS1_Radiator.TminLog == 125.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(DS1_Radiator.TminLog ,ffFixed,3,1); };
 FormCap += "; Max = ";
 if(DS1_Radiator.TmaxLog == -90.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(DS1_Radiator.TmaxLog ,ffFixed,3,1); };

 FormCap += " | Датчик 2 Min  = ";
 if(DS2_Caldron.TminLog == 125.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(DS2_Caldron.TminLog ,ffFixed,3,1); };
 FormCap += "; Max = ";
 if(DS2_Caldron.TmaxLog == -90.5){FormCap += "N/A";}else{ FormCap += FloatToStrF(DS2_Caldron.TmaxLog ,ffFixed,3,1); };
 FormCap += ")";

 Form3->Caption = FormCap;

 // легенда
 Form3->Chart1->Series[0]->Title = "Датчик 1 (";
 Form3->Chart1->Series[0]->Title += ProgramSettings.DS1Name;
 Form3->Chart1->Series[0]->Title += ") ";
 Form3->Chart1->Series[0]->Title += IntToStr(DS1_Radiator.Quality);
 Form3->Chart1->Series[0]->Title += "%";

 Form3->Chart1->Series[1]->Title = "Датчик 2 (";
 Form3->Chart1->Series[1]->Title += ProgramSettings.DS2Name;
 Form3->Chart1->Series[1]->Title += ") ";
 Form3->Chart1->Series[1]->Title += IntToStr(DS2_Caldron.Quality);
 Form3->Chart1->Series[1]->Title += "%";

 if(ProgramSettings.AutoRange == true)  // автонастройка графика
 {
  float Min = DS1_Radiator.TminLog;
  float Max = DS1_Radiator.TmaxLog;
  if(Min > DS2_Caldron.TminLog){Min = DS2_Caldron.TminLog;};
  if(Max < DS2_Caldron.TmaxLog){Max = DS2_Caldron.TmaxLog;};
  Form3->Chart1->LeftAxis->SetMinMax(Min-0.5, Max+0.5);
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm3::Response(TWMDropFiles& Message) // перетаскивание
{
 HDROP hdropHandle = (HDROP)Message.Drop;
 char chName[MAX_PATH];  // Переменная для хранения имени файла

 // int viNumber = DragQueryFile(hdropHandle,-1,NULL,NULL);  // Число буксируемых файлов
 DragQueryFile(hdropHandle,0,chName,MAX_PATH);

 String strEx = ExtractFileExt(chName);                      // только .мес
 if(strEx != ".sit"){return;};

 Form3->Tag = 1;                                             // режим просмотра лог файлов
 LoadLogs( chName );
}
//---------------------------------------------------------------------------

void __fastcall TForm3::SetAutoRange()
{
 float Min;
 float Max;

 if(Form3->Tag == 0)  // режим on-line
 {
  float min;
  float max;

  DS1_Radiator.GetMinMaxT(min,max);

  Min = min;
  Max = max;

  DS2_Caldron.GetMinMaxT(min,max);

  if(Min > min){Min = min;};
  if(Max < max){Max = max;};
  Form3->Chart1->LeftAxis->SetMinMax(Min-0.5, Max+0.5);
 }
 else                  // Form3->Tag == 1;  // режим логов
 {
  Min = DS1_Radiator.TminLog;
  Max = DS1_Radiator.TmaxLog;
  if(Min > DS2_Caldron.TminLog){Min = DS2_Caldron.TminLog;};
  if(Max < DS2_Caldron.TmaxLog){Max = DS2_Caldron.TmaxLog;};
  Form3->Chart1->LeftAxis->SetMinMax(Min-0.5, Max+0.5);
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button1Click(TObject *Sender) // загрузить лог
{
 if(!OpenDialog1->Execute()){return;};
 Form3->Tag = 1;                      // режим просмотра лог файлов

 LoadLogs( OpenDialog1->FileName );
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Chart1DblClick(TObject *Sender) // настройка графика
{
 Form4->Show();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormCreate(TObject *Sender)
{
 Form3->Chart1->AutoSize = false;
 Form3->Chart1->AllowZoom = false;
 //Form3->Chart1->AllowPanning = true;

 Form3->ScrollBar1->Max = 10;
 Form3->ScrollBar1->Position = 0;

 Chart1->BottomAxis->Automatic = false;
 // Chart1->BottomAxis->Maximum = MaxVisableX;

 Form3->Chart1->LeftAxis->AutomaticMaximum = true;
 // Form3->Chart1->LeftAxis->SetMinMax(MinTlog,MaxTlog);

 DragAcceptFiles(Handle,true);  // включить буксировку файлов
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormShow(TObject *Sender) // первоначальная установка границ, легенда
{
 // легенда
 Form3->Chart1->Series[0]->Title = "Датчик 1 (";
 Form3->Chart1->Series[0]->Title += ProgramSettings.DS1Name;
 Form3->Chart1->Series[0]->Title += ") ";
 Form3->Chart1->Series[0]->Title += IntToStr(DS1_Radiator.Quality);
 Form3->Chart1->Series[0]->Title += "%";

 Form3->Chart1->Series[1]->Title = "Датчик 2 (";
 Form3->Chart1->Series[1]->Title += ProgramSettings.DS2Name;
 Form3->Chart1->Series[1]->Title += ") ";
 Form3->Chart1->Series[1]->Title += IntToStr(DS2_Caldron.Quality);
 Form3->Chart1->Series[1]->Title += "%";

 // границы
 Form3->Chart1->LeftAxis->AutomaticMaximum = true;
 Form3->Chart1->LeftAxis->SetMinMax(ProgramSettings.MinT/*-0.5*/,ProgramSettings.MaxT/*+0.5*/);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::ScrollBar1Change(TObject *Sender)
{
 if(ScrollBar1->Position >= Form3->Series1->MaxXValue()-58){return;};
 Chart1->BottomAxis->Minimum = ScrollBar1->Position;
 Chart1->BottomAxis->Maximum = ScrollBar1->Position + MaxVisableX; // exception!
}
//---------------------------------------------------------------------------

void __fastcall TForm3::ScrollBar1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
 switch (ScrollCode)
 {
  case scTrack:
   // без следующей конструкции, при перемотке например назад говорит что то вроде MaxValue mast be >= MinValue
   if (Chart1->BottomAxis->Minimum < ScrollPos)
   {
    Chart1->BottomAxis->Maximum = ScrollPos + MaxVisableX;
    Chart1->BottomAxis->Minimum = ScrollPos;
   }else
   {
    Chart1->BottomAxis->Minimum = ScrollPos;
    Chart1->BottomAxis->Maximum = ScrollPos + MaxVisableX;
   }
  break;

  case scEndScroll:
   Chart1->SetFocus();
  break;

  default: return;
 } 
}
//---------------------------------------------------------------------------

