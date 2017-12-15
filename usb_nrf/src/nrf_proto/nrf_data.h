#ifndef _NRF_DATA_H
#define _NRF_DATA_H
#include "stm32f10x.h"

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef int8_t  INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef char    BYTE;
typedef uint8_t BOOL;    

#define TRUE    1
#define FALSE   0
#define NULL    0

#define DATABUFCOUNT   5
#define ARPINFOCOUNT   20
#define ROUTERITEMCOUNT 40
#define SMALLDATABUFCOUNT 50
#define ROUTERNODEINFOCOUNT 10

#define PAYLOADLEGTH 32
#define PALYOADHEADERLENGTH 9
#define PAYLOADDATALENGTH 23
#define DATABUF_PAYLOADCOUNT 10
#define MACLENGTH  5



#define DISTURBVAL   3      //信道状况最大值 用于自动信道切换 
#define MINFAIELDCOUNT  500
#define LINKTIMEOUT  3       //心跳包超时数 
#define MAXFAILEDCOUNT  10  //发送失败次数
#define MAXACKTIME  1000   //接收ACK的超时时间
#define MAXRETRYCOUNT 3    //未接收到ACK时的重试次数
#define MAXRECVTIME   1000  //接收数据超时
#define HEARTPACKTIME 1000  //心跳包发送间隔
#define ARPINFOTIME   10000 //未接收到ACk的超时时间
#define ARPINFOTIMEOUT 5000 //失去连接超时

#define THREADTIME   1500   //线程执行间隔，单位us
#define LINKINFO_INTERVAL 200  //发送连接请求间隔

enum PAYLOADSTATUS{IDLE,WAIT_USE,WAIT_FREE,WAIT_START,WAIT_SEND,WAIT_PATH,WAIT_RECV,WAIT_SEND_RETRY,WAIT_ACK,WAIT_RETRY,SEND_FAILED,SEND_SUCCESS,RECV_FAILED,WAIT_HANDLER};
enum LINKSTATUS{DISCONNECT,STARTCONNECT,WAIT_CONNECT,CONNECTED,ROUTERWORK};
enum LINKTYPE{NODE2NODE,NODE2ROUTER,ROUTER2NODE};
enum NODETYPE{NODE,ROUTER};
enum PACKAGETYPE{STARTLINK,STARTLINKACK,BUILDCONNECT,BUILDCONNECTACK,CHECKLINK,CHECKLINKACK,SYNCARP,ONEDATA,ONEDATAACK,MULTIDATA,MULTIDATAACK,ROUTERBROADCAST,CHANGECHANNEL,BROADCASTONEDATA,BROADCASTMULTIDATA};
enum WORKSTATUS{SENDSTATE,RECVSTATE,STANDYSTATE};
enum LAST_SEND_STATUS{NO_PAYLOAD,SMALLDATABUF_TYPE,DATABUF_TYPE};
enum ARPSTATUS{ARP_NO_USE,ARP_WAIT_USE,ARP_CONNECTED,ARP_TIMEOUT,ARP_DISCONNECT};
enum ROUTERNODESTATUS{ROUTERNODE_NO_USE,ROUTERNODE_WAIT_USE,ROUTERNODE_NORMOL,ROUTERNODE_TIME_OUT};

#define PACKAGE_SUCCESS   0x01
#define PACKAGE_FAILED    0x00
typedef void (*HANDLECALLBACK)(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status);

#pragma pack(1)
typedef struct 
{
	UINT8 src_addr;              //源地址
	UINT8 dst_addr;              //目的地址
	UINT8 pack_type;             //payload 类型
	UINT8 seq;                   //payload 序列号
	UINT16 ID;                    //发送payload ID
	UINT16 RID;                   //接收ID
	UINT8 length;                //payload 数据长度
}PAYLOADHEADER,*LPPAYLOADHEADER;      //payload头结构

typedef struct
{
	PAYLOADHEADER header;          //payload 头
	UINT8 buf[PAYLOADDATALENGTH]; //数据缓冲区
}PAYLOAD,*LPPAYLOAD;           //payload结构 
#pragma pack()
typedef struct
{
	UINT8 status;                //数据包发送状态
	UINT8 payload_total_count;         //包分割的payload总数
	UINT8 payload_num;           //当前发送/接收的payload编号
	UINT8 last_payload_bytes;    //最后一个payload字节数
	UINT8 send_failed_count;     //发送失败次数
	UINT8 wait_or_retry_count;           //当前payload重发数/收到前次payload后等待数
	UINT16 time_count;             //超时计数
	UINT8 dst_channel;           //发送信道
	UINT8 dst_mac[MACLENGTH];    //发送的目的MAC地址
	HANDLECALLBACK HandleCallBack;   //回调
	UINT8 data_ack[DATABUF_PAYLOADCOUNT]; //接收确认ACK
	PAYLOAD payload[DATABUF_PAYLOADCOUNT];  //payload缓冲区  
}DATABUF,*LPDATABUF;             //数据包结构

