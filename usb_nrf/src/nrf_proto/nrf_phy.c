#include "nrf_phy.h"

NODESTATUS NodeStatus;      //�ڵ�״̬
static DATABUF DataBuf[DATABUFCOUNT];   //�������ݻ���    ������ݳ���DATABUFLENGTH
static SMALLDATABUF SmallDataBuf[SMALLDATABUFCOUNT];  //�������ݻ���  ��󳤶�PAYLOADDATALENGTH
ARPINFO ArpInfo[ARPINFOCOUNT];   //ARP��
static ROUTERNODEINFO RouterNodeInfo[ROUTERNODEINFOCOUNT];  //���յ���·�ɽڵ���Ϣ
static UINT8 init_channel=124;      //��ʼ���ŵ� 124  �����ŵ�0~124
static const UINT8 init_mac[5]={0xFF,0xFF,0xFF,0xFF,0xFF}; //��ʼ��mac��ַFFFFFFFFFF
static const UINT8 multi_mac[5]={0xFE,0xFF,0xFF,0xFF,0xFF}; //����ͨ��1��ַ�����ڽ���һ�Զ���Ϣ
static const UINT8 init_addr=0xFF;           //��ʼ����ַ
static const UINT8 init_router_addr=0x01;    //·�ɳ�ʼ����ַ
static UINT8 nrf_mac[5]={0xFF,0xEE,0x10,0x11,0x21}; //�ڵ�mac
static UINT8 link_mac[5]={0xFF,0xEE,0x10,0x11,0x20};//���ӵĽڵ�mac
static UINT16 router_id=1; //·��id
static UINT8 user_channel=0;  //�û�ָ���������ŵ�
static BOOL bUserChannel=FALSE;    //�Ƿ�ʹ���û�ָ�����ŵ�
static BOOL bBroadcast=TRUE;         //�Ƿ�㲥
static UINT8 node_type=NODE;
static UINT8 link_type=NODE2NODE;
static UINT8 search_index=0;
static UINT8 search_small_index=0;
static HANDLECALLBACK RecvHandler=NULL;
static HANDLECALLBACK BroadcastRecvHandler=NULL;
static UINT8 CallBackBuf[PAYLOADLEGTH*DATABUF_PAYLOADCOUNT];
void InitNrfProto(void)   //��ʼ��Э��
{

	//��ʼ��������Ϊ����״̬
	nrfmemset((BYTE *)&DataBuf,0,sizeof(DATABUF)*DATABUFCOUNT);
	nrfmemset((BYTE *)&SmallDataBuf,0,sizeof(SMALLDATABUF)*SMALLDATABUFCOUNT);
	
	//���·�ɽڵ���Ϣ
	nrfmemset((BYTE *)&RouterNodeInfo,0,sizeof(ROUTERNODEINFO)*ROUTERNODEINFOCOUNT);
	//���ARP��
	nrfmemset((BYTE *)&ArpInfo,0,sizeof(ARPINFO)*ARPINFOCOUNT);
	
	search_index=0;
	search_small_index=0;
	//��ʼ���ڵ�״̬
	nrfmemset((BYTE *)&NodeStatus,0,sizeof(NODESTATUS));
	if(IsHaveStoreData())
	{
		ReadNrfParam();
	}
	NodeStatus.node_type=node_type;
	NodeStatus.work_status=RECVSTATE;
	NodeStatus.link_type=link_type;
	copyMac(NodeStatus.node_mac,nrf_mac);
	NodeStatus.router_id=router_id;
	copyMac(NodeStatus.link_node_mac,link_mac); 
	NodeStatus.channel=user_channel;   
	if(NODE==NodeStatus.node_type)   //���ǽڵ�
	{
		NodeStatus.link_status=DISCONNECT;
		NodeStatus.node_addr=init_addr;    
		if(NODE2ROUTER==NodeStatus.link_type)
		{
			NodeStatus.link_node_addr=init_router_addr;
			//Ӳ����ʼ��
			NRF24L01_Config(init_channel,(UINT8 *)init_mac);
		}
		else
		{
			
			NodeStatus.link_node_addr=init_addr;
						//Ӳ����ʼ��
			NRF24L01_Config(init_channel,NodeStatus.node_mac);
			if(bUserChannel)
			{
				NodeStatus.channel=user_channel;    //ʹ���û�ָ���ŵ�
			}
			else
			{
				NodeStatus.channel=AppropriateChannelDetect();  //�Զ�ѡ������ŵ�
			}
		}
		EnablePipe1((UINT8 *)multi_mac);
	}
	else   //����·��
	{
		NodeStatus.link_status=ROUTERWORK;
		NodeStatus.node_addr=init_router_addr;
				//Ӳ����ʼ��
		NRF24L01_Config(NodeStatus.channel,NodeStatus.node_mac);
		if(!bUserChannel)
		{
			NodeStatus.channel=AppropriateChannelDetect();
		}
		else
		{
			NodeStatus.channel=user_channel;
		}
		SetChannel(NodeStatus.channel);
	}
	
}

void SetDefaultRecvHandler(HANDLECALLBACK pFun)
{
	RecvHandler=pFun;
}

void SetBroadcastRecvHandler(HANDLECALLBACK pFun)//���ý��չ㲥���ݴ�����
{
	BroadcastRecvHandler=pFun;
}

void GetParamData(LPSENDPARAMDATA pBuf)
{
	nrfmemcopy((BYTE *)&pBuf->NodeStatus,(BYTE *)&NodeStatus,sizeof(NODESTATUS));
	pBuf->init_channel=init_channel;
	pBuf->bBroadcast=bBroadcast;
	pBuf->bUserChannel=bUserChannel;
}

void SetRouterId(UINT16 id)                //����ROUTER�ڵ㣬������·��ID������NODE�ڵ㣬������Ҫ�������ID
{
	router_id=id;
	NodeStatus.router_id=id;
	StoreNrfParam();
}

void SetLocalMac(UINT8 *mac)
{
	copyMac(nrf_mac,mac);
	copyMac(NodeStatus.node_mac,mac);
	StoreNrfParam();
}
void SetLinkMac(UINT8 *mac)
{
	copyMac(link_mac,mac);
	copyMac(NodeStatus.link_node_mac,mac);
	StoreNrfParam();
}
void SetBroadcastChannel(UINT8 channel)
{
	init_channel=channel;
	StoreNrfParam();
}
void SetLinkChannel(UINT8 channel)
{
	user_channel=channel;
	if(bUserChannel)
	{
		NodeStatus.channel=user_channel;
	}
	StoreNrfParam();
}
void SetBroadcastSwitch(BOOL bVal)
{
	bBroadcast=bVal;
	StoreNrfParam();
}
void SetUserChannelSwitch(BOOL bVal)
{
	bUserChannel=bVal;
	if(!bUserChannel)
	{
		NodeStatus.channel=AppropriateChannelDetect();
	}
	StoreNrfParam();
}

void SetNode(UINT8 node,UINT8 link)
{
	node_type=node;
	link_type=link;
	StoreNrfParam();
	InitNrfProto();
}

void StoreNrfParam(void)
{
	NRFSTOREDATA store_data;
	nrfmemset((BYTE *)&store_data,0,sizeof(NRFSTOREDATA));
	store_data.bBroadcast=bBroadcast;
	store_data.bUserChannel=bUserChannel;
	store_data.init_channel=init_channel;
	store_data.link_channel=user_channel;
	store_data.router_id=router_id;
	store_data.node_type=node_type;
	store_data.link_type=link_type;
	copyMac(store_data.local_mac,nrf_mac);
	copyMac(store_data.link_mac,link_mac);
	nrfmemcopy((BYTE *)store_data.ArpInfo,(BYTE *)ArpInfo,sizeof(ARPINFO)*ARPINFOCOUNT);
	NrfWriteDataToStore((UINT8 *)&store_data,sizeof(NRFSTOREDATA));
}

static void ReadNrfParam(void)
{
	NRFSTOREDATA store_data;
	UINT8 i=0;
	if(IsHaveStoreData())
	{
		nrfmemset((BYTE *)&store_data,0,sizeof(NRFSTOREDATA));
		NrfReadDataFromStore((UINT8 *)&store_data,sizeof(NRFSTOREDATA));
		bBroadcast=store_data.bBroadcast;
		bUserChannel=store_data.bUserChannel;
		init_channel=store_data.init_channel;
		user_channel=store_data.link_channel;
		router_id=store_data.router_id;
		node_type=store_data.node_type;
		link_type=store_data.link_type;
		copyMac(nrf_mac,store_data.local_mac);
		copyMac(link_mac,store_data.link_mac);
		nrfmemcopy((BYTE *)ArpInfo,(BYTE *)store_data.ArpInfo,sizeof(ARPINFO)*ARPINFOCOUNT);
		for(i=0;i<ARPINFOCOUNT;i++)
		{
				DelOneArpByAddr(i,0);
		}
	}
}

static void DetectAndHandleDisturb(void)  //����ŵ�״��������������ֵ�ж��Ƿ���Ҫ�ı�ȫ�����ŵ�
{
	LPLINKDATA pLinkData;
	UINT8 idx=0;
	if(bBroadcast&&bUserChannel&&(NodeStatus.shift_failed_count>DISTURBVAL*NodeStatus.shift_count)&&NodeStatus.shift_failed_count>MINFAIELDCOUNT)
	{
			if((NODE2NODE==NodeStatus.link_type&&NODE==NodeStatus.node_type)||ROUTER==NodeStatus.node_type)
			{
			idx=MallocIdleSmallDataBufId();
			if(0xFF==idx)
			{
				return;
			}
			SmallDataBuf[idx].retry_count=0;
			nrfmemset((BYTE *)&(SmallDataBuf[idx].payload),0,sizeof(PAYLOAD));
			SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
			SmallDataBuf[idx].payload.header.pack_type=CHANGECHANNEL;
			SmallDataBuf[idx].payload.header.src_addr=NodeStatus.node_addr;
			SmallDataBuf[idx].payload.header.dst_addr=0xFF;
			SmallDataBuf[idx].payload.header.length=sizeof(LINKDATA);
			SmallDataBuf[idx].dst_channel=NodeStatus.channel;
			copyMac(SmallDataBuf[idx].dst_mac,multi_mac);
			pLinkData=(LPLINKDATA)SmallDataBuf[idx].payload.buf;
			copyMac(pLinkData->mac,NodeStatus.node_mac);
			pLinkData->channel=NodeStatus.channel+1;
			if(pLinkData->channel>=124)
			{
				pLinkData->channel=0;
				if(pLinkData->channel==NodeStatus.channel)
				{
					pLinkData->channel++;
				}
				if(pLinkData->channel==init_channel)
				{
					pLinkData->channel++;
				}
			}
			else if(pLinkData->channel==init_channel)
			{
				pLinkData->channel++;
				if(pLinkData->channel>=124)
				{
					pLinkData->channel=0;
					if(pLinkData->channel==NodeStatus.channel)
					{
						pLinkData->channel++;
					}
					if(pLinkData->channel==init_channel)
					{
						pLinkData->channel++;
					}
				}
			}
			pLinkData->addr=NodeStatus.node_addr;
			SmallDataBuf[idx].status=WAIT_SEND;
		}
	}
}

