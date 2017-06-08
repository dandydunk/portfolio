#pragma warning(disable : 4201)
#pragma warning(disable : 4127)
#pragma warning(disable : 4101)
#pragma warning(disable : 4100)
#pragma warning(disable : 4214)
#pragma warning(disable : 4189)

#define INITGUID

#define NDIS_WDM 1
#define NDIS60 1
#define NDIS_SUPPORT_NDIS6 1
#include <Fwpsk.h>
#include <ntddk.h>
#include <wdf.h>
#include <fwpmk.h>
#include <stdio.h>

#include <wsk.h>
#include "trace.h"
#include <Ntstrsafe.h>
#include "Queue.h"
#include "MList.h"
#include "mhash.h"

#define POOLTAGDRIVER 'ptd'
#define POOL_TAG_CURRENT_URL_TAG 'cut'
#define POOL_TAG_CALLOUT_STREAM 'cos'
#define POOL_TAG_HOST_NAME 'pth'
#define POOL_TAG_PAGE_PATH 'ptp'
#define POOL_TAG_SEND_BUF 'tsb'

#define BUFFER_SEND_SIZE 2048

#define STREAM_EDITOR_NDIS_OBJ_TAG 'oneS'
#define STREAM_EDITOR_NBL_POOL_TAG 'pneS'
#define STREAM_EDITOR_FLAT_BUFFER_TAG 'bfeS'


typedef struct __structpacketinjection {
	char *dataToInject;
	MDL *mdl;
} STRUCTPACKETINJECTION, *PSTRUCTPACKETINJECTION;


const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

// {3A14B548-BA3A-4811-A46E-C9DF803FF25D}
static const GUID GuidProvider =
{ 0x3a14b548, 0xba3a, 0x4811, { 0xa4, 0x6e, 0xc9, 0xdf, 0x80, 0x3f, 0xf2, 0x5d } };


// {899D432C-2DCD-482C-833D-9B2C961187C4}
static const GUID GuidCallOut =
{ 0x899d432c, 0x2dcd, 0x482c, { 0x83, 0x3d, 0x9b, 0x2c, 0x96, 0x11, 0x87, 0xc4 } };

// {1E8027D7-FD3E-42D1-BEDE-8D6B782ECB78}
static const GUID GuidCalloutFilter =
{ 0x1e8027d7, 0xfd3e, 0x42d1, { 0xbe, 0xde, 0x8d, 0x6b, 0x78, 0x2e, 0xcb, 0x78 } };

// {EB84D581-7508-464C-AB11-C6F042FD5CD9}
static const GUID GuidHttpSubLayer =
{ 0xeb84d581, 0x7508, 0x464c, { 0xab, 0x11, 0xc6, 0xf0, 0x42, 0xfd, 0x5c, 0xd9 } };


// {40C45FF6-4506-472B-A4CF-E0073C87CAAF}
static const GUID GuidCalloutV6 =
{ 0x40c45ff6, 0x4506, 0x472b, { 0xa4, 0xcf, 0xe0, 0x7, 0x3c, 0x87, 0xca, 0xaf } };


// {7FBA4821-1A35-4A0B-A3DD-64822D36ED04}
static const GUID GuidCalloutV6Filter =
{ 0x7fba4821, 0x1a35, 0x4a0b, { 0xa3, 0xdd, 0x64, 0x82, 0x2d, 0x36, 0xed, 0x4 } };


// {265C9CA7-303C-480B-8ED1-3AC020DBF29E}
static const GUID GuidSSLCallOut =
{ 0x265c9ca7, 0x303c, 0x480b, { 0x8e, 0xd1, 0x3a, 0xc0, 0x20, 0xdb, 0xf2, 0x9e } };

// {F770D46B-91B3-44D2-A079-FACBFC074C69}
static const GUID GuidSSLCalloutFilter =
{ 0xf770d46b, 0x91b3, 0x44d2, { 0xa0, 0x79, 0xfa, 0xcb, 0xfc, 0x7, 0x4c, 0x69 } };



NTSTATUS RegisterCallout(PDEVICE_OBJECT deviceObject,
	wchar_t *description,
	wchar_t *name,
	UINT32 *calloutId,
	GUID layer, GUID calloutKey,
	FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteFn,
	FWPS_CALLOUT_NOTIFY_FN notifyFn);

