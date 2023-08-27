//---------------------------------------------------------------------------

#ifndef Unit4H
#define Unit4H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TForm4 : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TEdit *Edit1;
        TEdit *Edit2;
        TLabel *Label1;
        TLabel *Label2;
        TCheckBox *CheckBox1;
        TUpDown *UpDown1;
        TUpDown *UpDown2;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall CheckBox1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm4(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm4 *Form4;
//---------------------------------------------------------------------------
#endif