static BOOL LinkNodeByMacAndChannel(UINT8 *mac,UINT8 channel)              //ͨ��mac��channel����ָ���ڵ�
{
	UINT8 id;
	LINKDATA LinkData;
	if(DISCONNECT==NodeStatus.link_status&&NODE==NodeStatus.node_type)
	{
		id=MallocIdleSmallDataBufId();
		if(0xFF==id)
		{
				return FALSE;
		}
		SmallDataBuf[id].retry_count=0;
		nrfmemset((BYTE *)&(SmallDataBuf[id].payload),0,sizeof(PAYLOAD));
		SmallDataBuf[id].payload.header.ID=GetSmallDataBufID();
		SmallDataBuf[id].payload.header.pack_type=STARTLINK;
		SmallDataBuf[id].payload.header.src_addr=init_addr;
		SmallDataBuf[id].payload.header.dst_addr=init_addr;
		SmallDataBuf[id].payload.header.length=sizeof(LINKDATA);
		
		NodeStatus.channel=channel;                  //���������ŵ�
		copyMac(NodeStatus.link_node_mac,mac);       //��������mac��ַ
		copyMac(LinkData.mac,NodeStatus.node_mac);
		
		copyMac(SmallDataBuf[id].dst_mac,mac);
		if(NODE2NODE==NodeStatus.link_type)
		{
			LinkData.channel=NodeStatus.channel;
			SmallDataBuf[id].dst_channel=init_channel;
		}
		else
		{
			SetPipe0Mac(NodeStatus.node_mac);         //���ó�ʵ��mac
			SetChannel(NodeStatus.channel);           //���������ŵ�
			LinkData.channel=NodeStatus.channel;
			SmallDataBuf[id].dst_channel=NodeStatus.channel;
		}
		nrfmemcopy((BYTE *)SmallDataBuf[id].payload.buf,(BYTE *)&LinkData,sizeof(LINKDATA));
		SmallDataBuf[id].status=WAIT_SEND;
		NodeStatus.link_status=STARTCONNECT;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static void HearPackCallBack(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status)
{
		UINT8 id;
		if(status)
		{
			NodeStatus.time_count=0;
		}
		else
		{
			NodeStatus.time_count++;
			if(LINKTIMEOUT<NodeStatus.time_count)
			{
					NodeStatus.link_status=DISCONNECT;
					if(NODE2ROUTER==NodeStatus.link_type)
					{
						id=SerachRouterNodeByRouterId(NodeStatus.router_id);
						if(0xFF!=id)
						{
							DelRouterNodeInfo(id);
						}
						SetPipe0Mac((UINT8 *)init_mac);
					}
					NodeStatus.node_addr=init_addr;
					SetChannel(init_channel);
			}
		}
	NodeStatus.heart_status=0;
}

static void AutoLink(void)    //����NODE��ROUTER���Զ�����
{
	UINT8 id=0;
	static UINT16 time=0;
	if(NODE==NodeStatus.node_type&&NODE2ROUTER==NodeStatus.link_type)
	{
		if(DISCONNECT==NodeStatus.link_status&&0!=NodeStatus.router_id)
		{
			if(bBroadcast)
			{
				//�ȴ����յ�·�ɽڵ���Ϣ���ٽ�������
				id=SerachRouterNodeByRouterId(NodeStatus.router_id);
				if(0xFF!=id)
				{
					if(ROUTERNODE_NORMOL==RouterNodeInfo[id].status)
					{				
						LinkNodeByMacAndChannel(RouterNodeInfo[id].mac,RouterNodeInfo[id].channel);
					}
				}
			}
			else
			{
				//��������
				time++;
				if(time>LINKINFO_INTERVAL)
				{
					time=0;
					LinkNodeByMacAndChannel(NodeStatus.link_node_mac,NodeStatus.channel);
				}
			}
		}
	}
	else if(NODE==NodeStatus.node_type&&NODE2NODE==NodeStatus.link_type&&bBroadcast)
	{
		time++;
		if(time>LINKINFO_INTERVAL&&DISCONNECT==NodeStatus.link_status)
		{
			time=0;
			LinkNodeByMacAndChannel(NodeStatus.link_node_mac,NodeStatus.channel);
		}
	}
}

static void SendHeartPack(void)
{
	static UINT16 time=0;
	UINT8 id;
	LPLINKDATA pLinkData;
	LPROUTERBROADCASTDATA pRouterData;
	if(CONNECTED==NodeStatus.link_status&&NODE==NodeStatus.node_type&&!NodeStatus.heart_status)
	{
		time++;
		if(time>HEARTPACKTIME)
		{
			
			id=MallocIdleSmallDataBufId();
			if(0xFF==id)
			{
				return;
			}
			time=0;
			SmallDataBuf[id].retry_count=0;
			nrfmemset((BYTE *)&(SmallDataBuf[id].payload),0,sizeof(PAYLOAD));
			SmallDataBuf[id].payload.header.ID=GetSmallDataBufID();
			SmallDataBuf[id].payload.header.pack_type=CHECKLINK;
			SmallDataBuf[id].payload.header.src_addr=NodeStatus.node_addr;
			SmallDataBuf[id].payload.header.dst_addr=NodeStatus.link_node_addr;
			SmallDataBuf[id].payload.header.length=sizeof(LINKDATA);
			SmallDataBuf[id].dst_channel=NodeStatus.channel;
			SmallDataBuf[id].HandleCallBack=HearPackCallBack;
			copyMac(SmallDataBuf[id].dst_mac,NodeStatus.link_node_mac);
			pLinkData=(LPLINKDATA)SmallDataBuf[id].payload.buf;
			copyMac(pLinkData->mac,NodeStatus.node_mac);
			pLinkData->channel=NodeStatus.channel;
			pLinkData->addr=NodeStatus.node_addr;
			SmallDataBuf[id].status=WAIT_SEND;
			NodeStatus.heart_status=1;
		}
	}
	else if(ROUTER==NodeStatus.node_type&&0!=NodeStatus.router_id&&bBroadcast)   //�㲥�ڵ���Ϣ
	{
			time++;
		if(time>HEARTPACKTIME)
		{
			
			id=MallocIdleSmallDataBufId();
			if(0xFF==id)
			{
				return;
			}
			time=0;
			SmallDataBuf[id].retry_count=0;
			nrfmemset((BYTE *)&(SmallDataBuf[id].payload),0,sizeof(PAYLOAD));
			SmallDataBuf[id].payload.header.ID=GetSmallDataBufID();
			SmallDataBuf[id].payload.header.pack_type=ROUTERBROADCAST;
			SmallDataBuf[id].payload.header.src_addr=NodeStatus.node_addr;
			SmallDataBuf[id].payload.header.dst_addr=init_addr;
			SmallDataBuf[id].payload.header.length=sizeof(LINKDATA);
			SmallDataBuf[id].dst_channel=init_channel;
			SmallDataBuf[id].HandleCallBack=NULL;
			copyMac(SmallDataBuf[id].dst_mac,init_mac);
			pRouterData=(LPROUTERBROADCASTDATA)SmallDataBuf[id].payload.buf;
			copyMac(pRouterData->mac,NodeStatus.node_mac);
			pRouterData->channel=NodeStatus.channel;
			pRouterData->router_id=NodeStatus.router_id;
			SmallDataBuf[id].status=WAIT_SEND;
		}
	}
}



BOOL nrf_senddata(UINT8 dst_addr,UINT16 length,UINT8 *buf,HANDLECALLBACK fun)   //�������ݽӿ�
{
	UINT8 i,idx;
	UINT16 pack_id;
	if(length>PAYLOADLEGTH*DATABUF_PAYLOADCOUNT)
	{
		return FALSE;
	}
	if(length<=PAYLOADDATALENGTH)
	{
		idx=MallocIdleSmallDataBufId();
		if((SMALLDATABUFCOUNT-3)<idx)
		{
			return FALSE;
		}
		SmallDataBuf[idx].dst_channel=NodeStatus.channel;

		SmallDataBuf[idx].HandleCallBack=fun;
		SmallDataBuf[idx].payload.header.src_addr=NodeStatus.node_addr;
		SmallDataBuf[idx].payload.header.dst_addr=dst_addr;
		SmallDataBuf[idx].payload.header.length=length;
		SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
		
		nrfmemcopy((BYTE *)SmallDataBuf[idx].payload.buf,(BYTE *)buf,length);
		if(0xFF!=dst_addr)
		{
			SmallDataBuf[idx].payload.header.pack_type=ONEDATA;
			if(dst_addr!=NodeStatus.node_addr)
			{			
				SmallDataBuf[idx].status=WAIT_PATH;
			}
			else
			{
				SmallDataBuf[idx].status=SEND_SUCCESS;
				i=MallocIdleSmallDataBufId();
				if((SMALLDATABUFCOUNT-3)<i)
				{
					return FALSE;
				}
				nrfmemcopy((BYTE *)&SmallDataBuf[i],(BYTE *)&SmallDataBuf[idx],sizeof(SMALLDATABUF));
				SmallDataBuf[i].status=WAIT_HANDLER;
			}
		}
		else
		{
			SmallDataBuf[idx].payload.header.pack_type=BROADCASTONEDATA;
			copyMac(SmallDataBuf[idx].dst_mac,multi_mac);
			SmallDataBuf[idx].status=WAIT_SEND;
		}
	}
	else
	{
		idx=MallocIdleDataBufId();
		if(0xFF==idx)
		{
			return FALSE;
		}
		DataBuf[idx].dst_channel=NodeStatus.channel;	
		DataBuf[idx].HandleCallBack=fun;
		DataBuf[idx].payload_total_count=length/PAYLOADDATALENGTH+1;
		DataBuf[idx].last_payload_bytes=length%PAYLOADDATALENGTH;
		pack_id=GetDataBufID();
		for(i=0;i<DataBuf[idx].payload_total_count;i++)
		{
			DataBuf[idx].payload[i].header.src_addr=NodeStatus.node_addr;
			DataBuf[idx].payload[i].header.dst_addr=dst_addr;
			DataBuf[idx].payload[i].header.seq=i;
			DataBuf[idx].payload[i].header.ID=pack_id;
			if(0xFF!=dst_addr)
			{
				DataBuf[idx].payload[i].header.pack_type=MULTIDATA;
			}
			else
			{
				DataBuf[idx].payload[i].header.pack_type=BROADCASTMULTIDATA;
			}
			if(i!=(DataBuf[idx].payload_total_count-1))
			{
				DataBuf[idx].payload[i].header.length=PAYLOADDATALENGTH;
			}
			else
			{
				DataBuf[idx].payload[i].header.length=DataBuf[idx].last_payload_bytes;
			}
			nrfmemcopy((BYTE *)DataBuf[idx].payload[i].buf,(BYTE *)buf+i*PAYLOADDATALENGTH,DataBuf[idx].payload[i].header.length);
		}
		if(0xFF!=dst_addr)
		{
				if(dst_addr!=NodeStatus.node_addr)
				{
					DataBuf[idx].status=WAIT_PATH;
				}
				else
				{
					DataBuf[idx].status=SEND_SUCCESS;
					i=MallocIdleDataBufId();
					if(0xFF==i)
					{
					return FALSE;
					}
					nrfmemcopy((BYTE *)&DataBuf[i],(BYTE *)&DataBuf[idx],sizeof(DATABUF));
					DataBuf[i].payload_num=DataBuf[idx].payload_total_count;
					DataBuf[i].status=WAIT_HANDLER;
				}
		}
		else
		{
			copyMac(DataBuf[idx].dst_mac,multi_mac);
			DataBuf[idx].status=WAIT_SEND;
		}
	}
	return TRUE;
}

void proto_thread(void)   //��ʱ�̣߳�ִ��Э��
{
	AutoLink();
	HandleSendPath();
	NrfSendData();
	HandleBuf();
	SendHeartPack();
	HandleArpInfoBuf();
	DetectAndHandleDisturb();
}

static void NrfSendData(void)    //���ݻ�����״̬�������� ����ǰ�ȼ�����
{
	UINT8 index,id;
	BOOL bSendFind=FALSE;
	if(NO_PAYLOAD==NodeStatus.last_send_status||DATABUF_TYPE==NodeStatus.last_send_status) //�ж��ϴη��͵����ݰ�����
	{
			switch(NodeStatus.link_status)
			{
				case DISCONNECT:
					bSendFind=FALSE;
					break;
				case STARTCONNECT:
					index=SearchSmallDataByPackType(STARTLINK,WAIT_SEND);
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINK,WAIT_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINK,WAIT_SEND_RETRY);
					}
					if(0xFF!=index)
					{
						bSendFind=TRUE;
					}
					break;
				case WAIT_CONNECT:
					index=SearchSmallDataByPackType(STARTLINKACK,WAIT_SEND);
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINKACK,WAIT_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINKACK,WAIT_SEND_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(BUILDCONNECT,WAIT_SEND);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(BUILDCONNECT,WAIT_RETRY);
					}
					if(0xFF!=index)
					{
						bSendFind=TRUE;
					}
					break;
				case CONNECTED:
					index=SearchSmallDataByPackType(CHECKLINKACK,WAIT_SEND);
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(BUILDCONNECTACK,WAIT_SEND);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_SEND);
					}
					
					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_SEND_RETRY);
					}
					if(0xFF!=index)
					{
						bSendFind=TRUE;
					}
					break;
				case ROUTERWORK:
					index=SearchSmallDataByPackType(ROUTERBROADCAST,WAIT_SEND);
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINKACK,WAIT_SEND);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINKACK,WAIT_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(STARTLINKACK,WAIT_SEND_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(CHECKLINKACK,WAIT_SEND);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataByPackType(BUILDCONNECTACK,WAIT_SEND);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_SEND);
					}

					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_RETRY);
					}
					if(0xFF==index)
					{
						index=SearchSmallDataBuf(WAIT_SEND_RETRY);
					}
					if(0xFF!=index)
					{
						bSendFind=TRUE;
					}
					break;
			}
	}
	if(!bSendFind)//����databuf����
	{
		if((NO_PAYLOAD==NodeStatus.last_send_status||SMALLDATABUF_TYPE==NodeStatus.last_send_status))
		{
			index=SearchDataBuf(WAIT_SEND);
				if(0xFF==index)
				{
					index=SearchDataBuf(WAIT_RETRY);
				}
				if(0xFF==index)
					{
						index=SearchDataBuf(WAIT_SEND_RETRY);
					}
				if(0xFF!=index)
				{
					bSendFind=TRUE;
				}
			
			if(bSendFind)
			{
				switch(NodeStatus.link_status)
				{
					case DISCONNECT:
					case STARTCONNECT:
					case WAIT_CONNECT:
						NodeStatus.last_send_status=NO_PAYLOAD;		
						break;
					case CONNECTED:
					case ROUTERWORK:
						switch(DataBuf[index].payload[DataBuf[index].payload_num].header.pack_type)
						{
							case MULTIDATA:
								if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
								{
									SetNRFTxMode(); 
									NodeStatus.shift_count++;
									if(!SendPayload(DataBuf[index].dst_mac,DataBuf[index].dst_channel,(UINT8 *)&DataBuf[index].payload[DataBuf[index].payload_num]))
									{			
										DataBuf[index].status=WAIT_SEND_RETRY;
										DataBuf[index].send_failed_count++;
									}
									else
									{
											DataBuf[index].status=WAIT_ACK;
									}
									SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
									NodeStatus.last_send_status=DATABUF_TYPE;
								}
								else
								{
									NodeStatus.shift_failed_count++;
									NodeStatus.last_send_status=NO_PAYLOAD;			
								}
								break;
						case BROADCASTMULTIDATA:
							if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
								{
									SetNRFTxMode(); 
									NodeStatus.shift_count++;
									SendPayload(DataBuf[index].dst_mac,DataBuf[index].dst_channel,(UINT8 *)&DataBuf[index].payload[DataBuf[index].payload_num]);
									DataBuf[index].payload_num++;
									if(DataBuf[index].payload_total_count==DataBuf[index].payload_num)
									{
										DataBuf[index].status=SEND_SUCCESS;
									}
									else
									{
										DataBuf[index].status=WAIT_SEND;
									}
									SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
									NodeStatus.last_send_status=DATABUF_TYPE;
								}
								else
								{
									NodeStatus.shift_failed_count++;
									NodeStatus.last_send_status=NO_PAYLOAD;			
								}
							break;
						default:
								DataBuf[index].status=WAIT_FREE;
								break;
							
					}
					break;	
				}
				
			}
			else
			{
				NodeStatus.last_send_status=NO_PAYLOAD;		
			}
		}
		else
		{
			NodeStatus.last_send_status=NO_PAYLOAD;		
		}
	}
	else  //����SMALLDATABUF����
	{
		switch(NodeStatus.link_status)
		{
			case DISCONNECT:
				switch(SmallDataBuf[index].payload.header.pack_type)
				{
					case STARTLINK:
					case STARTLINKACK:
					case BUILDCONNECT:
					case ROUTERBROADCAST:
					case BUILDCONNECTACK:
					case CHECKLINK:
					case CHECKLINKACK:
					case CHANGECHANNEL:
						SmallDataBuf[index].status=WAIT_FREE;
						break;
					case ONEDATA:
					case ONEDATAACK:
					case MULTIDATA:
					case MULTIDATAACK:
						break;					
				}
				break;
			case STARTCONNECT:
				switch(SmallDataBuf[index].payload.header.pack_type)
				{
					case STARTLINK:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								NodeStatus.link_status=DISCONNECT;
								if(NODE2ROUTER==NodeStatus.link_type)
								{
									id=SerachRouterNodeByRouterId(NodeStatus.router_id);
									if(0xFF!=id)
									{
										DelRouterNodeInfo(id);
									}
									SetPipe0Mac((UINT8 *)init_mac);
								}
									NodeStatus.node_addr=init_addr;
									SetChannel(init_channel);
								SmallDataBuf[index].status=WAIT_FREE;
							}
							else
							{
								SmallDataBuf[index].status=WAIT_ACK;
								SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
							}

							
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}
						break;
					case STARTLINKACK:
					case BUILDCONNECT:
					case ROUTERBROADCAST:
					case BUILDCONNECTACK:
					case CHECKLINK:
					case CHECKLINKACK:
					case CHANGECHANNEL:
						SmallDataBuf[index].status=WAIT_FREE;
						break;
					case ONEDATA:
					case ONEDATAACK:
					case MULTIDATA:
					case MULTIDATAACK:
						break;					
				}
				break;
			case WAIT_CONNECT:
				switch(SmallDataBuf[index].payload.header.pack_type)
				{
					case STARTLINK:
					case CHECKLINK:
					case CHECKLINKACK:
					case BUILDCONNECTACK:
					case ROUTERBROADCAST:
					case MULTIDATA:
					case MULTIDATAACK:
					case CHANGECHANNEL:
						SmallDataBuf[index].status=WAIT_FREE;
						break;
					case STARTLINKACK:
					case BUILDCONNECT:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								SmallDataBuf[index].status=WAIT_SEND_RETRY;
								SmallDataBuf[index].send_failed_count++;
							}
							else
							{
								SmallDataBuf[index].status=WAIT_ACK;
							}
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							SetChannel(NodeStatus.channel);
							SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}

					case ONEDATA:
					case ONEDATAACK:
						break;					
				}
				break;
			case CONNECTED:
				switch(SmallDataBuf[index].payload.header.pack_type)
				{
					case STARTLINK:
					case STARTLINKACK:
					case BUILDCONNECT:
					case ROUTERBROADCAST:
					case MULTIDATA:
						SmallDataBuf[index].status=WAIT_FREE;
						break;
					case BUILDCONNECTACK:
					case CHECKLINKACK:
					case ONEDATAACK:
					case MULTIDATAACK:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								SmallDataBuf[index].status=WAIT_SEND_RETRY;
								SmallDataBuf[index].send_failed_count++;
							}
							else
							{
								SmallDataBuf[index].status=WAIT_FREE;
							}
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}
						break;
					case CHECKLINK:
					case ONEDATA:
					
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								SmallDataBuf[index].status=WAIT_SEND_RETRY;
								SmallDataBuf[index].send_failed_count++;
							}
							else
							{
								SmallDataBuf[index].status=WAIT_ACK;
							}
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}
						break;
					case CHANGECHANNEL:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload);
							SmallDataBuf[index].status=WAIT_FREE;
							NodeStatus.channel=SmallDataBuf[index].payload.buf[1];
							SetChannel(NodeStatus.channel);
							SetPipe0Mac(NodeStatus.node_mac);
							NodeStatus.shift_count=0;
							NodeStatus.shift_failed_count=0;
							NodeStatus.last_send_status=NO_PAYLOAD;	
						}
						break;
					case BROADCASTONEDATA:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload);
							SmallDataBuf[index].status=SEND_SUCCESS;
							SetPipe0Mac(NodeStatus.node_mac);
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;	
						}
						break;
					default:
						SmallDataBuf[index].status=WAIT_FREE;
						break;
				}
				break;
			case ROUTERWORK:
				switch(SmallDataBuf[index].payload.header.pack_type)
				{
					case STARTLINK:
					case BUILDCONNECT:
					case CHECKLINK:

						SmallDataBuf[index].status=WAIT_FREE;
						break;
					case STARTLINKACK:
							if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
							{
								SetNRFTxMode();
								NodeStatus.shift_count++;
								if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
								{	
									SmallDataBuf[index].status=WAIT_SEND_RETRY;
									SmallDataBuf[index].send_failed_count++;
								}
								else
								{
									SmallDataBuf[index].status=WAIT_ACK;
								}
								SetChannel(NodeStatus.channel);
								SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
								NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							}
							else
							{
								NodeStatus.shift_failed_count++;
								NodeStatus.last_send_status=NO_PAYLOAD;			
							}
						break;
					case ROUTERBROADCAST:
							if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
							{
								SetNRFTxMode();
								NodeStatus.shift_count++;
								SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload);	
								SetChannel(NodeStatus.channel);
								SmallDataBuf[index].status=WAIT_FREE;
								SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
								NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							}
							else
							{
								NodeStatus.shift_failed_count++;
								NodeStatus.last_send_status=NO_PAYLOAD;			
							}
						break;
					case BUILDCONNECTACK:			
					case CHECKLINKACK:
					case ONEDATAACK:
					case MULTIDATAACK:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								SmallDataBuf[index].status=WAIT_SEND_RETRY;
								SmallDataBuf[index].send_failed_count++;
							}
							else
							{
								SmallDataBuf[index].status=WAIT_FREE;
							}
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
							SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}
						break;					
					case ONEDATA:
					case MULTIDATA:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							if(!SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload))
							{	
								SmallDataBuf[index].status=WAIT_SEND_RETRY;
								SmallDataBuf[index].send_failed_count++;
							}
							else
							{
								if(SmallDataBuf[index].payload.header.src_addr!=NodeStatus.node_addr)  //�ж���ת��������·�ɽڵ��������İ�
								{
									SmallDataBuf[index].status=WAIT_FREE;
								}
								else
								{
									SmallDataBuf[index].status=WAIT_ACK;
								}
							}
							SetPipe0Mac(NodeStatus.node_mac);  //�Զ�ACK��ʽ��ı�mac��ַ��������������������
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;
						}
						else
						{
							NodeStatus.shift_failed_count++;
							NodeStatus.last_send_status=NO_PAYLOAD;			
						}
						break;	
					case CHANGECHANNEL:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload);
							SmallDataBuf[index].status=WAIT_FREE;
							NodeStatus.channel=SmallDataBuf[index].payload.buf[1];
							SetChannel(NodeStatus.channel);
							SetPipe0Mac(NodeStatus.node_mac);
							NodeStatus.shift_count=0;
							NodeStatus.shift_failed_count=0;
							NodeStatus.last_send_status=NO_PAYLOAD;	
						}
						break;
					case BROADCASTONEDATA:
					case BROADCASTMULTIDATA:
						if(!QuickDisturbanceDetect())     //����Ƿ��������ڵ��ڷ�������
						{
							SetNRFTxMode();
							NodeStatus.shift_count++;
							SendPayload(SmallDataBuf[index].dst_mac,SmallDataBuf[index].dst_channel,(UINT8* )&SmallDataBuf[index].payload);
							SmallDataBuf[index].status=SEND_SUCCESS;
							SetPipe0Mac(NodeStatus.node_mac);
							NodeStatus.last_send_status=SMALLDATABUF_TYPE;	
						}
						break;
					default:
						SmallDataBuf[index].status=WAIT_FREE;
					break;
				}
				break;
		}
	}
	SetNRFRxMode();
}

