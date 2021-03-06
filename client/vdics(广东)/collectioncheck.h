//---------------------------------------------------------------------------

#ifndef collectioncheckH
#define collectioncheckH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormWsCheck : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TPanel *Panel2;
    TListView *lstFile;
    TPanel *Panel3;
    TPanel *Panel4;
    TImage *image;
    TButton *btnExit;
    TButton *btnSave;
    TButton *btnCheck;
    TEdit *txtAmount;
    TLabel *LabelAmount;
    TCheckBox *chkMain;
    void __fastcall btnExitClick(TObject *Sender);
    void __fastcall imageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall lstFileSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
    void __fastcall chkMainClick(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall btnCheckClick(TObject *Sender);
    void __fastcall txtAmountKeyPress(TObject *Sender, char &Key);
    void __fastcall txtAmountChange(TObject *Sender);
    void __fastcall txtAmountEnter(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
    int _type;
    int _sid;
    int _curSid;
    void MoneyChange(TEdit * txtMoney);
    void MoneyKeyPress(TEdit* txtMoney, char & Key);

    bool bFormatMoney;
    int nPopos;
    bool bPoint;
    
public:		// User declarations
    __fastcall TFormWsCheck(TComponent* Owner);
    int setMethod(int type, int sid);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormWsCheck *FormWsCheck;
//---------------------------------------------------------------------------
#endif
