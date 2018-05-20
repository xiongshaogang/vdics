/*
 ***************************************************************************
 * ����   Nie
 * ��ַ�� www.tchzt.com
 * ���䣺 niejianjun@tchzt.com
 *
 ***************************************************************************
 */
//------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "badclean.h"

#include <dstcommon.h>
#include <distControllor.h>
#include <bcbext.h>
#include <zcommon.h>
#include "main.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//---------------------------------------------------------------------
/**
    ���캯��
    �Գ�Ա�������г�ʼ��
*/
__fastcall TBadCleanDlg::TBadCleanDlg(TComponent* AOwner)
	: TForm(AOwner)
{
    m_distUI = NULL;
    count = 0;
    countjf = 0;
    countdf = 0;
}
/**
    ������ʾ�¼�����
*/
//---------------------------------------------------------------------
void __fastcall TBadCleanDlg::FormShow(TObject *Sender)
{
    if( count == 0 )
    {
        MessageBox(Handle,"û��Ʊ�ݼ�¼","��ʾ",MB_OK|MB_ICONINFORMATION);
        return;
    }

    DISTDATA *data;                     
    data = new DISTDATA[count];
    VchList->Items->Clear();

    //������ģʽ��Ʊ����Ϣ��ʾ����

    int ret;
    String b;
    
    GetAlltems(m_distUI, data,&count);   
    for( int i=0; i<count; i++ )
    {
        TListItem* li = VchList->Items->Add();
        li->Caption = data[i].vchno;
        li->ImageIndex = data[i].isEnvelop;

        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����","����",MB_OK|MB_ICONERROR);
            return ;
        }
        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����","����",MB_OK|MB_ICONERROR);
            return ;
        }
        MyRes res =   FmDistMan->gdb.Store();
        MyRow row = res.Fetch();

        ret = atoi(row[0]);

        if(ret != 0 )
        {
            b = "����";
        }
        else
            b = "�쳣";

        li->Data = (void*)i;
        String str = String(data[i].exchno)
            + "\n" + data[i].codeacc
            + "\n" + String(data[i].pbcvt)
            + "\n" + FloatToThousandStr( atof(data[i].amount)/100 )
            + "\n" + b;         
        li->SubItems->Text = str;
    }

    VchList->Items->Item[0]->Selected = true;
    delete []data;
}
//---------------------------------------------------------------------------
/**
    ����ɾ���¼�����
*/
void __fastcall TBadCleanDlg::btnDelClick(TObject *Sender)
{
    TListItem* li = VchList->Selected;
    if( !li )
    {
        return;
    }

    int index = (int)li->Data;

    //������������ɾ����Ӧ����
    DelSpecialItems(m_distUI, index);

    index = li->ImageIndex;
    
    String tmpmoney = RemoveThousandChar(li->SubItems->Strings[3].c_str());

    FmDistMan->SubOne(li->SubItems->Strings[2].c_str(),index,tmpmoney.ToDouble());

    li->Delete();

    //ɾ�����ˢ�½���
    if(RadioA->Checked == true )
    {
        count = count - 1;
        RadioAClick(NULL);
    }
    if(RadioD->Checked == true)
    {
        countjf= countjf -1;
        RadioDClick(NULL);
    }
    if(RadioC->Checked == true)
    {
       countdf = countdf -1;
       RadioCClick(NULL);
    }
     if(RadioM->Checked == true )
    {
        count = count - 1;
        RadioMClick(NULL);
    }
}
//---------------------------------------------------------------------------
/**
    ������������¼�����
*/
void __fastcall TBadCleanDlg::FormKeyPress(TObject *Sender, char &Key)
{
    switch( Key )
    {
        case 'D':
        case 'd':
            btnDelClick(0);
            break;
        case 'X':
        case 'x':
            Close();
            break;
    }
}
//---------------------------------------------------------------------------
/**
    ѡ��ȫ��Ʊ���¼�����
*/
void __fastcall TBadCleanDlg::RadioAClick(TObject *Sender)
{
    if( count <= 0 )
    {
        MessageBox(Handle,"û��Ʊ�ݼ�¼","��ʾ",MB_OK|MB_ICONINFORMATION);
        return;
    }

    DISTDATA *data;                     
    data = new DISTDATA[count];
    VchList->Items->Clear();

    //�õ����ݲ���ʾ
    GetAlltems(m_distUI, data,&count);
    int ret;
    String b;

    for( int i=0; i<count; i++ )
    {
        TListItem* li = VchList->Items->Add();
        li->Caption = data[i].vchno;
        li->ImageIndex = data[i].isEnvelop;

        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����!\t","����",MB_OK|MB_ICONERROR);
            return ;
        }
        MyRes res =   FmDistMan->gdb.Store();
        MyRow row = res.Fetch();

        ret = atoi(row[0]);

        if(ret != 0 )
        {
            b = "����";
        }
        else
            b = "�쳣";

        li->Data = (void*)i;
        String str = String(data[i].exchno)
            + "\n" + data[i].codeacc
            + "\n" + String(data[i].pbcvt)
            + "\n" + FloatToThousandStr( atof(data[i].amount)/100 )
            + "\n" + b;         
        li->SubItems->Text = str;
    }

    VchList->Items->Item[0]->Selected = true;
    delete []data;
}
//---------------------------------------------------------------------------
/**
    ѡ�н跽Ʊ���¼�����
*/
void __fastcall TBadCleanDlg::RadioDClick(TObject *Sender)
{
    if( countjf <= 0 )
    {
        VchList->Items->Clear();
        MessageBox(Handle,"û��Ʊ�ݼ�¼","��ʾ",MB_OK|MB_ICONINFORMATION);
        return;
    }

    DISTDATA *data;                     
    data = new DISTDATA[count];
    VchList->Items->Clear();

    GetAlltems(m_distUI, data,&count);
    countjf = 0;
    countdf = 0;

    int ret;
    String b;


    for( int i=0; i<count; i++ )
    {
        if(data[i].cdcode  == 1)
        {
            countdf =countdf +1;
            continue ;
        }
        TListItem* li = VchList->Items->Add();
        li->Caption = data[i].vchno;
        li->ImageIndex = data[i].isEnvelop;

        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����","����",MB_OK|MB_ICONERROR);
            return ;
        }
        MyRes res =   FmDistMan->gdb.Store();
        MyRow row = res.Fetch();

        ret = atoi(row[0]);

        if(ret != 0 )
        {
            b = "����";
        }
        else
            b = "�쳣";

        li->Data = (void*)i;
        String str = String(data[i].exchno)
            + "\n" + data[i].codeacc
            + "\n" + String(data[i].pbcvt)
            + "\n" + FloatToThousandStr( atof(data[i].amount)/100 )
            + "\n" + b;         
        li->SubItems->Text = str;

        countjf = countjf +1;
    }
    if(  VchList->Items->Count>0)
        VchList->Items->Item[0]->Selected = true;
    delete []data;
}
//---------------------------------------------------------------------------
/**
    ѡ�д���Ʊ���¼�����
*/
void __fastcall TBadCleanDlg::RadioCClick(TObject *Sender)
{
    if( countdf <= 0 )
    {
        VchList->Items->Clear();
        MessageBox(Handle,"û��Ʊ�ݼ�¼","��ʾ",MB_OK|MB_ICONINFORMATION);
        return;
    }

    DISTDATA *data;
    data = new DISTDATA[count];
    VchList->Items->Clear();

    GetAlltems(m_distUI, data,&count);
    countjf = 0;
    countdf = 0;
    countyc = 0;

    int ret;
    String b;
    
    for( int i=0; i<count; i++ )
    {
        if(data[i].cdcode  == 0)
        {
           countjf = countjf+1;
           continue ;
        }
        TListItem* li = VchList->Items->Add();
        li->Caption = data[i].vchno;
        li->ImageIndex = data[i].isEnvelop;

        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����","����",MB_OK|MB_ICONERROR);
            return ;
        }
        MyRes res =   FmDistMan->gdb.Store();
        MyRow row = res.Fetch();

        ret = atoi(row[0]);

        if(ret != 0 )
        {
            b = "����";
        }
        else
        {
            b = "�쳣";
            countyc = countyc +1;
        }
        li->Data = (void*)i;
        String str = String(data[i].exchno)
            + "\n" + data[i].codeacc
            + "\n" + String(data[i].pbcvt)
            + "\n" + FloatToThousandStr( atof(data[i].amount)/100 )
            + "\n" + b;
        li->SubItems->Text = str;
        countdf = countdf +1;
    }

    if(  VchList->Items->Count>0)
        VchList->Items->Item[0]->Selected = true;
    delete []data;
}
//---------------------------------------------------------------------------