void HandleStartLink(LPPAYLOAD pPayLoad)   //����STARTLINK�����  
{
	UINT8 idx;
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;
	LPLINKDATA pLinkData2=NULL;
	if(NODE==NodeStatus.node_type)
	{
		if(NODE2NODE==NodeStatus.link_type)
		{
			if(!cmpMac(link_mac,pLinkData->mac))
			{
				return;
			}
		}		
			copyMac(NodeStatus.link_node_mac,pLinkData->mac);
			NodeStatus.channel=pLinkData->channel;
	}
	else
	{
		if(NodeStatus.link_node_count>=ARPINFOCOUNT)
		{
			return;
		}
	}
	idx=MallocIdleSmallDataBufId();
	if(0xFF!=idx)
	{
		copyMac((UINT8 *)SmallDataBuf[idx].dst_mac,pLinkData->mac);
		SmallDataBuf[idx].payload.header.dst_addr=pPayLoad->header.src_addr;
		SmallDataBuf[idx].payload.header.src_addr=pPayLoad->header.dst_addr;
		SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
		SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
		SmallDataBuf[idx].payload.header.pack_type=STARTLINKACK;
		SmallDataBuf[idx].payload.header.length=sizeof(LINKDATA);
		pLinkData2=(LPLINKDATA)SmallDataBuf[idx].payload.buf;
		copyMac(pLinkData2->mac,NodeStatus.node_mac);
		pLinkData2->channel=NodeStatus.channel;
		SmallDataBuf[idx].status=WAIT_SEND;
		if(NODE==NodeStatus.link_type)
		{
			pLinkData2->addr=init_addr;
			SmallDataBuf[idx].dst_channel=init_channel;
			NodeStatus.link_status=WAIT_CONNECT;
		}
		else
		{
			pLinkData2->addr=GetIdleArpBuf(pLinkData->mac);
			if(ARP_CONNECTED==ArpInfo[pLinkData2->addr].status)
			{
				ArpInfo[pLinkData2->addr].time_count=0;
				ArpInfo[pLinkData2->addr].status=WAIT_USE;
			}
				SmallDataBuf[idx].dst_channel=NodeStatus.channel;
		}
		
			
	}
}

