//----------------------Настройка графика-----------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit4.h"

#include "Unit1.h"
#include "Unit3.h"
#include "include\Settings.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm4 *Form4;

extern CSettings ProgramSettings;

//---------------------------------------------------------------------------
__fastcall TForm4::TForm4(TComponent* Owner):TForm(Owner){}
//---------------------------------------------------------------------------
void __fastcall TForm4::FormShow(TObject *Sender)
{
 CheckBox1->Checked = ProgramSettings.AutoRange;

 if(ProgramSettings.AutoRange == true)
 {
  Edit1->Enabled = false;
  Edit2->Enabled = false;
 }
 else
 {
  Edit1->Enabled = true;
  Edit2->Enabled = true;
 }

  Edit1->Text = ProgramSettings.MaxT;
  Edit2->Text = ProgramSettings.MinT; // пределы графика
}
//---------------------------------------------------------------------------
void __fastcall TForm4::Button1Click(TObject *Sender)
{
 float Min;
 float Max;

 try
 {
  Max = StrToFloat(Edit1->Text);
  Min = StrToFloat(Edit2->Text);
 }
 catch(...)
 {
  return;
 }

 if(ProgramSettings.AutoRange == true) // автонастройка графика
 {
  Form3->SetAutoRange();               // расшаренная функция формы
 }
 else
 {
  Form3->Chart1->LeftAxis->AutomaticMaximum = true;
  Form3->Chart1->LeftAxis->SetMinMax(Min/*-0.5*/,Max/*+0.5*/);
  ProgramSettings.MaxT = Max;
  ProgramSettings.MinT = Min;
 }

 ProgramSettings.SaveSettings();
}
//---------------------------------------------------------------------------
void __fastcall TForm4::CheckBox1Click(TObject *Sender) // установка автомасштабировани
{
 ProgramSettings.AutoRange = CheckBox1->Checked;

 if(CheckBox1->Checked)
 {
  Edit1->Enabled = false;
  Edit2->Enabled = false;
 }
 else
 {
  Edit1->Enabled = true;
  Edit2->Enabled = true;
 }

}
//---------------------------------------------------------------------------
