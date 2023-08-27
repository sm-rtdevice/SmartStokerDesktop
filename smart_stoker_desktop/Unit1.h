//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
#include <Grids.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TStaticText *StaticText2;
        TEdit *Edit1;
        TButton *Button1;
        TEdit *Edit2;
        TEdit *Edit3;
        TLabel *Label1;
        TLabel *Label2;
        TRadioGroup *RadioGroup1;
        TTimer *Timer1;
        TEdit *Edit4;
        TEdit *Edit5;
        TEdit *Edit6;
        TEdit *Edit7;
        TButton *Button2;
        TButton *Button3;
        TOpenDialog *OpenDialog1;
        TButton *Button4;
        TButton *Button5;
        TStatusBar *StatusBar1;
        TImageList *ImageList1;
        TButton *Button7;
        TButton *Button6;
        TButton *Button8;
        TButton *Button9;
        TButton *Button10;
        TStaticText *StaticText1;
        TCheckBox *CheckBox1;
        TCheckBox *CheckBox2;
        TButton *Button11;
        TCheckBox *CheckBox3;
        TButton *Button12;
        TButton *Button13;
        TButton *Button14;
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall Edit1KeyPress(TObject *Sender, char &Key);
        void __fastcall Edit2KeyPress(TObject *Sender, char &Key);
        void __fastcall Edit3KeyPress(TObject *Sender, char &Key);
        void __fastcall Edit4KeyPress(TObject *Sender, char &Key);
        void __fastcall Edit5KeyPress(TObject *Sender, char &Key);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Button7Click(TObject *Sender);
        void __fastcall Button6Click(TObject *Sender);
        void __fastcall StatusBar1MouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
        void __fastcall StatusBar1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall Button8Click(TObject *Sender);
        void __fastcall Button9Click(TObject *Sender);
        void __fastcall Button10Click(TObject *Sender);
        void __fastcall Button12Click(TObject *Sender);
        void __fastcall Button13Click(TObject *Sender);
        void __fastcall Button14Click(TObject *Sender);
private:	// User declarations

//void   __fastcall          Response(TWMDropFiles &Message);
// BEGIN_MESSAGE_MAP
//  MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,Response)
// END_MESSAGE_MAP(TForm);

public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