void HandleStartLinkACK(LPPAYLOAD pPayLoad)   //����STARTLINKACK��
{
	UINT8 idx;
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;

	if(!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)) //�ж��Ƿ����������ӵĽڵ�mac
	{
		return;
	}
	NodeStatus.node_addr=pLinkData->addr;
	idx=SearchSmallDataByID(pPayLoad->header.RID);
	if(0xFF!=idx)
	{
		SmallDataBuf[idx].status=SEND_SUCCESS;
	}
	else
	{
		return;
	}
	idx=MallocIdleSmallDataBufId();
	if(0xFF!=idx)
	{
		SmallDataBuf[idx].dst_channel=NodeStatus.channel;
		copyMac((UINT8 *)SmallDataBuf[idx].dst_mac,(UINT8 *)NodeStatus.link_node_mac);
		SmallDataBuf[idx].payload.header.dst_addr=pPayLoad->header.src_addr;
		SmallDataBuf[idx].payload.header.src_addr=NodeStatus.node_addr;
		SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
		SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
		SmallDataBuf[idx].payload.header.pack_type=BUILDCONNECT;
		SmallDataBuf[idx].payload.header.length=sizeof(LINKDATA);
		pLinkData=(LPLINKDATA)SmallDataBuf[idx].payload.buf;
		copyMac(pLinkData->mac,NodeStatus.node_mac);
		pLinkData->channel=NodeStatus.channel;
		pLinkData->addr=NodeStatus.node_addr;
		SmallDataBuf[idx].status=WAIT_SEND;
		NodeStatus.link_status=WAIT_CONNECT;
	}
}



