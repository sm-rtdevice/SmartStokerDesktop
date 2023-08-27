//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit2.h"
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TForm2 *Form2;

//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner): TForm(Owner){}
//---------------------------------------------------------------------------

void __fastcall TForm2::ListBox1DblClick(TObject *Sender)
{
 switch (Form2->Tag)
 {
   case 0:
    Form1->Edit6->Text = ListBox1->Items->Strings[ListBox1->ItemIndex]; // выбранный датчик
   break;

   case 1:
    Form1->Edit7->Text = ListBox1->Items->Strings[ListBox1->ItemIndex]; // выбранный датчик
   break;
 }

 Form2->Close();
}
//---------------------------------------------------------------------------