NTSTATUS RegisterCalloutFilter(GUID filterKey, GUID layer, GUID subLayer, GUID calloutKey,
	wchar_t *displayName, wchar_t *displayDescription,
	FWPM_FILTER_CONDITION *filterConditions, UINT32 numFilterConditions);


DRIVER_INITIALIZE DriverEntry;
VOID Unload(IN WDFDRIVER Driver);
void UnloadCallout();
void UnloadSSLCallout();
NTSTATUS MakeInjection(PDRIVER_OBJECT DriverObject);

/* device*/
NTSTATUS RegisterDevice(WDFDRIVER driver, PDEVICE_OBJECT *deviceObject);


#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
SSLClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  flowContext,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
);

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI SSLClassifyFn(
	_In_     const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_     const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_        void                           *layerData,
	_In_opt_ const void                           *classifyContext,
	_In_     const FWPS_FILTER1                   *filter,
	_In_           UINT64                         flowContext,
	_Inout_        FWPS_CLASSIFY_OUT             *classifyOut
	);

#else

void NTAPI SSLClassifyFn(
	_In_        const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         flowContext,
	_Inout_           FWPS_CLASSIFY_OUT             *classifyOut
	);

#endif





NTSTATUS NTAPI
SSLNotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
);

VOID NTAPI
SSLFlowDeleteFn(IN UINT16  layerId, IN UINT32  calloutId, IN UINT64 flowContext);





#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
ClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  flowContext,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
);

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI ClassifyFn(
	_In_     const FWPS_INCOMING_VALUES0          *inFixedValues,
	_In_     const FWPS_INCOMING_METADATA_VALUES0 *inMetaValues,
	_Inout_        void                           *layerData,
	_In_opt_ const void                           *classifyContext,
	_In_     const FWPS_FILTER1                   *filter,
	_In_           UINT64                         flowContext,
	_Inout_        FWPS_CLASSIFY_OUT0             *classifyOut
	);

#else

void NTAPI ClassifyFn(
	_In_        const FWPS_INCOMING_VALUES0          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES0 *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         flowContext,
	_Inout_           FWPS_CLASSIFY_OUT0             *classifyOut
	);

#endif

NTSTATUS NTAPI
NotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
);

VOID NTAPI
FlowDeleteFn(IN UINT16  layerId, IN UINT32  calloutId, IN UINT64 flowContext);
void NTAPI PacketInjectionCompletion(_In_ VOID *context, _Inout_ NET_BUFFER_LIST *netBufferList, _In_ BOOLEAN dispatchLevel);
void InjectPacket(UINT64 flowId, UINT16 layerId, UINT32 calloutId, char *dataToInject, size_t sizeDataToInject, NET_BUFFER_LIST **clonedNetBufferList, int setFin);
void InjectClonedPacket(UINT64 flowId, UINT16 layerId, UINT32 flags, UINT32 calloutId, NET_BUFFER_LIST *clonedNetBufferList, size_t sizeDataToInject, int setFin);

char* FlowIdToStr(UINT64 flowId) {
	int lenAA = _snprintf(NULL, 0, "%lu", flowId);
	if (lenAA <= 0) {
		return NULL;
	}
	
	char *aa = ExAllocatePoolWithTag(NonPagedPool, lenAA + 1, POOLTAGDRIVER);
	_snprintf(aa, lenAA + 1, "%ld", flowId);

	return aa;
}

void NTAPI PacketClonedInjectionCompletion(
	_In_    VOID            *context,
	_Inout_ NET_BUFFER_LIST *clonedNetBufferList,
	_In_    BOOLEAN         dispatchLevel
	);

void NTAPI PacketClonedInjectionCompletion2(
	_In_    VOID            *context,
	_Inout_ NET_BUFFER_LIST *clonedNetBufferList,
	_In_    BOOLEAN         dispatchLevel
	);

typedef struct __flow {
	int stopThread;
	PCQUEUE queueInjectionPackets;
	UINT64 flowId;
	PKTHREAD hPkThread;
	UINT64 processId;
	int foundHeader;
	int permitPacket;
	int dataLength;
	KEVENT eventQueue;
	unsigned int refs;
	KSPIN_LOCK spinLock;
	int gotInFin;
	int gotOutFin;
	UINT16 layerId;
	UINT32 calloutId;
} FLOWPACKET, *PFLOWPACKET;