void HandleBuildConnect(LPPAYLOAD pPayLoad)   //����BUILDCONNECT��
{
	UINT8 idx;
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;

	if(NODE==NodeStatus.node_type&&!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)) //�ж��Ƿ����������ӵĽڵ�mac
	{
		return;
	}
	idx=SearchSmallDataByID(pPayLoad->header.RID);
	if(0xFF!=idx)
	{
		if(ROUTER==NodeStatus.node_type)
		{
			if(!cmpMac(ArpInfo[pLinkData->addr].mac,pLinkData->mac))
			{
				return;
			}
			else
			{
				ArpInfo[pLinkData->addr].status=ARP_CONNECTED;
			}
		}
		SmallDataBuf[idx].status=SEND_SUCCESS;
	}
	else
	{
		return;
	}
	idx=MallocIdleSmallDataBufId();
	if(0xFF!=idx)
	{
		SmallDataBuf[idx].dst_channel=NodeStatus.channel;
		copyMac((UINT8 *)SmallDataBuf[idx].dst_mac,pLinkData->mac);
		SmallDataBuf[idx].payload.header.dst_addr=pPayLoad->header.src_addr;
		SmallDataBuf[idx].payload.header.src_addr=pPayLoad->header.dst_addr;
		SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
		SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
		SmallDataBuf[idx].payload.header.pack_type=BUILDCONNECTACK;
		SmallDataBuf[idx].payload.header.length=sizeof(LINKDATA);
		pLinkData=(LPLINKDATA)SmallDataBuf[idx].payload.buf;
		copyMac(pLinkData->mac,NodeStatus.node_mac);
		pLinkData->channel=NodeStatus.channel;
		SmallDataBuf[idx].status=WAIT_SEND;
		if(NODE==NodeStatus.node_type)
		{
			NodeStatus.time_count=0;
			NodeStatus.link_status=CONNECTED;
		}
	}
}

void HandleBuildConnectACK(LPPAYLOAD pPayLoad)   //����BUILDCONNECTACK��
{
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;
	UINT8 idx;
	if(!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)) //�ж��Ƿ����������ӵĽڵ�mac
	{
		return;
	}
	idx=SearchSmallDataByID(pPayLoad->header.RID);
	if(0xFF!=idx)
	{
		SmallDataBuf[idx].status=SEND_SUCCESS;
	}
	else
	{
		return;
	}
	NodeStatus.time_count=0;
	NodeStatus.link_status=CONNECTED;
}

void RelayPackage(LPPAYLOAD pPayLoad)   //ת�����ݰ�
{
	UINT8 idx;
	if(ROUTER==NodeStatus.node_type)   //�����·�ɽڵ�
		{
			if(pPayLoad->header.dst_addr<ARPINFOCOUNT)
			{
				idx=MallocIdleSmallDataBufId();
				if(0xFF!=idx)
				{
					nrfmemcopy((BYTE*)&SmallDataBuf[idx].payload,(BYTE *)pPayLoad,sizeof(PAYLOAD));
					SmallDataBuf[idx].dst_channel=NodeStatus.channel;	
					SmallDataBuf[idx].status=WAIT_PATH;
				}
			}
			else if(0xFF==pPayLoad->header.dst_addr)   //���ǹ㲥��ַ
			{
				
			}
		}
}

static void HandleOneData(LPPAYLOAD pPayLoad)       //����ONEDATA���ݰ�
{
	UINT8 idx,idx2;
	if(pPayLoad->header.dst_addr==NodeStatus.node_addr)
	{
		idx=MallocIdleSmallDataBufId();
		if(0xFF!=idx)
		{
			nrfmemcopy((BYTE*)&SmallDataBuf[idx].payload,(BYTE *)pPayLoad,sizeof(PAYLOAD));
			SmallDataBuf[idx].payload.header.ID=0xFFFF;
			SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
			SmallDataBuf[idx].status=WAIT_HANDLER;
		}
		idx2=MallocIdleSmallDataBufId();
		if(0xFF!=idx2)
		{
			nrfmemcopy((BYTE*)&SmallDataBuf[idx2].payload,(BYTE *)pPayLoad,sizeof(PAYLOAD));
			SmallDataBuf[idx2].payload.header.dst_addr=SmallDataBuf[idx].payload.header.src_addr;
			SmallDataBuf[idx2].payload.header.src_addr=SmallDataBuf[idx].payload.header.dst_addr;
			SmallDataBuf[idx2].payload.header.ID=GetSmallDataBufID();
			SmallDataBuf[idx2].payload.header.RID=pPayLoad->header.ID;
			SmallDataBuf[idx2].payload.header.pack_type=ONEDATAACK;
			SmallDataBuf[idx2].dst_channel=NodeStatus.channel;		
			SmallDataBuf[idx2].status=WAIT_PATH;
		}
	}
	else
	{
		RelayPackage(pPayLoad);
	}
}

static void HandleBroadcastOneData(LPPAYLOAD pPayLoad)       //����BROADCASTONEDATA���ݰ�
{
	UINT8 idx;
	idx=MallocIdleSmallDataBufId();
	if(0xFF!=idx)
	{
		nrfmemcopy((BYTE*)&SmallDataBuf[idx].payload,(BYTE *)pPayLoad,sizeof(PAYLOAD));
		SmallDataBuf[idx].payload.header.ID=0xFFFF;
		SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
		SmallDataBuf[idx].status=WAIT_HANDLER;
	}
}

static void HandleOneDataACK(LPPAYLOAD pPayLoad)       //����ONEDATAACK���ݰ�
{
	UINT8 idx;
	if(pPayLoad->header.dst_addr==NodeStatus.node_addr)
	{
		idx=SearchSmallDataByID(pPayLoad->header.RID);
		if(0xFF!=idx)
		{
			SmallDataBuf[idx].status=SEND_SUCCESS;
		}
		else
		{
			return;
		}
	}
	else
	{
		RelayPackage(pPayLoad);
	}
}

static void HandleMultiData(LPPAYLOAD pPayLoad)            //����MULTIDATA���ݰ�
{
	UINT8 idx,idx2;
	if(pPayLoad->header.dst_addr==NodeStatus.node_addr)
	{
		  if(0==pPayLoad->header.seq)
			{
				idx=MallocIdleDataBufId();
				if(0xFF!=idx)
				{
						nrfmemcopy((BYTE*)&DataBuf[idx].payload[0],(BYTE *)pPayLoad,sizeof(PAYLOAD));
						DataBuf[idx].payload[0].header.RID=DataBuf[idx].payload[0].header.ID;
						DataBuf[idx].payload[0].header.ID=GetDataBufID();
						DataBuf[idx].payload_num++;
						DataBuf[idx].status=WAIT_RECV;
						
				}
			}
			else
			{
					idx=SearchDataBufByID(pPayLoad->header.RID);
					if(0xFF!=idx)
					{
						nrfmemcopy((BYTE*)&DataBuf[idx].payload[DataBuf[idx].payload_num],(BYTE *)pPayLoad,sizeof(PAYLOAD));
						DataBuf[idx].payload_num++;
						if(DataBuf[idx].payload_num<DATABUF_PAYLOADCOUNT)
						{
							if(pPayLoad->header.length==PAYLOADDATALENGTH)
							{
								DataBuf[idx].status=WAIT_RECV;
							}
							else
							{
								DataBuf[idx].last_payload_bytes=pPayLoad->header.length;
								DataBuf[idx].status=WAIT_HANDLER;
							}
						}
						else
						{
							DataBuf[idx].last_payload_bytes=pPayLoad->header.length;
							DataBuf[idx].status=WAIT_HANDLER;
						}
					}
			}
			idx2=MallocIdleSmallDataBufId();
			if(0xFF!=idx&&0xFF!=idx2)
			{
					DataBuf[idx].time_count=0;   //�ȴ���������
					nrfmemcopy((BYTE*)&SmallDataBuf[idx2].payload,(BYTE *)pPayLoad,sizeof(PAYLOAD));
					SmallDataBuf[idx2].payload.header.dst_addr=pPayLoad->header.src_addr;
					SmallDataBuf[idx2].payload.header.src_addr=pPayLoad->header.dst_addr;
					SmallDataBuf[idx2].payload.header.ID=DataBuf[idx].payload[0].header.ID;
					SmallDataBuf[idx2].payload.header.RID=pPayLoad->header.ID;
					SmallDataBuf[idx2].payload.header.pack_type=MULTIDATAACK;
					SmallDataBuf[idx2].dst_channel=NodeStatus.channel;		
					SmallDataBuf[idx2].status=WAIT_PATH;
			}
	}
	else
	{
		RelayPackage(pPayLoad);
	}
}

static void HandleBroadcastMultiData(LPPAYLOAD pPayLoad)            //����BROADCASTMULTIDATA���ݰ�
{
	UINT8 idx;
	if(0==pPayLoad->header.seq)
	{
		idx=MallocIdleDataBufId();
		if(0xFF!=idx)
		{
				nrfmemcopy((BYTE*)&DataBuf[idx].payload[0],(BYTE *)pPayLoad,sizeof(PAYLOAD));
				DataBuf[idx].payload[0].header.RID=DataBuf[idx].payload[0].header.ID;
				DataBuf[idx].payload[0].header.ID=GetDataBufID();
				DataBuf[idx].payload_num++;
				DataBuf[idx].status=WAIT_RECV;
				
		}
	}
	else
	{
			idx=SearchDataBufByRID(pPayLoad->header.ID);
			if(0xFF!=idx)
			{
				
				nrfmemcopy((BYTE*)&DataBuf[idx].payload[DataBuf[idx].payload_num],(BYTE *)pPayLoad,sizeof(PAYLOAD));
				DataBuf[idx].payload_num++;
				if(DataBuf[idx].payload_num<DATABUF_PAYLOADCOUNT)
				{
					if(pPayLoad->header.length==PAYLOADDATALENGTH)
					{
						DataBuf[idx].status=WAIT_RECV;
					}
					else
					{
						DataBuf[idx].last_payload_bytes=pPayLoad->header.length;
						DataBuf[idx].status=WAIT_HANDLER;
					}
				}
				else
				{
					DataBuf[idx].last_payload_bytes=pPayLoad->header.length;
					DataBuf[idx].status=WAIT_HANDLER;
				}
			}
	}
	if(0xFF!=idx)
	{
		DataBuf[idx].time_count=0;   //��������
	}

}

