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
UINT8 SearchDataBufByRID(UINT16 ID);   //根据RID查找缓冲区
UINT8 MallocIdleSmallDataBufId(void);
void FreeSmallDataBuf(UINT8 id);
UINT8 SearchSmallDataBuf(UINT8 status);
UINT8 SearchSmallDataByPackType(UINT8 type,UINT8 status);   //查找待发送的type类型数据包
UINT8 SearchSmallDataByID(UINT16 ID);
UINT16 GetSmallDataBufID(void);
UINT16 GetDataBufID(void);

UINT8 SerchRouterNodeByMac(const UINT8 *mac);
UINT8 SerachRouterNodeByRouterId(const UINT8 router_id);
UINT8 GetIdleRouterNodeInfoId(const UINT8 *mac);
void DelRouterNodeInfo(const UINT8 id);

UINT8 SearchArpByMac(const UINT8 *mac); //通过mac地址查询ARP表项，若找到，返回索引，若未找到，返回0xFF
UINT8 GetIdleArpBuf(const UINT8 *mac);  //得到一个可用的arp缓冲区索引，判空依据为addr为0，若未找到，返回0xFF
BOOL AddOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic);   //增加一条ARP信息
BOOL ModeOneArp(const UINT8 originAddr,const UINT8 newAddr,BOOL bStatic);  //更新一条ARP信息
BOOL DelOneArpByAddr(const UINT8 addr,BOOL bForce);    //根据addr删除一条ARP信息
BOOL DelOneArpByMac(const UINT8 *mac,BOOL bForce);    //根据mac删除一条ARP信息
void UpdateOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic); //更新一条ARP表项，以mac地址为基准，若已存在，更新，否则，添加


void SetNode(UINT8 node,UINT8 link); //设定节点类型及连接类型
void InitNrfProto(void);   //初始化协议
void SetRouterId(UINT16 id);                //对于ROUTER节点，设置其路由ID，对于NODE节点，设置其要连接入的ID
static BOOL LinkNodeByMacAndChannel(UINT8 *mac,UINT8 channel);              //通过mac连接指定节点
static void NrfSendData(void);    //根据缓冲区状态发送数据 发送前先检测干扰
static void HandleStartLink(LPPAYLOAD pPayLoad);   //处理STARTLINK请求包  
static void HandleStartLinkACK(LPPAYLOAD pPayLoad);   //处理STARTLINKACK包
static void HandleBuildConnect(LPPAYLOAD pPayLoad);   //处理BUILDCONNECT包
static void HandleBuildConnectACK(LPPAYLOAD pPayLoad);   //处理BUILDCONNECTACK包
static void HandleOneData(LPPAYLOAD pPayLoad);       //处理ONEDATA数据包
static void HandleOneDataACK(LPPAYLOAD pPayLoad);       //处理ONEDATAACK数据包
static void HandleMultiData(LPPAYLOAD pPayLoad);            //处理MULTIDATA数据包
static void HandleMultiDataACK(LPPAYLOAD pPayLoad);            //处理MULTIDATAACK数据包
static void HandleCheckLink(LPPAYLOAD pPayLoad);
static void HandleCheckLinkACK(LPPAYLOAD pPayLoad);
static void HandleRouterBrocast(LPPAYLOAD pPayLoad);   //处理ROUTERBROCAST包
static void HandleChangeChannel(LPPAYLOAD pPayLoad);  //处理CHANGECHANNEL包
static void HandleBroadcastOneData(LPPAYLOAD pPayLoad);       //处理BROADCASTONEDATA数据包
static void HandleBroadcastMultiData(LPPAYLOAD pPayLoad);            //处理BROADCASTMULTIDATA数据包
void NrfRecvData(void);
void HandleSendPath(void );                //确定发送MAC地址 
void HandleBuf(void);      //根据缓冲区状态处理缓冲区
void RelayPackage(LPPAYLOAD pPayLoad);//转发数据包

static void HandleArpInfoBuf(void); //处理连接节点的状态
static void AutoLink(void);    //用于NODE到ROUTER的自动连接
static void SendHeartPack(void);  //发送心跳包
static void HandleCheckLink(LPPAYLOAD pPayLoad);  //处理CHECKLINK请求包
static void HandleCheckLinkACK(LPPAYLOAD pPayLoad);//处理CHECKLINKACK包
static void HearPackCallBack(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status);//心跳包回调函数
static void ReadNrfParam(void);
static void DetectAndHandleDisturb(void);  //检测信道状况，并依据门限值判定是否需要改变全网络信道


void SetDefaultRecvHandler(HANDLECALLBACK pFun);//设置接收数据处理函数
void SetBroadcastRecvHandler(HANDLECALLBACK pFun);//设置接收广播数据处理函数
void SetRouterId(UINT16 id);                //对于ROUTER节点，设置其路由ID，对于NODE节点，设置其要连接入的ID
void SetLocalMac(UINT8 *mac);
void SetLinkMac(UINT8 *mac);
void SetBroadcastChannel(UINT8 channel);
void SetLinkChannel(UINT8 channel);
void StoreNrfParam(void);
void SetBroadcastSwitch(BOOL bVal);
void SetUserChannelSwitch(BOOL bVal);
void GetParamData(LPSENDPARAMDATA pBuf);

BOOL nrf_senddata(UINT8 dst_addr,UINT16 length,UINT8 *buf,HANDLECALLBACK fun);   //发送数据接口
UINT8 AppropriateChannelDetect(void);  //检测拥挤程度最低的信道
void proto_thread(void);   //定时线程，执行协议

extern ARPINFO ArpInfo[ARPINFOCOUNT];
extern NODESTATUS NodeStatus;
#endif
