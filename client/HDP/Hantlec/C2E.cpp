/*
 ***************************************************************************
 * ����   Nie
 * ��ַ�� www.tchzt.com
 * ���䣺 niejianjun@tchzt.com
 *
 ***************************************************************************
 */
//-------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "C2E.h"
#include "main.h"
#include <zcommon.h>
#include "ConfigFile.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormC2E *FormC2E;
//---------------------------------------------------------------------------
__fastcall TFormC2E::TFormC2E(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------  
/**
    ����
    �������洫���ʱ�䣬���Σ��������Ϣ
*/
int TFormC2E::SetData(const char* pkgno, const char* session)
{
    //TODO: Add your source code here
    memset(_pkgno,0,sizeof(pkgno));
    memset(_session,0,sizeof(session));
    strcpy(_pkgno,pkgno);
    strcpy(_session,session);
}
//---------------------------------------------------------------------------
/**
    ������ʾ�¼�����
    �����ݿⲢ��ѯ��ʾҪ���Ե�������Ϣ
*/
void __fastcall TFormC2E::FormShow(TObject *Sender)
{
    EnvList->Clear();
    ChkList->Clear();
    MergeList->Clear();

    //�����ݿ�
    int ret = db.Open("ZTIC_DB_HOST",DB_USER,DB_PASSWD,DB_DBAlias);
    if( ret )
    {
        FmDistMan->InsertLog("�������ݿ����");
        Close();
        return ;
    }

    //��ѯ����ʾ
    LoadData();
}
//---------------------------------------------------------------------------
/**
    ����
    ������ѯ����ʾҪ�ϲ���������Ϣ
*/
int TFormC2E::LoadData()
{
    //TODO: Add your source code here
    EnvList->Clear();
    ChkList->Clear();
    MergeList->Clear();
    debugout("pkgno [%s] session [%s]\n",_pkgno,_session);
    int ret = db.VQuery(1024,"select sid,vchno,codeacc,amount,pbcvt,fname from dists where pkgno='%s' and session='%s' and  \
                      isenvelop=1 and codecode='0' and proctype=1",_pkgno,_session);
    MyRes res = db.Store();
    MyRow row = res.Fetch();
    TListItem* li;
    while(row!=NULL)
    {
        li = EnvList->Items->Add();
        li->Caption = row[0];
        li->SubItems->Insert(0,row[1]);
        li->SubItems->Insert(1,row[2]);
        li->SubItems->Insert(2,row[3]);
        li->SubItems->Insert(3,row[4]);
        li->Data = (void*)row[5];
        row = res.Fetch();
    }

    ret = db.VQuery(1024,"select sid,vchno,codeacc,amount,vchtype,fname \
            from extimage where pkgno='%s' and session='%s' and  \
            matched=-1",_pkgno,_session);
    if(ret)
    {
        debugout("sql error [%s]\n",mysql_error(db.Handle()))  ;
        return ret;
    }
    res = db.Store();
    row = res.Fetch();
    while(row!=NULL)
    {
        li = ChkList->Items->Add();
        li->Caption = row[0];
        li->SubItems->Insert(0,row[1]);
        li->SubItems->Insert(1,row[2]);
        li->SubItems->Insert(2,row[3]);
        li->SubItems->Insert(3,row[4]);
        li->Data = (void*)row[5];
        row = res.Fetch();
    }

    ret = db.VQuery(1024,"select sid,vchno,codeacc,amount,pbcvt,fname \
            from at_fsort where pkgno='%s' and session='%s' and  \
            isenvelop=2",_pkgno,_session);
    if(ret)
    {
        debugout("sql error [%s]\n",mysql_error(db.Handle()))  ;
        return ret;
    }
    res = db.Store();
    row = res.Fetch();
    while(row!=NULL)
    {
        li = MergeList->Items->Add();
        li->Caption = row[0];
        li->SubItems->Insert(0,row[1]);
        li->SubItems->Insert(1,row[2]);
        li->SubItems->Insert(2,row[3]);
        li->SubItems->Insert(3,row[4]);
        li->Data = (void*)row[5];
        row = res.Fetch();
    }
    return 0;
}
//---------------------------------------------------------------------------
/**
    �����¼�����
    ����һ�ʼ�¼�󱣴��¼�
*/
void __fastcall TFormC2E::btnOkClick(TObject *Sender)
{
    //���¹��Խ�������ݿ�
    int ret = db.VQuery(512,"update imgfile,extimage set imgfile.proctype=1 \
            where imgfile.fname=extimage.fname and extimage.matched!=-1 \
            and extimage.pkgno='%s' and extimage.session='%s'",_pkgno,_session);
    if(ret)
    {
        debugout("sql error [%s]\n",mysql_error(db.Handle()));
        FmDistMan->InsertLog("��ѯ����..!");
        return;
    }
}
//---------------------------------------------------------------------------