static void HandleMultiDataACK(LPPAYLOAD pPayLoad)            //����MULTIDATAACK���ݰ�
{
	UINT8 idx,i;
	if(pPayLoad->header.dst_addr==NodeStatus.node_addr)
	{
		idx=SearchDataBufByID(pPayLoad->header.RID);
		if(0xFF!=idx)
		{
			if(WAIT_ACK==DataBuf[idx].status)
			{	
				if(0==pPayLoad->header.seq)
				{
						for(i=0;i<DataBuf[idx].payload_total_count;i++)
						{
							DataBuf[idx].payload[i].header.RID=pPayLoad->header.ID;
						}
						
				}
				DataBuf[idx].data_ack[DataBuf[idx].payload_num]=1;
				DataBuf[idx].payload_num++;
				DataBuf[idx].time_count=0;
				DataBuf[idx].wait_or_retry_count=0;
				if(DataBuf[idx].payload_num>=DataBuf[idx].payload_total_count)
				{
					DataBuf[idx].status=SEND_SUCCESS;
				}
				else
				{
					DataBuf[idx].status=WAIT_SEND;
				}
			}
		}
	}
	else
	{
		RelayPackage(pPayLoad);
	}
}

static void HandleCheckLink(LPPAYLOAD pPayLoad)
{
	UINT8 idx,addr;
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;
	if(NODE==NodeStatus.node_type&&!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)) //�ж��Ƿ����������ӵĽڵ�mac
	{
		return;
	}
	else if(ROUTER==NodeStatus.node_type)
	{
		addr=SearchArpByMac(pLinkData->mac);
		if(0xFF==addr||addr!=pLinkData->addr)
		{
			return;
		}
	}
		idx=MallocIdleSmallDataBufId();
		if(0xFF!=idx)
		{
			if(ROUTER==NodeStatus.node_type)
			{
				ArpInfo[addr].status=ARP_CONNECTED;
				ArpInfo[addr].time_count=0;
			}
			SmallDataBuf[idx].payload.header.dst_addr=pPayLoad->header.src_addr;
			SmallDataBuf[idx].payload.header.src_addr=pPayLoad->header.dst_addr;
			SmallDataBuf[idx].payload.header.ID=GetSmallDataBufID();
			SmallDataBuf[idx].payload.header.RID=pPayLoad->header.ID;
			SmallDataBuf[idx].payload.header.pack_type=CHECKLINKACK;
			SmallDataBuf[idx].dst_channel=NodeStatus.channel;		
			copyMac(SmallDataBuf[idx].dst_mac,pLinkData->mac);
			pLinkData=(LPLINKDATA)SmallDataBuf[idx].payload.buf;
			copyMac(pLinkData->mac,NodeStatus.node_mac);
			pLinkData->channel=NodeStatus.channel;
			pLinkData->addr=NodeStatus.node_addr;
			SmallDataBuf[idx].status=WAIT_SEND;
			
		}
}

static void HandleCheckLinkACK(LPPAYLOAD pPayLoad)
{
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;
	UINT8 idx;
	if(!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)) //�ж��Ƿ����������ӵĽڵ�mac
	{
		return;
	}
	idx=SearchSmallDataByID(pPayLoad->header.RID);
	if(0xFF!=idx)
	{
		SmallDataBuf[idx].status=SEND_SUCCESS;
	}
	else
	{
		return;
	}
}

static void HandleRouterBrocast(LPPAYLOAD pPayLoad)   //����ROUTERBROCAST��
{
	UINT8 id=0;
	LPROUTERBROADCASTDATA pRouterData=NULL;
	pRouterData=(LPROUTERBROADCASTDATA)pPayLoad->buf;
	id=GetIdleRouterNodeInfoId(pRouterData->mac);
	if(0xFF!=id)
	{
		RouterNodeInfo[id].channel=pRouterData->channel;
		copyMac(RouterNodeInfo[id].mac,pRouterData->mac);
		RouterNodeInfo[id].router_id=pRouterData->router_id;
		RouterNodeInfo[id].time_count=0;
		RouterNodeInfo[id].status=ROUTERNODE_NORMOL;
	}
}

static void HandleChangeChannel(LPPAYLOAD pPayLoad)  //����CHANGECHANNEL��
{
	LPLINKDATA pLinkData=(LPLINKDATA)pPayLoad->buf;
	if(!cmpMac(NodeStatus.link_node_mac,pLinkData->mac)||ROUTER==NodeStatus.node_type)
	{
		return;
	}
	if(pLinkData->channel!=NodeStatus.channel)
	{
		NodeStatus.channel=pPayLoad->buf[1];
		SetChannel(NodeStatus.channel);
	}
}
void NrfRecvData(void)
{
	PAYLOAD tmp;
	UINT8 pipe_num=0;
	nrfmemset((BYTE *)&tmp,0,sizeof(PAYLOAD));
	pipe_num=ReadRevData((UINT8 *)&tmp,PAYLOADLEGTH);
	NodeStatus.recv_count++;
	if(0==pipe_num)  //ͨ����ͨ���յ�������
	{
		
		if(NODE==NodeStatus.node_type&&tmp.header.dst_addr!=NodeStatus.node_addr)   //�ж�Ŀ���ַ�Ƿ񱾽ڵ��ַ
		{
				return;
		}
		switch(tmp.header.pack_type)
		{
			case STARTLINK:
				if((STARTCONNECT==NodeStatus.link_status||DISCONNECT==NodeStatus.link_status)||ROUTER==NodeStatus.node_type)  //�ڵ���봦��δ����״̬�ſ��Ա�����
				{
					HandleStartLink(&tmp);                  //����ȷ�ϰ������л���ָ���ŵ�
				}
				break;
			case STARTLINKACK:
				if(STARTCONNECT==NodeStatus.link_status)    
				{
					HandleStartLinkACK(&tmp);              //�յ�����ȷ�ϰ���ʹ�����ŵ����ͽ����������ݰ�
				}
				break;
			case BUILDCONNECT:
				if(WAIT_CONNECT==NodeStatus.link_status||ROUTER==NodeStatus.node_type)
				{
					HandleBuildConnect(&tmp);             //���ͽ�������ȷ�ϰ�����������
				}
				break;
			case BUILDCONNECTACK:
				if(WAIT_CONNECT==NodeStatus.link_status)
				{
					HandleBuildConnectACK(&tmp);         //�յ�ȷ�ϰ�����������
				}
				break;
			case ONEDATA:
				HandleOneData(&tmp);
				break;
			case ONEDATAACK:
				HandleOneDataACK(&tmp);
				break;
			case MULTIDATA:
				HandleMultiData(&tmp);
				break;
			case MULTIDATAACK:
				HandleMultiDataACK(&tmp);
				break;
			case CHECKLINK:
				HandleCheckLink(&tmp);
				break;
			case CHECKLINKACK:
				HandleCheckLinkACK(&tmp);
				break;
			case ROUTERBROADCAST:
				HandleRouterBrocast(&tmp);
				break;
			default:
				break;
		
		}	
	}
	else if(1==pipe_num)
	{
		switch(tmp.header.pack_type)
		{
			case BROADCASTONEDATA:
				HandleBroadcastOneData(&tmp);
				break;
			case BROADCASTMULTIDATA:
				HandleBroadcastMultiData(&tmp);
				break;
			case CHANGECHANNEL:
				HandleChangeChannel(&tmp);
				break;
			default:
				break;
		}
	}

}

void HandleSendPath(void )                //ȷ������MAC��ַ   
{
	UINT8 i=0;

		for(i=0;i<SMALLDATABUFCOUNT;i++)
		{
			if(WAIT_PATH==SmallDataBuf[i].status)
			{
				if(NODE==NodeStatus.node_type)
				{
					copyMac(SmallDataBuf[i].dst_mac,NodeStatus.link_node_mac);
					SmallDataBuf[i].status=WAIT_SEND;
				}
				else
				{
					if(SmallDataBuf[i].payload.header.dst_addr<ARPINFOCOUNT&&ARP_CONNECTED==ArpInfo[SmallDataBuf[i].payload.header.dst_addr].status)
					{
						copyMac(SmallDataBuf[i].dst_mac,ArpInfo[SmallDataBuf[i].payload.header.dst_addr].mac);
						SmallDataBuf[i].status=WAIT_SEND;
					}
					else
					{
						SmallDataBuf[i].status=WAIT_FREE;
					}
				}
			
			}
		}
		for(i=0;i<DATABUFCOUNT;i++)
		{
			if(WAIT_PATH==DataBuf[i].status)
			{
				if(NODE==NodeStatus.node_type)
				{
					copyMac(DataBuf[i].dst_mac,NodeStatus.link_node_mac);
					DataBuf[i].status=WAIT_SEND;
				}
				else
				{
					if(ARP_CONNECTED==ArpInfo[DataBuf[i].payload[0].header.dst_addr].status)
					{
						copyMac(DataBuf[i].dst_mac,ArpInfo[DataBuf[i].payload[0].header.dst_addr].mac);
						DataBuf[i].status=WAIT_SEND;
					}
					else
					{
						DataBuf[i].status=WAIT_FREE;
					}
				}
			}
		}
}