typedef struct
{
	UINT8 status;         //数据包发送状态
	UINT8 retry_count;           //当前数据包重发数
	UINT8 send_failed_count;     //发送失败次数
	UINT8 dst_channel;           //发送信道
	UINT8 dst_mac[MACLENGTH];    //发送的目的MAC地址
	UINT16 time_count;             //超时计数
	HANDLECALLBACK HandleCallBack;   //回调
	PAYLOAD payload;      //数据包
}SMALLDATABUF,*LPSMALLDATABUF;   //同步数据包

typedef struct
{
	UINT8 src_addr;       //源地址
	UINT8 dst_addr;       //目的地址
	UINT8 channel;         //信道
	UINT8 failed_count;   //通信失败次数
	UINT8 status;         //路径状态
}ROUTERITEM,*LPROUTERITEM;   //路由结构

#pragma pack(1)
typedef struct
{
	UINT8 status;         //状态
//	UINT8 addr;           //网络地址
	BOOL bStatic;         //是否为静态地址   静态地址不会被释放
	UINT16 time_count;    //超时计数
	UINT8 mac[MACLENGTH]; //MAC地址
}ARPINFO;               //网络地址和MAC地址映射结构
#pragma pack()

typedef struct
{
	UINT8 status;              //路由节点状态
	UINT8 channel;             //路由信道
	UINT16 time_count;         //超时计数
	UINT16 router_id;          //路由ID
	UINT8 mac[MACLENGTH];      //MAC地址
}ROUTERNODEINFO;       //路由节点信息结构




typedef struct
{
	UINT8 addr;               //路由模式使用 分配的地址
	UINT8 channel;                   //使用的信道
	UINT8 mac[MACLENGTH];            //mac地址
}LINKDATA,*LPLINKDATA;   //请求连接发送的数据结构

typedef struct
{
	UINT8 channel;              //信道
	UINT8 mac[MACLENGTH];       //mac地址
	UINT16 router_id;           //路由ID
}ROUTERBROADCASTDATA,*LPROUTERBROADCASTDATA;  //路由广播信息结构


typedef struct
{
	UINT8 local_mac[MACLENGTH];    //节点的mac
	UINT8 link_mac[MACLENGTH];     //要连接的节点mac
	UINT16 router_id;              //路由id
	UINT8 init_channel;            //初始信道
	UINT8 link_channel;            //用户指定的连接信道
	BOOL bUserChannel;            //是否使用用户指定信道
	BOOL bBroadcast;              //是否广播
	UINT8 node_type;               //节点类型
	UINT8 link_type;               //连接类型
	ARPINFO ArpInfo[ARPINFOCOUNT]; //地址信息
}NRFSTOREDATA,*LPNRFSTOREDATA;

#pragma pack(1)
typedef struct
{
	UINT8 node_type;      //节点类型
	UINT8 link_type;      //连接类型
	UINT8 link_status;    //连接状态
	UINT8 time_count;  //超时计数
	UINT8 work_status; //工作状态
	UINT8 heart_status;  //心跳包状态
	UINT8 last_send_status;        //指示上次发送缓冲区
	UINT8 channel;            //当前信道
	UINT8 node_addr;           //当前地址
	UINT8 node_mac[MACLENGTH]; //当前mac地址
	UINT8 link_node_addr;      //连接节点地址
	UINT8 link_node_mac[MACLENGTH]; //连接节点mac地址
	UINT16 router_id;            //路由id
	UINT32 shift_failed_count; //切换失败计数
	UINT32 shift_count;        //切换成功计数器
	UINT32 recv_count;         //接收数据包数
	UINT8  link_node_count;    //连接节点数
}NODESTATUS;           //节点状态结构

typedef struct
{
	NODESTATUS NodeStatus;
	UINT8 init_channel;
	BOOL bUserChannel;            //是否使用用户指定信道
	BOOL bBroadcast;              //是否广播
}SENDPARAMDATA,*LPSENDPARAMDATA;
#pragma pack()
#endif
