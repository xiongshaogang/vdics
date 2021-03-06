/*
 ***************************************************************************
 * 作者   Nie
 * 网址： www.tchzt.com
 * 邮箱： niejianjun@tchzt.com
 *
 ***************************************************************************
 */
//----------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "selNet.h"
#include <zcommon.h>
#include "main.h"
#include "ConfigFile.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFmSelNet *FmSelNet;
//---------------------------------------------------------------------------
__fastcall TFmSelNet::TFmSelNet(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
/**
    完成事件函数
    用来返回选择的清分类型和网点号信息
*/
void __fastcall TFmSelNet::btnOkClick(TObject *Sender)
{
    if( cbNetNo->Text.IsEmpty() )
    {
        cbNetNo->SetFocus();
        GetNetnos();
    }  

    ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TFmSelNet::FormCreate(TObject *Sender)
{
    cbNetNo->ItemIndex = 0;
}
//---------------------------------------------------------------------------
/**
    窗体显示事件函数
*/
void __fastcall TFmSelNet::FormShow(TObject *Sender)
{
    //查询并显示可以清分的网点号
    GetNetnos();
    cbNetNo->ItemIndex = 0;
    cbNetNo->SetFocus();
}
//---------------------------------------------------------------------------
/**
    窗体键盘事件
*/
void __fastcall TFmSelNet::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    switch( (int)Key )
    {
        case 13:    // enter
            PostMessage( Handle, WM_NEXTDLGCTL, 0, 0);
            break;
        case 27:    // esc
            ModalResult = mrCancel;
            break;
        case 33:    // pageup
            PostMessage( Handle, WM_NEXTDLGCTL, 1, 0);
            break;
        case 119:   // F8
            ToggleTcBtn();
            break;
        case 115:   // F4
            cbTallyMode->Checked = !cbTallyMode->Checked;
            ToggleTallyMode();
            break;
    }
}
//---------------------------------------------------------------------------
/**
    查询显示网点号函数
    用来查询并显示可以清分的网点的网点号
*/
int TFmSelNet::GetNetnos()
{
    m_vchTc = (btnCheck->Down)? 12 : (btnDaiji->Down)? 62 : 0;

    //TODO: Add your source code here
    cbNetNo->Items->Clear();

    int ret = FmDistMan->gdb.VQuery(512,"select distinct exchno from bocctrl where  substring(state,4,1)='1' and  substring(state,9,1)='0' and substring(state,10,1)='0' and session='%s' ", FmDistMan->session);
    if( ret )
    {
        MessageBox(Handle,"查询出错!\t","错误",MB_OK|MB_ICONERROR);
        return -1;
    }
    MyRes res = FmDistMan->gdb.Store();
    int count = (int)res.Count();
    if( count <= 0 ){
       btnOk->Enabled = true;
       return 0;
    }
    btnOk->Enabled = true;

    MyRow row = res.Fetch();
    while( row!=NULL )
    {
        cbNetNo->Items->Add( row[0] );
        row = res.Fetch();
    }
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TFmSelNet::cbNetNoDropDown(TObject *Sender)
{
    GetNetnos();
}
//---------------------------------------------------------------------------
int TFmSelNet::GetNetNoEx()
{
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TFmSelNet::btnCheckClick(TObject *Sender)
{
    GetNetnos();
}
//---------------------------------------------------------------------------
/**
    函数
    用来处理选择要清分的票据类型事件
*/
void TFmSelNet::ToggleTcBtn()
{
    if( btnCheck->Down )
    {
        if( btnDaiji->Enabled ) btnDaiji->Down = true;
    }
    else if( btnDaiji->Down )
    {
        if( btnAll->Enabled ) btnAll->Down = true;
        else btnCheck->Down = true;
    }
    else btnCheck->Down = true;

    GetNetnos();
}
//---------------------------------------------------------------------------

/**
    判断选择模式
*/
void TFmSelNet::ToggleTallyMode()
{
    if( !cbTallyMode->Checked ) btnCheck->Down=true;

    btnDaiji->Enabled = cbTallyMode->Checked;
    btnAll->Enabled = cbTallyMode->Checked;

    GetNetnos();
}
//---------------------------------------------------------------------------