void HandleBuf(void)      //���ݻ�����״̬��������
{
	UINT8 i,j,id;
	UINT16 data_length;
	for(i=0;i<SMALLDATABUFCOUNT;i++)
	{
		switch(SmallDataBuf[i].status)
		{
			case SEND_SUCCESS:
				if(SmallDataBuf[i].HandleCallBack)
				{
					SmallDataBuf[i].HandleCallBack(SmallDataBuf[i].payload.header.src_addr,SmallDataBuf[i].payload.header.dst_addr,
																				 SmallDataBuf[i].payload.header.length,SmallDataBuf[i].payload.buf,PACKAGE_SUCCESS);
				}
				FreeSmallDataBuf(i);
				break;
			case WAIT_ACK:
				SmallDataBuf[i].time_count++;
			  if(SmallDataBuf[i].time_count>MAXACKTIME)
				{
					SmallDataBuf[i].time_count=0;
					SmallDataBuf[i].send_failed_count=0;
					SmallDataBuf[i].retry_count++;
					SmallDataBuf[i].status=WAIT_RETRY;
					if(SmallDataBuf[i].retry_count>MAXRETRYCOUNT)
					{
						SmallDataBuf[i].status=SEND_FAILED;
					}
				}
				break;
			case WAIT_FREE:
				FreeSmallDataBuf(i);
				break;
			case WAIT_HANDLER:
				if(RecvHandler&&BROADCASTONEDATA!=SmallDataBuf[i].payload.header.pack_type&&BROADCASTMULTIDATA!=SmallDataBuf[i].payload.header.pack_type)
				{
					RecvHandler(SmallDataBuf[i].payload.header.src_addr,SmallDataBuf[i].payload.header.dst_addr,
																				 SmallDataBuf[i].payload.header.length,SmallDataBuf[i].payload.buf,PACKAGE_SUCCESS);
				}
				if(BroadcastRecvHandler&&(BROADCASTONEDATA==SmallDataBuf[i].payload.header.pack_type||BROADCASTMULTIDATA==SmallDataBuf[i].payload.header.pack_type))
				{
					BroadcastRecvHandler(SmallDataBuf[i].payload.header.src_addr,SmallDataBuf[i].payload.header.dst_addr,
																				 SmallDataBuf[i].payload.header.length,SmallDataBuf[i].payload.buf,PACKAGE_SUCCESS);
				}
				FreeSmallDataBuf(i);
				break;
			case WAIT_RETRY:
				if(SmallDataBuf[i].retry_count>MAXRETRYCOUNT)
				{
					SmallDataBuf[i].status=SEND_FAILED;
				}
				break;
			case WAIT_SEND_RETRY:
				if(SmallDataBuf[i].send_failed_count>MAXFAILEDCOUNT)
				{
					SmallDataBuf[i].status=SEND_FAILED;
				}
				break;
			case SEND_FAILED:
				if(SmallDataBuf[i].HandleCallBack)
				{
					SmallDataBuf[i].HandleCallBack(SmallDataBuf[i].payload.header.src_addr,SmallDataBuf[i].payload.header.dst_addr,
																				 SmallDataBuf[i].payload.header.length,SmallDataBuf[i].payload.buf,PACKAGE_FAILED);
				}
				if((STARTCONNECT==NodeStatus.link_status||WAIT_CONNECT==NodeStatus.link_status)&&NODE==NodeStatus.node_type)
				{
					NodeStatus.link_status=DISCONNECT;
					NodeStatus.node_addr=init_addr;
					NodeStatus.time_count=0;
					if(NODE2ROUTER==NodeStatus.link_type)
					{
						id=SerachRouterNodeByRouterId(NodeStatus.router_id);
						if(0xFF!=id)
						{
							DelRouterNodeInfo(id);
						}
						SetPipe0Mac((UINT8 *)init_mac);
					}
					SetChannel(init_channel);
				}
				if(STARTLINKACK==SmallDataBuf[i].payload.header.pack_type&&ROUTER==NodeStatus.node_type)
				{
					DelOneArpByAddr(SmallDataBuf[i].payload.buf[0],0);
				}
				FreeSmallDataBuf(i);
				break;
		}
	}
	for(i=0;i<DATABUFCOUNT;i++)
	{
		switch(DataBuf[i].status)
		{
			case SEND_SUCCESS:
				data_length=0;
				if(DataBuf[i].HandleCallBack)
				{
					for(j=0;j<DataBuf[i].payload_total_count;j++)
					{
							nrfmemcopy((BYTE *)CallBackBuf+j*PAYLOADDATALENGTH,(BYTE *)DataBuf[i].payload[j].buf,DataBuf[i].payload[j].header.length);
							data_length+=DataBuf[i].payload[j].header.length;
					}
					DataBuf[i].HandleCallBack(DataBuf[i].payload[0].header.src_addr,DataBuf[i].payload[0].header.dst_addr,data_length,CallBackBuf,PACKAGE_SUCCESS);
				}
				FreeDataBuf(i);
				break;
			case WAIT_ACK:
				DataBuf[i].time_count++;
			  if(DataBuf[i].time_count>MAXACKTIME)
				{
					DataBuf[i].time_count=0;
					DataBuf[i].send_failed_count=0;
					DataBuf[i].wait_or_retry_count++;
					DataBuf[i].status=WAIT_RETRY;
					if(DataBuf[i].wait_or_retry_count>MAXRETRYCOUNT)
					{
						DataBuf[i].status=SEND_FAILED;
					}
				}
				break;
			case WAIT_RECV:
				DataBuf[i].time_count++;
			  if(DataBuf[i].time_count>MAXACKTIME)
				{
					DataBuf[i].time_count=0;
					DataBuf[i].status=RECV_FAILED;
				}
				break;
			case WAIT_FREE:
				FreeDataBuf(i);
				break;
			case WAIT_HANDLER:
					data_length=0;
				if(RecvHandler&&BROADCASTONEDATA!=SmallDataBuf[i].payload.header.pack_type&&BROADCASTMULTIDATA!=SmallDataBuf[i].payload.header.pack_type)
				{
					for(j=0;j<DataBuf[i].payload_num;j++)
					{
							nrfmemcopy((BYTE *)CallBackBuf+j*PAYLOADDATALENGTH,(BYTE *)DataBuf[i].payload[j].buf,DataBuf[i].payload[j].header.length);
							data_length+=DataBuf[i].payload[j].header.length;
					}
					RecvHandler(DataBuf[i].payload[0].header.src_addr,DataBuf[i].payload[0].header.dst_addr,data_length,CallBackBuf,PACKAGE_SUCCESS);
				}
				if(BroadcastRecvHandler&&(BROADCASTONEDATA==SmallDataBuf[i].payload.header.pack_type||BROADCASTMULTIDATA==SmallDataBuf[i].payload.header.pack_type))
				{
					for(j=0;j<DataBuf[i].payload_num;j++)
						{
							nrfmemcopy((BYTE *)CallBackBuf+j*PAYLOADDATALENGTH,(BYTE *)DataBuf[i].payload[j].buf,DataBuf[i].payload[j].header.length);
							data_length+=DataBuf[i].payload[j].header.length;
						}
					BroadcastRecvHandler(DataBuf[i].payload[0].header.src_addr,DataBuf[i].payload[0].header.dst_addr,data_length,CallBackBuf,PACKAGE_SUCCESS);
				}
				FreeDataBuf(i);
				break;
			case RECV_FAILED:
				FreeDataBuf(i);
				break;
			case WAIT_RETRY:
				if(DataBuf[i].wait_or_retry_count>MAXRETRYCOUNT)
				{
					DataBuf[i].status=SEND_FAILED;
				}
				break;
			case WAIT_SEND_RETRY:
				if(DataBuf[i].send_failed_count>MAXFAILEDCOUNT)
				{
					DataBuf[i].status=SEND_FAILED;
				}
				break;
			case SEND_FAILED:
					data_length=0;
				if(DataBuf[i].HandleCallBack)
				{
					for(j=0;j<DataBuf[i].payload_total_count;j++)
					{
							nrfmemcopy((BYTE *)CallBackBuf+j*PAYLOADDATALENGTH,(BYTE *)DataBuf[i].payload[j].buf,DataBuf[i].payload[j].header.length);
							data_length+=DataBuf[i].payload[j].header.length;
					}
					DataBuf[i].HandleCallBack(DataBuf[i].payload[0].header.src_addr,DataBuf[i].payload[0].header.dst_addr,data_length,CallBackBuf,PACKAGE_FAILED);
				}
				FreeDataBuf(i);
				break;
		}
	}
}

static void HandleArpInfoBuf(void)
{
	UINT8 i;
	UINT8 count=0;
	if(NODE==NodeStatus.node_type)
	{
		return;
	}
	for(i=0;i<ARPINFOCOUNT;i++)
	{
		switch(ArpInfo[i].status)
		{
			case ARP_WAIT_USE:
				ArpInfo[i].time_count++;
				if(ARPINFOTIME<ArpInfo[i].time_count)
				{
					ArpInfo[i].time_count=0;
					ArpInfo[i].status=ARP_TIMEOUT;
				}
				
				break;
			case ARP_CONNECTED:
				ArpInfo[i].time_count++;
				if(ARPINFOTIME<ArpInfo[i].time_count)
				{
					ArpInfo[i].time_count=0;
					ArpInfo[i].status=ARP_TIMEOUT;
				}
				count++;
				break;
			case ARP_TIMEOUT:
				ArpInfo[i].time_count++;
				if(ARPINFOTIMEOUT<ArpInfo[i].time_count)
				{
					ArpInfo[i].time_count=0;
					ArpInfo[i].status=ARP_DISCONNECT;
				}
				break;
			case ARP_DISCONNECT:
				DelOneArpByAddr(i,FALSE);
				break;
		}
	}
	NodeStatus.link_node_count=count;
}

UINT8 AppropriateChannelDetect(void)  //���ӵ���̶���͵��ŵ�
{
	UINT8 channel=0,min_disturb_count=11,i=0,j=0;
	UINT8 disturb_count[125];
	THREAD_LOCK(1);
	nrfmemset((BYTE *)disturb_count,0,125);
	channel=ReadChannel();
	for(i=0;i<10;i++)
	{
		for(j=0;j<125;j++)
		{
			SetChannel(i);
			if(Disturbance_Detect())
			{
				disturb_count[j]++;
			}
		}
	}
	SetChannel(channel);
	for(i=0;i<125;i++)
	{
		if(disturb_count[i]<min_disturb_count)
		{
			min_disturb_count=disturb_count[i];
			channel=i;
		}
	}
	ClearRxFIFO();
	THREAD_LOCK(0);
	return channel;
}

/**********************���º�������Ҫ����ͬ��,�ᱻ����̵߳���**************************************/
UINT8 MallocIdleDataBufId(void)    //�õ�һ������DATABUF����������Ϊ���� ��δ�ҵ�������0xFF ��֧�ֵ���󻺳�������Ϊ244
{
	UINT8 i;
	//����
	THREAD_LOCK(1);
	for(i=0;i<DATABUFCOUNT;i++)
	{
		if(IDLE==DataBuf[i].status)
		{
			DataBuf[i].status=WAIT_USE;
			//����
			THREAD_LOCK(0);
			return i;
		}
	}
	//����
	THREAD_LOCK(0);
	return 0xFF;
}	

void FreeDataBuf(UINT8 id)    //�ͷű�ʹ�õ�DATABUF,���ı�Ϊ����״̬
{
	if(id>(DATABUFCOUNT-1)||id==0xFF)
	{
		return;
	}
	//����
	THREAD_LOCK(1);
	nrfmemset((BYTE *)&DataBuf[id],0,sizeof(DATABUF));
	//����
	THREAD_LOCK(0);
}