typedef struct __ListInjectionPackets {
	SLIST_ENTRY SListEntry;
	char* data;
	SIZE_T data_length;
	UINT64 flow_id;
	UINT16 layerId;
	NET_BUFFER_LIST *clonedNetBufferList;
	UINT32 streamFlags;
	PFLOWPACKET pFlowPacket;
} INJECTION_PACKET, *PINJECTION_PACKET;

typedef struct __ListInjectionPacketsSSL {
	SLIST_ENTRY SListEntry;
	char* data;
	SIZE_T data_length;
	UINT64 flow_id;
	UINT16 port;
	UINT16 layerId;
	UINT32 remoteIpAddress;
	NET_BUFFER_LIST *clonedNetBufferList;
	int sendFin;
	int recvFin;
	UINT64 processId;
	UINT32 streamFlags;
	PFLOWPACKET pFlowPacket;
} INJECTION_PACKET_SSL, *PINJECTION_PACKET_SSL;

void FlowPacketAddRef(PFLOWPACKET pFlowPacket);
void FlowPacketDelete(PFLOWPACKET pFlowPacket);


typedef struct __workitemhttppacket {
	int stopThread;
	PCQUEUE queueInjectionPackets;
	PIO_WORKITEM  pWorkItem;
	SIZE_T dataLength;
	UINT64 flowId;
	UINT16 layerId;
	NET_BUFFER_LIST *clonedNetBufferList;
	UINT32 streamFlags;
	char *data;
	char *url;
	PFLOWPACKET pFlowPacket;
	PINJECTION_PACKET_SSL pInjectionPacket;
} WORKITEMHTTPPACKET, *PWORKITEMHTTPPACKET;

void InjectClonedPacket2(PWORKITEMHTTPPACKET pPacket);

int HandleInjectionPacket(PWORKITEMHTTPPACKET);

typedef struct __flowthread {
	int stopThread;
	PCQUEUE queueInjectionPackets;
	UINT64 flowId;
	PKTHREAD hPkThread;
} FLOWTHREADPACKET, *PFLOWTHREADPACKET;

VOID WorkItemHttpPacket(_In_  PDEVICE_OBJECT DeviceObject, _In_opt_ PVOID context);
int HandleInjectionPacketSSL(PINJECTION_PACKET_SSL);


VOID ThreadOrganizeHTTPFlows(_In_ PVOID StartContext);
VOID ThreadOrganizeSSLFlows(_In_ PVOID StartContext);

VOID ThreadMakeSocket(_In_ PVOID StartContext);

typedef struct __outboundflow {
	SLIST_ENTRY SListEntry;
	UINT32 remoteIp;
	UINT32 srcIp;
	unsigned short srcPort;
	IF_INDEX interfaceP;
	IF_INDEX subInterface;
	UINT64 endPointHandle;
	COMPARTMENT_ID compartmentId;
} FLOWCONTEXT, *PFLOWCONTEXT;

void CleanUp();











// {0DD12DA6-EE4A-41EE-AEAF-03D5B34AE4D0}
static const GUID GuidSSLConnectCallOut =
{ 0xdd12da6, 0xee4a, 0x41ee, { 0xae, 0xaf, 0x3, 0xd5, 0xb3, 0x4a, 0xe4, 0xd0 } };

// {810286EE-05D0-4271-A783-8D18687272AE}
static const GUID GuidSSLConnectCalloutFilter =
{ 0x810286ee, 0x5d0, 0x4271, { 0xa7, 0x83, 0x8d, 0x18, 0x68, 0x72, 0x72, 0xae } };



#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
SSLConnectClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  flowContext,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
);

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI SSLConnectClassifyFn(
	_In_     const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_     const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_        void                           *layerData,
	_In_opt_ const void                           *classifyContext,
	_In_     const FWPS_FILTER1                   *filter,
	_In_           UINT64                         flowContext,
	_Inout_        FWPS_CLASSIFY_OUT             *classifyOut
	);

#else

void NTAPI SSLConnectClassifyFn(
	_In_        const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         flowContext,
	_Inout_           FWPS_CLASSIFY_OUT             *classifyOut
	);

#endif




NTSTATUS NTAPI
SSLConnectNotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
);

VOID NTAPI
SSLConnectFlowDeleteFn(IN UINT16  layerId, IN UINT32  calloutId, IN UINT64 flowContext);

void UnloadSSLConnectCallout();

VOID WorkItemSSLPacket(_In_  PDEVICE_OBJECT DeviceObject, _In_opt_ PVOID context);