void __fastcall TBadCleanDlg::RadioMClick(TObject *Sender)
{
    DISTDATA *data;
    data = new DISTDATA[count];
    VchList->Items->Clear();

    GetAlltems(m_distUI, data,&count);
    countjf = 0;
    countdf = 0;

     int ret;
    String b;
    for( int i=0; i<count; i++ )
    {
       
        if(data[i].cdcode  == 1)
        {
            countdf =countdf +1;
        }
        else
            countjf =countjf +1;
            
        ret = FmDistMan->gdb.VQuery(512,"select count(*) from pbcdata   where vchtype='%s' and amount=%f and  exchno='%s' and date='%s' and session='%s'", data[i].pbcvt,atof(data[i].amount)/100,data[i].exchno,FmDistMan->date,FmDistMan->session);
        if(ret)
        {
            MessageBox(Handle,"��ѯ���ݿ����","����",MB_OK|MB_ICONERROR);
            return ;
        }
        MyRes res =   FmDistMan->gdb.Store();
        MyRow row = res.Fetch();

        ret = atoi(row[0]);

        if(ret>0) continue ;

        TListItem* li = VchList->Items->Add();
        li->Caption = data[i].vchno;
        li->ImageIndex = data[i].isEnvelop;

        b = "�쳣";

        li->Data = (void*)i;
        String str = String(data[i].exchno)
            + "\n" + data[i].codeacc
            + "\n" + String(data[i].pbcvt)
            + "\n" + FloatToThousandStr( atof(data[i].amount)/100 )
            + "\n" + b;
        li->SubItems->Text = str;
    }

    if(  VchList->Items->Count>0)
        VchList->Items->Item[0]->Selected = true;
    delete []data;
}
//---------------------------------------------------------------------------