UINT8 SearchDataBuf(UINT8 status)    //����״̬Ϊstatus��DataBuf ,������������δ�ҵ�������0xFF
{
	UINT8 i=0;
	for(i=search_index;i<DATABUFCOUNT;i++)
	{
		if(status==DataBuf[i].status)
			{
				search_index=i+1;              //�´�������ʼλ��
				return i;
			}
	}
	for(i=0;i<search_index;i++)
		{
			if(status==DataBuf[i].status)
			{
				search_index=i+1;            //�´�������ʼλ��     
				return i;
			}
		}
	return 0xFF;
}

UINT8 SearchDataBufByID(UINT16 ID)   //����ID���һ�����
{
	UINT8 i=0;
	for(i=0;i<DATABUFCOUNT;i++)
		{
			if(IDLE!=DataBuf[i].status&&ID==DataBuf[i].payload[0].header.ID)
			{
				return i;
			}
		}
	return 0xFF;
}

UINT8 SearchDataBufByRID(UINT16 ID)   //����RID���һ�����
{
	UINT8 i=0;
	for(i=0;i<DATABUFCOUNT;i++)
		{
			if(IDLE!=DataBuf[i].status&&ID==DataBuf[i].payload[0].header.RID)
			{
				return i;
			}
		}
	return 0xFF;
}

UINT8 MallocIdleSmallDataBufId(void)    //�õ�һ������SMALLDATABUF����������Ϊ���� ��δ�ҵ�������0xFF ��֧�ֵ���󻺳�������Ϊ244
{
	UINT8 i;
	//����
	THREAD_LOCK(1);
	for(i=0;i<SMALLDATABUFCOUNT;i++)
	{
		if(IDLE==SmallDataBuf[i].status)
		{
			SmallDataBuf[i].status=WAIT_USE;
			//����
			THREAD_LOCK(0);
			return i;
		}
	}
	//����
	THREAD_LOCK(0);
	return 0xFF;
}	

void FreeSmallDataBuf(UINT8 id)    //�ͷű�ʹ�õ�SMALLDATABUF,���ı�Ϊ����״̬
{
	if(id>(SMALLDATABUFCOUNT-1)||id==0xFF)
	{
		return;
	}
	//����
	THREAD_LOCK(1);
	nrfmemset((BYTE *)&SmallDataBuf[id],0,sizeof(SMALLDATABUF));
	//����
	THREAD_LOCK(0);
}

UINT8 SearchSmallDataBuf(UINT8 status)    //����״̬Ϊstatus��SmallDataBuf ,������������δ�ҵ�������0xFF
{
	UINT8 i=0;
	for(i=search_small_index;i<SMALLDATABUFCOUNT;i++)
	{
			if(status==SmallDataBuf[i].status)
			{
				search_small_index=i+1;
				return i;
			}
	}
	for(i=0;i<search_small_index;i++)
		{
			if(status==SmallDataBuf[i].status)
			{
				search_small_index=i+1;
				return i;
			}
		}
	return 0xFF;
}

UINT8 SearchSmallDataByPackType(UINT8 type,UINT8 status)    //���Ҵ����͵�type�������ݰ�
{
	UINT8 i=0;
	for(i=0;i<SMALLDATABUFCOUNT;i++)
		{
			if(status==SmallDataBuf[i].status)
			{
				if(type==SmallDataBuf[i].payload.header.pack_type)
				{
					return i;
				}
			}
		}
	return 0xFF;
}

UINT8 SearchSmallDataByID(UINT16 ID)   //����ID����������
{
	UINT8 i=0;
	for(i=0;i<SMALLDATABUFCOUNT;i++)
		{
			if(IDLE!=SmallDataBuf[i].status&&ID==SmallDataBuf[i].payload.header.ID)
			{
				return i;
			}
		}
	return 0xFF;
}

UINT16 GetSmallDataBufID(void)    //��ȡһ��Ψһ��ID,��Զ����ݻ�����   ע�⣺�з���
{
	static UINT16 id=0;
	id++;
	if(0xFFFF==id)
	{
		id=0;
	}
	return id;
}

UINT16 GetDataBufID(void)    //��ȡһ��Ψһ��ID,��Գ����ݻ�����
{
	static UINT16 id=0;
	id++;
	if(0xFFFF==id)
	{
		id=0;
	}
	return id;
}


UINT8 SearchArpByMac(const UINT8 *mac) //ͨ��mac��ַ��ѯARP������ҵ���������������δ�ҵ�������0xFF
{
	UINT8 i;
	if(NULL==mac)
	{
		return 0xFF;
	}
	for(i=0;i<ARPINFOCOUNT;i++)
	{
		if(cmpMac(mac,ArpInfo[i].mac))
		{
				return i;
		}
	}
	return 0xFF;
}

UINT8 GetIdleArpBuf(const UINT8 *mac)  //�õ�һ�����õ�arp�������������п�����ΪaddrΪ0����δ�ҵ�������0xFF
{
	UINT8 i,index;
	index=SearchArpByMac(mac);
	if(0xFF!=index)
	{
		return index;
	}
			//����
	THREAD_LOCK(1);
	for(i=0;i<ARPINFOCOUNT;i++)
	{
		if(ARP_NO_USE==ArpInfo[i].status&&(i!=NodeStatus.node_addr))
		{
			ArpInfo[i].status=ARP_WAIT_USE;
			copyMac(ArpInfo[i].mac,mac);
				//����
			THREAD_LOCK(0);
			return i;
		}
	}
		//����
		THREAD_LOCK(0);
	return 0xFF;
}


BOOL AddOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic)   //����һ��ARP��Ϣ
{
	
	if(0xFF!=addr&&addr<ARPINFOCOUNT)
	{
		//����
		THREAD_LOCK(1);
		ArpInfo[addr].status=ARP_DISCONNECT;
		ArpInfo[addr].bStatic=bStatic;
		copyMac(ArpInfo[addr].mac,mac);
				//����
		THREAD_LOCK(0);
		return TRUE;
	}
	return FALSE;
}

BOOL ModeOneArp(const UINT8 originAddr,const UINT8 newAddr,BOOL bStatic)  //����һ��ARP��Ϣ
{
	if(0xFF!=newAddr)
	{
		//����
		THREAD_LOCK(1);
		if(originAddr!=newAddr&&originAddr<ARPINFOCOUNT&&newAddr<ARPINFOCOUNT)
		{
			nrfmemcopy((BYTE *)&ArpInfo[newAddr],(BYTE *)&ArpInfo[originAddr],sizeof(ARPINFO));
			nrfmemset((BYTE *)&ArpInfo[originAddr],0,sizeof(ARPINFO));
		}
		ArpInfo[newAddr].bStatic=bStatic;
			//����
		THREAD_LOCK(0);
		return TRUE;
	}
	return FALSE;
}

BOOL DelOneArpByAddr(const UINT8 addr,BOOL bForce)    //����addrɾ��һ��ARP��Ϣ
{
	if(0xFF!=addr&&addr<ARPINFOCOUNT)
	{
		//����
		THREAD_LOCK(1);
		if(!bForce&&ArpInfo[addr].bStatic)
		{
			ArpInfo[addr].status=ARP_DISCONNECT;
		}
		else
		{
			nrfmemset((BYTE *)&ArpInfo[addr],0,sizeof(ARPINFO));
		}
		//����
		THREAD_LOCK(0);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DelOneArpByMac(const UINT8 *mac,BOOL bForce)    //����macɾ��һ��ARP��Ϣ
{
	UINT8 index;
	index=SearchArpByMac(mac);
	if(0xFF!=index)
	{
		//����
		THREAD_LOCK(1);
		if(!bForce&&ArpInfo[index].bStatic)
		{
			ArpInfo[index].status=ARP_DISCONNECT;
		}
		else
		{
			nrfmemset((BYTE *)&ArpInfo[index],0,sizeof(ARPINFO));
		}
		//����
		THREAD_LOCK(0);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void UpdateOneArp(const UINT8 addr,const UINT8 *mac,BOOL bStatic) //����һ��ARP�����mac��ַΪ��׼�����Ѵ��ڣ����£��������
{
	UINT8 index;
	index=SearchArpByMac(mac);
	if(0xFF==index)
	{
		AddOneArp(addr,mac,bStatic);
	}
	else
	{
		ModeOneArp(index,addr,bStatic);
	}
	StoreNrfParam();
}

UINT8 SerchRouterNodeByMac(const UINT8 *mac)   //����mac����·�ɽڵ���Ϣ
{
	UINT8 i;
	for(i=0;i<ROUTERNODEINFOCOUNT;i++)
	{
		if(cmpMac(RouterNodeInfo[i].mac,mac))
		{
			return i;
		}
	}
	return 0xFF;
}

UINT8 SerachRouterNodeByRouterId(const UINT8 router_id) //����router_id����·�ɽڵ���Ϣ
{
	UINT8 i;
	for(i=0;i<ROUTERNODEINFOCOUNT;i++)
	{
		if(router_id==RouterNodeInfo[i].router_id)
		{
			return i;
		}
	}
	return 0xFF;
}
UINT8 GetIdleRouterNodeInfoId(const UINT8 *mac)    //��ȡһ���յ�·�ɽڵ㻺����id
{
	UINT8 i;
	static UINT8 last_router_index=0;
	i=SerchRouterNodeByMac(mac);
	if(0xFF!=i)
	{
		return i;
	}
	else
	{
		THREAD_LOCK(1);
		i=last_router_index++;
		if(i>=ROUTERNODEINFOCOUNT)
		{
			i=0;
			last_router_index=1;
		}
		RouterNodeInfo[i].status=ROUTERNODE_WAIT_USE;
		THREAD_LOCK(0);
		return i;
	}
	return 0xFF;
}

void DelRouterNodeInfo(const UINT8 id)     //ɾ��һ��·�ɽڵ���Ϣ
{
	if(id<ROUTERNODEINFOCOUNT)
	{
		THREAD_LOCK(1);
		nrfmemset((BYTE *)&RouterNodeInfo[id],0,sizeof(ROUTERNODEINFO));
		THREAD_LOCK(0);
	}
}

/*************************************************************************************************/

