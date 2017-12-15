#ifndef _NRF_PHY_H
#define _NRF_PHY_H
#include "nrf_data.h"
#include "nrf_memory.h"
#include "nrf_help.h"
#include "nrf_hal.h"

UINT8 MallocIdleDataBufId(void);
void FreeDataBuf(UINT8 id);
UINT8 SearchDataBuf(UINT8 status);
UINT8 SearchDataBufByID(UINT16 ID);
UINT8 SearchDataBufByRID(UINT16 ID);   //����RID���һ�����
UINT8 MallocIdleSmallDataBufId(void);
void FreeSmallDataBuf(UINT8 id);
UINT8 SearchSmallDataBuf(UINT8 status);
UINT8 SearchSmallDataByPackType(UINT8 type,UINT8 status);   //���Ҵ����͵�type�������ݰ�
UINT8 SearchSmallDataByID(UINT16 ID);
UINT16 GetSmallDataBufID(void);
UINT16 GetDataBufID(void);

UINT8 SerchRouterNodeByMac(const UINT8 *mac);
UINT8 SerachRouterNodeByRouterId(const UINT8 router_id);
UINT8 GetIdleRouterNodeInfoId(const UINT8 *mac);
void DelRouterNodeInfo(const UINT8 id);

UINT8 SearchArpByMac(const UINT8 *mac); //ͨ��mac��ַ��ѯARP������ҵ���������������δ�ҵ�������0xFF
UINT8 GetIdleArpBuf(const UINT8 *mac);  //�õ�һ�����õ�arp�������������п�����ΪaddrΪ0����δ�ҵ�������0xFF
BOOL AddOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic);   //����һ��ARP��Ϣ
BOOL ModeOneArp(const UINT8 originAddr,const UINT8 newAddr,BOOL bStatic);  //����һ��ARP��Ϣ
BOOL DelOneArpByAddr(const UINT8 addr,BOOL bForce);    //����addrɾ��һ��ARP��Ϣ
BOOL DelOneArpByMac(const UINT8 *mac,BOOL bForce);    //����macɾ��һ��ARP��Ϣ
void UpdateOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic); //����һ��ARP�����mac��ַΪ��׼�����Ѵ��ڣ����£��������


void SetNode(UINT8 node,UINT8 link); //�趨�ڵ����ͼ���������
void InitNrfProto(void);   //��ʼ��Э��
void SetRouterId(UINT16 id);                //����ROUTER�ڵ㣬������·��ID������NODE�ڵ㣬������Ҫ�������ID
static BOOL LinkNodeByMacAndChannel(UINT8 *mac,UINT8 channel);              //ͨ��mac����ָ���ڵ�
static void NrfSendData(void);    //���ݻ�����״̬�������� ����ǰ�ȼ�����
static void HandleStartLink(LPPAYLOAD pPayLoad);   //����STARTLINK�����  
static void HandleStartLinkACK(LPPAYLOAD pPayLoad);   //����STARTLINKACK��
static void HandleBuildConnect(LPPAYLOAD pPayLoad);   //����BUILDCONNECT��
static void HandleBuildConnectACK(LPPAYLOAD pPayLoad);   //����BUILDCONNECTACK��
static void HandleOneData(LPPAYLOAD pPayLoad);       //����ONEDATA���ݰ�
static void HandleOneDataACK(LPPAYLOAD pPayLoad);       //����ONEDATAACK���ݰ�
static void HandleMultiData(LPPAYLOAD pPayLoad);            //����MULTIDATA���ݰ�
static void HandleMultiDataACK(LPPAYLOAD pPayLoad);            //����MULTIDATAACK���ݰ�
static void HandleCheckLink(LPPAYLOAD pPayLoad);
static void HandleCheckLinkACK(LPPAYLOAD pPayLoad);
static void HandleRouterBrocast(LPPAYLOAD pPayLoad);   //����ROUTERBROCAST��
static void HandleChangeChannel(LPPAYLOAD pPayLoad);  //����CHANGECHANNEL��
static void HandleBroadcastOneData(LPPAYLOAD pPayLoad);       //����BROADCASTONEDATA���ݰ�
static void HandleBroadcastMultiData(LPPAYLOAD pPayLoad);            //����BROADCASTMULTIDATA���ݰ�
void NrfRecvData(void);
void HandleSendPath(void );                //ȷ������MAC��ַ 
void HandleBuf(void);      //���ݻ�����״̬��������
void RelayPackage(LPPAYLOAD pPayLoad);//ת�����ݰ�

static void HandleArpInfoBuf(void); //�������ӽڵ��״̬
static void AutoLink(void);    //����NODE��ROUTER���Զ�����
static void SendHeartPack(void);  //����������
static void HandleCheckLink(LPPAYLOAD pPayLoad);  //����CHECKLINK�����
static void HandleCheckLinkACK(LPPAYLOAD pPayLoad);//����CHECKLINKACK��
static void HearPackCallBack(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status);//�������ص�����
static void ReadNrfParam(void);
static void DetectAndHandleDisturb(void);  //����ŵ�״��������������ֵ�ж��Ƿ���Ҫ�ı�ȫ�����ŵ�


void SetDefaultRecvHandler(HANDLECALLBACK pFun);//���ý������ݴ�����
void SetBroadcastRecvHandler(HANDLECALLBACK pFun);//���ý��չ㲥���ݴ�����
void SetRouterId(UINT16 id);                //����ROUTER�ڵ㣬������·��ID������NODE�ڵ㣬������Ҫ�������ID
void SetLocalMac(UINT8 *mac);
void SetLinkMac(UINT8 *mac);
void SetBroadcastChannel(UINT8 channel);
void SetLinkChannel(UINT8 channel);
void StoreNrfParam(void);
void SetBroadcastSwitch(BOOL bVal);
void SetUserChannelSwitch(BOOL bVal);
void GetParamData(LPSENDPARAMDATA pBuf);

BOOL nrf_senddata(UINT8 dst_addr,UINT16 length,UINT8 *buf,HANDLECALLBACK fun);   //�������ݽӿ�
UINT8 AppropriateChannelDetect(void);  //���ӵ���̶���͵��ŵ�
void proto_thread(void);   //��ʱ�̣߳�ִ��Э��

extern ARPINFO ArpInfo[ARPINFOCOUNT];
extern NODESTATUS NodeStatus;
#endif
