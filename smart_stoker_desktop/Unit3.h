//---------------------------------------------------------------------------

#ifndef Unit3H
#define Unit3H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Chart.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>
#include <Series.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
//---------------------------------------------------------------------------
class TForm3 : public TForm
{
__published:	// IDE-managed Components
        TChart *Chart1;
        TLineSeries *Series1;
        TLineSeries *Series2;
        TScrollBar *ScrollBar1;
        TOpenDialog *OpenDialog1;
        TButton *Button1;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Chart1DblClick(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall ScrollBar1Change(TObject *Sender);
        void __fastcall ScrollBar1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
        
private:	// User declarations

void   __fastcall          Response(TWMDropFiles &Message);
 BEGIN_MESSAGE_MAP
  MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,Response)
 END_MESSAGE_MAP(TForm);


public:		// User declarations
        __fastcall TForm3(TComponent* Owner);
        void __fastcall SetAutoRange();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm3 *Form3;
//---------------------------------------------------------------------------
#endif
