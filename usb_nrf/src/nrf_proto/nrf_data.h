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



#define DISTURBVAL   3      //�ŵ�״�����ֵ �����Զ��ŵ��л� 
#define MINFAIELDCOUNT  500
#define LINKTIMEOUT  3       //��������ʱ�� 
#define MAXFAILEDCOUNT  10  //����ʧ�ܴ���
#define MAXACKTIME  1000   //����ACK�ĳ�ʱʱ��
#define MAXRETRYCOUNT 3    //δ���յ�ACKʱ�����Դ���
#define MAXRECVTIME   1000  //�������ݳ�ʱ
#define HEARTPACKTIME 1000  //���������ͼ��
#define ARPINFOTIME   10000 //δ���յ�ACk�ĳ�ʱʱ��
#define ARPINFOTIMEOUT 5000 //ʧȥ���ӳ�ʱ

#define THREADTIME   1500   //�߳�ִ�м������λus
#define LINKINFO_INTERVAL 200  //��������������

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
	UINT8 src_addr;              //Դ��ַ
	UINT8 dst_addr;              //Ŀ�ĵ�ַ
	UINT8 pack_type;             //payload ����
	UINT8 seq;                   //payload ���к�
	UINT16 ID;                    //����payload ID
	UINT16 RID;                   //����ID
	UINT8 length;                //payload ���ݳ���
}PAYLOADHEADER,*LPPAYLOADHEADER;      //payloadͷ�ṹ

typedef struct
{
	PAYLOADHEADER header;          //payload ͷ
	UINT8 buf[PAYLOADDATALENGTH]; //���ݻ�����
}PAYLOAD,*LPPAYLOAD;           //payload�ṹ 
#pragma pack()
typedef struct
{
	UINT8 status;                //���ݰ�����״̬
	UINT8 payload_total_count;         //���ָ��payload����
	UINT8 payload_num;           //��ǰ����/���յ�payload���
	UINT8 last_payload_bytes;    //���һ��payload�ֽ���
	UINT8 send_failed_count;     //����ʧ�ܴ���
	UINT8 wait_or_retry_count;           //��ǰpayload�ط���/�յ�ǰ��payload��ȴ���
	UINT16 time_count;             //��ʱ����
	UINT8 dst_channel;           //�����ŵ�
	UINT8 dst_mac[MACLENGTH];    //���͵�Ŀ��MAC��ַ
	HANDLECALLBACK HandleCallBack;   //�ص�
	UINT8 data_ack[DATABUF_PAYLOADCOUNT]; //����ȷ��ACK
	PAYLOAD payload[DATABUF_PAYLOADCOUNT];  //payload������  
}DATABUF,*LPDATABUF;             //���ݰ��ṹ

typedef struct
{
	UINT8 status;         //���ݰ�����״̬
	UINT8 retry_count;           //��ǰ���ݰ��ط���
	UINT8 send_failed_count;     //����ʧ�ܴ���
	UINT8 dst_channel;           //�����ŵ�
	UINT8 dst_mac[MACLENGTH];    //���͵�Ŀ��MAC��ַ
	UINT16 time_count;             //��ʱ����
	HANDLECALLBACK HandleCallBack;   //�ص�
	PAYLOAD payload;      //���ݰ�
}SMALLDATABUF,*LPSMALLDATABUF;   //ͬ�����ݰ�

typedef struct
{
	UINT8 src_addr;       //Դ��ַ
	UINT8 dst_addr;       //Ŀ�ĵ�ַ
	UINT8 channel;         //�ŵ�
	UINT8 failed_count;   //ͨ��ʧ�ܴ���
	UINT8 status;         //·��״̬
}ROUTERITEM,*LPROUTERITEM;   //·�ɽṹ

#pragma pack(1)
typedef struct
{
	UINT8 status;         //״̬
//	UINT8 addr;           //�����ַ
	BOOL bStatic;         //�Ƿ�Ϊ��̬��ַ   ��̬��ַ���ᱻ�ͷ�
	UINT16 time_count;    //��ʱ����
	UINT8 mac[MACLENGTH]; //MAC��ַ
}ARPINFO;               //�����ַ��MAC��ַӳ��ṹ
#pragma pack()

typedef struct
{
	UINT8 status;              //·�ɽڵ�״̬
	UINT8 channel;             //·���ŵ�
	UINT16 time_count;         //��ʱ����
	UINT16 router_id;          //·��ID
	UINT8 mac[MACLENGTH];      //MAC��ַ
}ROUTERNODEINFO;       //·�ɽڵ���Ϣ�ṹ




typedef struct
{
	UINT8 addr;               //·��ģʽʹ�� ����ĵ�ַ
	UINT8 channel;                   //ʹ�õ��ŵ�
	UINT8 mac[MACLENGTH];            //mac��ַ
}LINKDATA,*LPLINKDATA;   //�������ӷ��͵����ݽṹ

typedef struct
{
	UINT8 channel;              //�ŵ�
	UINT8 mac[MACLENGTH];       //mac��ַ
	UINT16 router_id;           //·��ID
}ROUTERBROADCASTDATA,*LPROUTERBROADCASTDATA;  //·�ɹ㲥��Ϣ�ṹ


typedef struct
{
	UINT8 local_mac[MACLENGTH];    //�ڵ��mac
	UINT8 link_mac[MACLENGTH];     //Ҫ���ӵĽڵ�mac
	UINT16 router_id;              //·��id
	UINT8 init_channel;            //��ʼ�ŵ�
	UINT8 link_channel;            //�û�ָ���������ŵ�
	BOOL bUserChannel;            //�Ƿ�ʹ���û�ָ���ŵ�
	BOOL bBroadcast;              //�Ƿ�㲥
	UINT8 node_type;               //�ڵ�����
	UINT8 link_type;               //��������
	ARPINFO ArpInfo[ARPINFOCOUNT]; //��ַ��Ϣ
}NRFSTOREDATA,*LPNRFSTOREDATA;

#pragma pack(1)
typedef struct
{
	UINT8 node_type;      //�ڵ�����
	UINT8 link_type;      //��������
	UINT8 link_status;    //����״̬
	UINT8 time_count;  //��ʱ����
	UINT8 work_status; //����״̬
	UINT8 heart_status;  //������״̬
	UINT8 last_send_status;        //ָʾ�ϴη��ͻ�����
	UINT8 channel;            //��ǰ�ŵ�
	UINT8 node_addr;           //��ǰ��ַ
	UINT8 node_mac[MACLENGTH]; //��ǰmac��ַ
	UINT8 link_node_addr;      //���ӽڵ��ַ
	UINT8 link_node_mac[MACLENGTH]; //���ӽڵ�mac��ַ
	UINT16 router_id;            //·��id
	UINT32 shift_failed_count; //�л�ʧ�ܼ���
	UINT32 shift_count;        //�л��ɹ�������
	UINT32 recv_count;         //�������ݰ���
	UINT8  link_node_count;    //���ӽڵ���
}NODESTATUS;           //�ڵ�״̬�ṹ

typedef struct
{
	NODESTATUS NodeStatus;
	UINT8 init_channel;
	BOOL bUserChannel;            //�Ƿ�ʹ���û�ָ���ŵ�
	BOOL bBroadcast;              //�Ƿ�㲥
}SENDPARAMDATA,*LPSENDPARAMDATA;
#pragma pack()
#endif
