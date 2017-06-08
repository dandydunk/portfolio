#include "driver.h"
#include <Fwpmtypes.h>
#include <Wsk.h>
#include "Utils.h"
#include "debugtrace.h"
#include "HttpParser.h"
#include <stdlib.h>
#include <Ntstrsafe.h>
#include "Sockets.h"

PDEVICE_OBJECT deviceObject = NULL;
UINT32 CalloutId, CalloutV6Id, SSLCalloutId, SSLConnectCalloutId;
WDFDEVICE wdfDevice = NULL;

HANDLE HThreadSSLFlows = NULL;

HANDLE gInjectionHandleIn = NULL;
HANDLE gInjectionHandleOut = NULL;
HANDLE hFilterEngine = NULL;
NDIS_GENERIC_OBJECT* gNdisGenericObj = NULL;
NDIS_HANDLE gNetBufferListPool = NULL;
NDIS_HANDLE gNetBufferPool = NULL;

//
KEVENT KEventNewInjectionPacketsSSL;

//
BOOL IsDriverUnloading;

//
PCQUEUE QueueSSLPackets = NULL;


//
PMHASHMAP PHashMapSSLFlows = NULL;
PMHASHMAP PHashMapHTTPFlows = NULL;

//


NPAGED_LOOKASIDE_LIST HttpFlowLookList;
long CountHttpWorkItems = 0;
long CountSSLWorkItems = 0;
BOOL ThreadHandleInjectionsSSLGoing = FALSE;
BOOL ThreadHandleInjectionsSSLStop = FALSE;

_Function_class_(DRIVER_INITIALIZE)
_IRQL_requires_same_
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);
	InitDebugTraceWriter();

	DebugTrace("%s Entry__", __FUNCTION__);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Driver INIT\r\n");

	NTSTATUS status;
	IsDriverUnloading = FALSE;
	gInjectionHandleIn = NULL;
	gInjectionHandleOut = NULL;
	gNetBufferListPool = NULL;
	gNdisGenericObj = NULL;
	CalloutId = 0;
	SSLCalloutId = 0;
	SSLConnectCalloutId = 0;

	KeInitializeEvent(&KEventNewInjectionPacketsSSL, NotificationEvent, FALSE);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "INIT sockets\r\n");
	InitSockets();

	PHashMapSSLFlows = MHashMapCreate();
	PHashMapHTTPFlows = MHashMapCreate();
	QueueSSLPackets = CQueueCreate();

	ExInitializeNPagedLookasideList(&HttpFlowLookList, NULL, NULL, POOL_NX_ALLOCATION, sizeof(FLOWPACKET), POOLTAGDRIVER, 0);

	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "thread ssl flows\r\n");
	HANDLE hThreadSSL = NULL;
	status = PsCreateSystemThread(&hThreadSSL, THREAD_ALL_ACCESS, NULL, NULL, NULL, ThreadOrganizeSSLFlows, NULL);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	ObReferenceObjectByHandle(hThreadSSL, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&HThreadSSLFlows, NULL);
	ZwClose(hThreadSSL);


	WDF_DRIVER_CONFIG config;
	WDFDRIVER driver = NULL;

	WDF_DRIVER_CONFIG_INIT(&config, NULL);
	config.DriverInitFlags = WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = Unload;


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Driver create\r\n");
	status = WdfDriverCreate(DriverObject, RegistryPath, NULL, &config, &driver);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}
				
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "register device\r\n");
	status = RegisterDevice(driver, &deviceObject);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}
	

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "make injection\r\n");
	status = MakeInjection(DriverObject);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}
	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "open filter engine\r\n");
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, NULL, &hFilterEngine);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "adding the provider\r\n");
	FWPM_PROVIDER httpProvider;
	RtlZeroMemory(&httpProvider, sizeof(httpProvider));
	httpProvider.displayData.description = L"proxyone provider";
	httpProvider.displayData.name = L"proxyone provider";
	httpProvider.providerKey = GuidProvider;
	status =  FwpmProviderAdd(hFilterEngine, &httpProvider, NULL);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "making the http sublayer\r\n");
	FWPM_SUBLAYER httpSubLayer;
	RtlZeroMemory(&httpSubLayer, sizeof(httpSubLayer));
	httpSubLayer.displayData.description = L"http sub layer";
	httpSubLayer.displayData.name = L"proxyone http sub layer";
	httpSubLayer.subLayerKey = GuidHttpSubLayer;
	httpSubLayer.weight = 100;
	httpSubLayer.providerKey = (GUID*)&GuidProvider;
	status = FwpmSubLayerAdd(hFilterEngine, &httpSubLayer, NULL);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	FWPM_FILTER_CONDITION filterConditions[1] = { 0 };
	UINT conditionIndex = 0;
	filterConditions[conditionIndex].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
	filterConditions[conditionIndex].matchType = FWP_MATCH_EQUAL;
	filterConditions[conditionIndex].conditionValue.type = FWP_UINT16;
	filterConditions[conditionIndex].conditionValue.uint16 = 80;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "register http callout\r\n");
	status = RegisterCallout(deviceObject,
		L"http traffic redirector", L"ProxyOne Http Callout",
		&CalloutId, FWPM_LAYER_STREAM_V4, GuidCallOut,
		ClassifyFn, FlowDeleteFn, NotifyFn);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}
								
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "register http callout filters\r\n");
	status = RegisterCalloutFilter(GuidCalloutFilter, FWPM_LAYER_STREAM_V4, GuidHttpSubLayer, GuidCallOut,
		L"ProxyOne Http Filter", L"filters port 80",
		filterConditions, 1);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	
	status = RegisterCallout(deviceObject,
		L"https traffic redirector", L"ProxyOne Https Callout",
		&SSLCalloutId, FWPM_LAYER_STREAM_V4, GuidSSLCallOut,
		SSLClassifyFn, SSLFlowDeleteFn, SSLNotifyFn);

	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	
	filterConditions[conditionIndex].conditionValue.uint16 = 443;
	status = RegisterCalloutFilter(GuidSSLCalloutFilter, FWPM_LAYER_STREAM_V4, FWPM_SUBLAYER_UNIVERSAL, GuidSSLCallOut,
		L"ProxyOne Https Filter", L"filters port 443", filterConditions, 1);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	status = RegisterCallout(deviceObject,
		L"https traffic redirector", L"ProxyOne Https Connect Callout",
		&SSLConnectCalloutId, FWPM_LAYER_ALE_AUTH_CONNECT_V4, GuidSSLConnectCallOut,
		SSLConnectClassifyFn, SSLConnectFlowDeleteFn, SSLConnectNotifyFn);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}

	status = RegisterCalloutFilter(GuidSSLConnectCalloutFilter, FWPM_LAYER_ALE_AUTH_CONNECT_V4, FWPM_SUBLAYER_UNIVERSAL, GuidSSLConnectCallOut,
		L"ProxyOne Https connect Filter", L"filters port 443 for connects",
		filterConditions, 1);
	if (!NT_SUCCESS(status)) {
		CleanUp();
		return status;
	}
	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Driver loaded\r\n");
	DebugTrace("%s Exit_", __FUNCTION__);

	return status;
}

NTSTATUS MakeInjection(PDRIVER_OBJECT DriverObject) {
	NTSTATUS status = STATUS_SUCCESS;

	DebugTrace("%s Entry", __FUNCTION__);

	gNdisGenericObj = NdisAllocateGenericObject(DriverObject, STREAM_EDITOR_NDIS_OBJ_TAG, 0);
	if (!gNdisGenericObj) {
		DebugTraceError("NdisAllocateGenericObject failed");
		status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	NET_BUFFER_LIST_POOL_PARAMETERS nblPoolParams = { 0 };
	nblPoolParams.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	nblPoolParams.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
	nblPoolParams.Header.Size = sizeof(nblPoolParams);
	nblPoolParams.fAllocateNetBuffer = TRUE;
	nblPoolParams.DataSize = 0;
	nblPoolParams.PoolTag = STREAM_EDITOR_NBL_POOL_TAG;

	gNetBufferListPool = NdisAllocateNetBufferListPool(gNdisGenericObj, &nblPoolParams);
	if (!gNetBufferListPool) {
		DebugTrace("NdisAllocateNetBufferListPool failed");
		status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	NET_BUFFER_POOL_PARAMETERS nbPoolParameters;
	nbPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	nbPoolParameters.Header.Revision = NET_BUFFER_POOL_PARAMETERS_REVISION_1;
	nbPoolParameters.Header.Size = NDIS_SIZEOF_NET_BUFFER_POOL_PARAMETERS_REVISION_1;
	nbPoolParameters.PoolTag = STREAM_EDITOR_NBL_POOL_TAG;
	nbPoolParameters.DataSize = 0;

	gNetBufferPool = NdisAllocateNetBufferPool(gNdisGenericObj, &nbPoolParameters);
	if (!gNetBufferPool) {
		DebugTrace("NdisAllocateNetBufferPool failed");
		status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	status = FwpsInjectionHandleCreate(AF_INET, FWPS_INJECTION_TYPE_STREAM, &gInjectionHandleIn);
	if (!NT_SUCCESS(status)) {
		DebugTrace("FwpsInjectionHandleCreate failed");
		goto Exit;
	}
	
	status = FwpsInjectionHandleCreate(AF_INET, FWPS_INJECTION_TYPE_STREAM, &gInjectionHandleOut);
	if (!NT_SUCCESS(status)) {
		DebugTrace("FwpsInjectionHandleCreate failed");
		goto Exit;
	}
Exit:
	if (!NT_SUCCESS(status)) {
		if (gNdisGenericObj) {
			NdisFreeGenericObject(gNdisGenericObj);
			gNdisGenericObj = NULL;
		}
		if (gNetBufferListPool) {
			NdisFreeNetBufferListPool(gNetBufferListPool);
			gNetBufferListPool = NULL;
		}
		if (gInjectionHandleIn) {
			FwpsInjectionHandleDestroy(gInjectionHandleIn);
			gInjectionHandleIn = NULL;
		}

		if (gInjectionHandleOut) {
			FwpsInjectionHandleDestroy(gInjectionHandleOut);
			gInjectionHandleOut = NULL;
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);

	return status;
}


VOID ThreadHandleSSLInjection(_In_ PVOID startContext) {
	PFLOWPACKET pFlowPacket = (PFLOWPACKET)startContext;
	FlowPacketAddRef(pFlowPacket);

	DebugTraceError("IN thread--- flow id[%ld]", pFlowPacket->flowId);

	do {
		PINJECTION_PACKET_SSL pInjectionPacket = CQueuePop(pFlowPacket->queueInjectionPackets);
		if (!pInjectionPacket) {
			if (pFlowPacket->stopThread) {
				break;
			}

			KeResetEvent(&pFlowPacket->eventQueue);
			KeWaitForSingleObject(&pFlowPacket->eventQueue, Executive, KernelMode, FALSE, NULL);
			continue;
		}

		pInjectionPacket->processId = pFlowPacket->processId;
		int result = HandleInjectionPacketSSL(pInjectionPacket);
		if (!result || result == 5) {
			pFlowPacket->stopThread = 1;
		}

		ExFreePoolWithTag(pInjectionPacket, 'eee');
	} while (1);

	FlowPacketDelete(pFlowPacket);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID ThreadOrganizeSSLFlows(_In_ PVOID StartContext) {
	DebugTrace("%s Entry", __FUNCTION__);

	ThreadHandleInjectionsSSLGoing = TRUE;

	do {
		PFLOWPACKET pInjectionPacket = CQueuePop(QueueSSLPackets);
		if (!pInjectionPacket) {
			KeResetEvent(&KEventNewInjectionPacketsSSL);
			KeWaitForSingleObject(&KEventNewInjectionPacketsSSL, Executive, KernelMode, FALSE, NULL);
			continue;
		}

		HANDLE hThread = NULL;
		PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, ThreadHandleSSLInjection, (void*)pInjectionPacket);
		ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&pInjectionPacket->hPkThread, NULL);
		ZwClose(hThread);
	} while (!ThreadHandleInjectionsSSLStop);

	ThreadHandleInjectionsSSLGoing = FALSE;
	PsTerminateSystemThread(STATUS_SUCCESS);

	DebugTrace("%s Exit", __FUNCTION__);
}

int HandleInjectionPacketSSL(PINJECTION_PACKET_SSL workItemContext) {
	int result = 0;
	char *redirectData = NULL;
	SIZE_T redirectDataSize = 0;
	NTSTATUS ntStatus;

	DebugTrace("%s Entry", __FUNCTION__);

	if (!workItemContext->data) {
		DebugTraceError("InjectClonedPacket---null data-- flow id[%ld]", workItemContext->flow_id);
		InjectClonedPacket(workItemContext->flow_id, workItemContext->layerId, workItemContext->streamFlags, SSLCalloutId, workItemContext->clonedNetBufferList, workItemContext->data_length, 0);
		return result;
	}

	if (workItemContext->streamFlags & FWPS_STREAM_FLAG_RECEIVE) {
		DebugTraceError("InjectClonedPacket---RECV-- flow id[%ld]", workItemContext->flow_id);
		InjectClonedPacket(workItemContext->flow_id, workItemContext->layerId, workItemContext->streamFlags, SSLCalloutId, workItemContext->clonedNetBufferList, workItemContext->data_length, 0);
		ExFreePoolWithTag(workItemContext->data, POOL_TAG_CALLOUT_STREAM);
		result = 1;
	}

	else {
		SocketConnection *sockConn = NULL;
		DebugTraceError("getting socket connection flow id[%ld]", workItemContext->flow_id);
		GetSocketConnection(&sockConn);

		if (sockConn && sockConn->socket) {
			int discardedData = 0;

			DebugTraceError("SendPacketCheckDestSSL flow id[%ld]", workItemContext->flow_id);
			ntStatus = SendPacketCheckDestSSL(sockConn->socket, workItemContext->flow_id, workItemContext->processId, workItemContext->data, workItemContext->data_length);
			ExFreePoolWithTag(workItemContext->data, POOL_TAG_CALLOUT_STREAM);
			if (!NT_SUCCESS(ntStatus)) {
				DebugTraceError("SendPacketCheckDestSSL failed");
				SetSocketConnectionNotInUse(sockConn);
				return 0;
			}

			
			do {
				DebugTraceError("RecvPacketCheckDestSSL flow id[%ld]", workItemContext->flow_id);
				ntStatus = RecvPacketCheckDestSSL(sockConn->socket, &result, &redirectData, &redirectDataSize);

				if (!NT_SUCCESS(ntStatus)) {
					DebugTraceError("RecvPacketCheckDestSSL failed");
					SetSocketConnectionNotInUse(sockConn);
					return 0;
				}

				if (result == -1) {
					DebugTraceError("error returned for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					if (workItemContext->clonedNetBufferList) {
						FwpsDiscardClonedStreamData(workItemContext->clonedNetBufferList, 0, FALSE);
						workItemContext->clonedNetBufferList = NULL;
					}
					break;
				}

				else if (result == 0) {
					//DebugTrace("injecting the cloned redirect SSL packet for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("InjectClonedPacket flow id[%ld]", workItemContext->flow_id);
					InjectClonedPacket(workItemContext->flow_id, workItemContext->layerId, workItemContext->streamFlags, SSLCalloutId, workItemContext->clonedNetBufferList, workItemContext->data_length, 0);
					
					break;
				}

				else if (result == 1) {
					//DebugTrace("injecting the SSL packet for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("InjectPacket1 flow id[%ld]", workItemContext->flow_id);
					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, redirectData, redirectDataSize, &workItemContext->clonedNetBufferList, 0);

					break;
				}

				else if (result == 2) {
					//DebugTrace("injecting the SSL packet for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("InjectPacket2 flow id[%ld]", workItemContext->flow_id);
					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, redirectData, redirectDataSize, &workItemContext->clonedNetBufferList, 0);
					
					continue;
				}

				else if (result == 3) {
					//DebugTrace("requesting more data for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("discarding flow id[%ld]", workItemContext->flow_id);
					FwpsDiscardClonedStreamData(workItemContext->clonedNetBufferList, 0, FALSE);
					break;
				}

				else if (result == 4) {
					//DebugTrace("injecting the LAST SSL packet for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("InjectPacket4 flow id[%ld]", workItemContext->flow_id);
					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, redirectData, redirectDataSize, &workItemContext->clonedNetBufferList, 0);
					
					break;
				}

				else if (result == 5) {
					//DebugTrace("injecting the LAST SSL packet + FIN for flow id[%llu] port[%d]...", workItemContext->flow_id, workItemContext->port);
					DebugTraceError("InjectPacket5 flow id[%ld]", workItemContext->flow_id);
					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, redirectData, redirectDataSize, &workItemContext->clonedNetBufferList, 0);
					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, NULL, 0, 
						NULL, FWPS_STREAM_FLAG_RECEIVE | FWPS_STREAM_FLAG_RECEIVE_PUSH | FWPS_STREAM_FLAG_RECEIVE_DISCONNECT);

					InjectPacket(workItemContext->flow_id, workItemContext->layerId, SSLCalloutId, NULL, 0,
						NULL, FWPS_STREAM_FLAG_SEND | FWPS_STREAM_FLAG_SEND_NODELAY | FWPS_STREAM_FLAG_SEND_DISCONNECT);

					break;
				}
			} while (1);

			SetSocketConnectionNotInUse(sockConn);
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);

	return result;
}

int HandleInjectionPacket(PWORKITEMHTTPPACKET workItemContext) {
	DebugTrace("%s Entry", __FUNCTION__);

	char *redirectUrl = NULL;
	int result = 0;

	if (workItemContext->url) {
		result = ShouldRedirect(workItemContext->url, &redirectUrl);
	}

	if (result > 0) {
		if (!redirectUrl) {
			DebugTraceError("the redirect url was NULL");
			return 0;
		}

		char *http = "HTTP/1.1 301 Moved Permanently\r\nLocation: \r\n\r\n";
		size_t httpLen1 = 0;
		if (!NT_SUCCESS(RtlStringCbLengthA(http, NTSTRSAFE_MAX_CCH * sizeof(char), &httpLen1))) {
			DebugTraceError("RtlStringCbLengthA failed");
			ExFreePoolWithTag(redirectUrl, POOL_TAG_BUILD_PACKET_DEST);
			return result;
		}

		size_t redirectUrlLen = 0;
		if (!NT_SUCCESS(RtlStringCbLengthA(redirectUrl, NTSTRSAFE_MAX_CCH * sizeof(char), &redirectUrlLen))) {
			DebugTraceError("RtlStringCbLengthA failed");
			ExFreePoolWithTag(redirectUrl, POOL_TAG_BUILD_PACKET_DEST);
			return result;
		}


		size_t httpLen = httpLen1 + redirectUrlLen + 1;
		char *http2 = ExAllocatePoolWithTag(NonPagedPool, httpLen, POOLTAGDRIVER);

		RtlStringCbPrintfA(http2, httpLen, "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\n\r\n", redirectUrl);
		ExFreePoolWithTag(redirectUrl, POOL_TAG_BUILD_PACKET_DEST);

		size_t http2Len = 0;
		if (!NT_SUCCESS(RtlStringCbLengthA(http2, NTSTRSAFE_MAX_CCH * sizeof(char), &http2Len))) {
			DebugTraceError("RtlStringCbLengthA failed");
			ExFreePoolWithTag(http2, POOLTAGDRIVER);
			return result;
		}

		InjectPacket(workItemContext->flowId, workItemContext->layerId, CalloutId, http2, http2Len, &workItemContext->clonedNetBufferList, 0);
	}

	else {
		InjectClonedPacket2(workItemContext);
	}

	DebugTrace("%s Exit", __FUNCTION__);

	return result;
}

void InjectPacket(UINT64 flowId, UINT16 layerId, UINT32 calloutId, char *dataToInject, size_t sizeDataToInject, NET_BUFFER_LIST **clonedNetBufferList, int streamFlags) {
	DebugTrace("%s Enter", __FUNCTION__);
	
	if (clonedNetBufferList) {
		for (NET_BUFFER_LIST* pCurrentNBL = *clonedNetBufferList; pCurrentNBL;) {
			NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

			FwpsDereferenceNetBufferList(pCurrentNBL, (KeGetCurrentIrql() == DISPATCH_LEVEL) ? TRUE : FALSE);

			pCurrentNBL = pNextNBL;
		}

		FwpsDiscardClonedStreamData(*clonedNetBufferList, 0, FALSE);
		*clonedNetBufferList = NULL;
	}

	if (!dataToInject) {
		NTSTATUS ntStatus = FwpsStreamInjectAsync(gInjectionHandleIn, NULL, 0, flowId, calloutId, layerId, streamFlags,
												NULL, 0, PacketInjectionCompletion, NULL);
		if (!NT_SUCCESS(ntStatus)) {
			DebugTraceError("FwpsStreamInjectAsync failed %x", ntStatus);
		}

		return;
	}
		
	MDL *mdl = IoAllocateMdl((PVOID)dataToInject, (ULONG)sizeDataToInject, FALSE, FALSE, 0);
	if (mdl) {
		NET_BUFFER_LIST *netBufferList;
		NTSTATUS ntStatus;

		MmBuildMdlForNonPagedPool(mdl);

		ntStatus = FwpsAllocateNetBufferAndNetBufferList(gNetBufferListPool, 0, 0, mdl, 0, sizeDataToInject, &netBufferList);
		if (NT_SUCCESS(ntStatus)) {
			if (!streamFlags) {
				streamFlags = FWPS_STREAM_FLAG_RECEIVE | FWPS_STREAM_FLAG_RECEIVE_PUSH;
			}
			
			PSTRUCTPACKETINJECTION pSpi = ExAllocatePoolWithTag(NonPagedPool, sizeof(STRUCTPACKETINJECTION), POOLTAGDRIVER);
			pSpi->dataToInject = dataToInject;
			pSpi->mdl = mdl;
			ntStatus = FwpsStreamInjectAsync(gInjectionHandleIn, NULL, 0, flowId, calloutId, layerId, streamFlags,
												netBufferList, sizeDataToInject, PacketInjectionCompletion, (HANDLE)pSpi);
			if (!NT_SUCCESS(ntStatus)) {
				DebugTraceError("FwpsStreamInjectAsync failed %x", ntStatus);
				FwpsFreeNetBufferList(netBufferList);
			}
		}

		else {
			DebugTraceError("FwpsAllocateNetBufferAndNetBufferList failed");
		}
	}

	else {
		DebugTraceError("making the MDL failed");
	}

	DebugTrace("%s exit", __FUNCTION__);
}

void InjectClonedPacket2(PWORKITEMHTTPPACKET pPacket) {
	NTSTATUS ntStatus;

	DebugTrace("%s Enter", __FUNCTION__);

	KIRQL irql;
	KeAcquireSpinLock(&pPacket->pFlowPacket->spinLock, &irql);
	if ((pPacket->pFlowPacket->gotInFin && (pPacket->streamFlags & FWPS_STREAM_FLAG_RECEIVE)) ||
		(pPacket->pFlowPacket->gotOutFin && (pPacket->streamFlags & FWPS_STREAM_FLAG_SEND))) {
		
		for (NET_BUFFER_LIST* pCurrentNBL = pPacket->clonedNetBufferList; pCurrentNBL;) {
			NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

			FwpsDereferenceNetBufferList(pCurrentNBL, (KeGetCurrentIrql() == DISPATCH_LEVEL) ? TRUE : FALSE);

			pCurrentNBL = pNextNBL;
		}

		FwpsFreeNetBufferList(pPacket->clonedNetBufferList);

		KeReleaseSpinLock(&pPacket->pFlowPacket->spinLock, irql);
		return;
	}

	if (pPacket->streamFlags & FWPS_CLASSIFY_OUT_FLAG_NO_MORE_DATA) {
		if (pPacket->streamFlags & FWPS_STREAM_FLAG_RECEIVE) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "END RECV flow id[%ld]\r\n", pPacket->pFlowPacket->flowId);
			pPacket->pFlowPacket->gotInFin = 1;
		}

		else  {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "END SEND flow id[%ld]\r\n", pPacket->pFlowPacket->flowId);
			pPacket->pFlowPacket->gotOutFin = 1;
		}
	}
	else {
		if ((pPacket->streamFlags & FWPS_STREAM_FLAG_RECEIVE_DISCONNECT) || (pPacket->streamFlags & FWPS_STREAM_FLAG_RECEIVE_ABORT)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "END RECV flow id[%ld]\r\n", pPacket->pFlowPacket->flowId);
			pPacket->pFlowPacket->gotInFin = 1;
		}

		if ((pPacket->streamFlags & FWPS_STREAM_FLAG_SEND_DISCONNECT) || (pPacket->streamFlags & FWPS_STREAM_FLAG_SEND_ABORT)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "END SEND flow id[%ld]\r\n", pPacket->pFlowPacket->flowId);
			pPacket->pFlowPacket->gotOutFin = 1;
		}
	}

	KeReleaseSpinLock(&pPacket->pFlowPacket->spinLock, irql);


	NDIS_HANDLE injectionHandle = gInjectionHandleIn;
	if (pPacket->streamFlags & FWPS_STREAM_FLAG_RECEIVE) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "INJECTING RECEIEVE flow id[%ld] layerId[%d] calloutId[%d] dataSize[%ld]\r\n",
			pPacket->pFlowPacket->flowId, pPacket->pFlowPacket->layerId, pPacket->pFlowPacket->calloutId, pPacket->dataLength);
		injectionHandle = gInjectionHandleOut;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "INJECTING SEND flow id[%ld] layerId[%d] calloutId[%d] dataSize[%ld]\r\n",
			pPacket->pFlowPacket->flowId, pPacket->pFlowPacket->layerId, pPacket->pFlowPacket->calloutId, pPacket->dataLength);
	}
	ntStatus = FwpsStreamInjectAsync(injectionHandle, NULL, 0, pPacket->pFlowPacket->flowId, pPacket->pFlowPacket->calloutId, pPacket->pFlowPacket->layerId, 
		pPacket->streamFlags, pPacket->clonedNetBufferList, pPacket->dataLength, PacketClonedInjectionCompletion2, NULL);

	if (!NT_SUCCESS(ntStatus)) {
		DebugTraceError("FwpsStreamInjectAsync flowId[%ld] failed %x", pPacket->pFlowPacket->flowId, ntStatus);
		/*for (NET_BUFFER_LIST* pCurrentNBL = pPacket->clonedNetBufferList; pCurrentNBL;) {
			NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

			FwpsDereferenceNetBufferList(pCurrentNBL, (KeGetCurrentIrql() == DISPATCH_LEVEL) ? TRUE : FALSE);

			pCurrentNBL = pNextNBL;
		}*/

		FwpsFreeNetBufferList(pPacket->clonedNetBufferList);
	}
	DebugTrace("%s exit", __FUNCTION__);
}

void InjectClonedPacket(UINT64 flowId, UINT16 layerId, UINT32 streamFlags, UINT32 calloutId, NET_BUFFER_LIST *clonedNetBufferList, size_t sizeDataToInject, int setFin) {
	NTSTATUS ntStatus;
	
	DebugTrace("%s Enter", __FUNCTION__);

	NDIS_HANDLE injectionHandle = gInjectionHandleIn;
	if (streamFlags & FWPS_STREAM_FLAG_RECEIVE) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "INJECTING RECEIEVE flow id[%ld] layerId[%d] calloutId[%d] dataSize[%ld]\r\n", flowId, layerId, calloutId, sizeDataToInject);
		injectionHandle = gInjectionHandleOut;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "INJECTING SEND flow id[%ld] layerId[%d] calloutId[%d] dataSize[%ld]\r\n", flowId, layerId, calloutId, sizeDataToInject);
	}

	
	ntStatus = FwpsStreamInjectAsync(injectionHandle, NULL, 0, flowId, calloutId, layerId, streamFlags,
									  clonedNetBufferList, sizeDataToInject, PacketClonedInjectionCompletion2, NULL);
	
	if (!NT_SUCCESS(ntStatus)) {
		DebugTraceError("FwpsStreamInjectAsync failed %x", ntStatus);
		FwpsFreeNetBufferList(clonedNetBufferList);
	}
	DebugTrace("%s exit", __FUNCTION__);
}

void NTAPI PacketClonedInjectionCompletion(
	_In_    VOID            *context,
	_Inout_ NET_BUFFER_LIST *clonedNetBufferList,
	_In_    BOOLEAN         dispatchLevel
	) {
	DebugTrace("%s Entry", __FUNCTION__);

	if (clonedNetBufferList) {
		FwpsFreeCloneNetBufferList(clonedNetBufferList, 0);
	}

	DebugTrace("%s Exit", __FUNCTION__);
}

void NTAPI PacketClonedInjectionCompletion2(
	_In_    VOID            *context,
	_Inout_ NET_BUFFER_LIST *clonedNetBufferList,
	_In_    BOOLEAN         dispatchLevel
	) {
	DebugTrace("%s Entry", __FUNCTION__);

	if (clonedNetBufferList) {
		FwpsDereferenceNetBufferList(clonedNetBufferList, dispatchLevel);
		FwpsFreeCloneNetBufferList(clonedNetBufferList, 0);
		
	}

	DebugTrace("%s Exit", __FUNCTION__);
}



void NTAPI PacketInjectionCompletion(
	_In_    VOID            *context,
	_Inout_ NET_BUFFER_LIST *netBufferList,
	_In_    BOOLEAN         dispatchLevel
	) {
	DebugTrace("%s Entry", __FUNCTION__);
	
	PSTRUCTPACKETINJECTION pSpi = NULL;
	if (context) {
		pSpi = (PSTRUCTPACKETINJECTION)context;
		if (pSpi->dataToInject) {
			ExFreePoolWithTag(pSpi->dataToInject, POOL_TAG_BUILD_PACKET_DEST);
		}

		if (pSpi->mdl) {
			IoFreeMdl(pSpi->mdl);
		}

		ExFreePoolWithTag(pSpi, POOLTAGDRIVER);
	}

	if (netBufferList) {
		FwpsFreeNetBufferList(netBufferList);
	}

	DebugTrace("%s Exit", __FUNCTION__);
}

//FWPM_LAYER_STREAM_V4
NTSTATUS RegisterCallout(PDEVICE_OBJECT deviceObject, 
						wchar_t *description, 
						wchar_t *name, 
						UINT32 *calloutId, 
						GUID layer, GUID calloutKey,
						FWPS_CALLOUT_CLASSIFY_FN classifyFn,
						FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteFn,
						FWPS_CALLOUT_NOTIFY_FN notifyFn) {
	FWPS_CALLOUT registerCallout;
	NTSTATUS status;

	DebugTrace("%s Entry", __FUNCTION__);

	memset(&registerCallout, 0, sizeof(registerCallout));
	registerCallout.calloutKey = calloutKey;
	registerCallout.classifyFn = classifyFn;
	registerCallout.flowDeleteFn = flowDeleteFn;
	registerCallout.notifyFn = notifyFn;

	*calloutId = 0;
	status = FwpsCalloutRegister(deviceObject, &registerCallout, calloutId);
	if (!NT_SUCCESS(status)) {
		DebugTrace("FwpsCalloutRegister failed");
		goto Exit;
	}

	FWPM_CALLOUT addCallout;
	memset(&addCallout, 0, sizeof(addCallout));
	addCallout.calloutKey = calloutKey;
	addCallout.displayData.description = description;
	addCallout.displayData.name = name;
	addCallout.calloutId = *calloutId;
	addCallout.applicableLayer = layer;

	status = FwpmCalloutAdd(hFilterEngine, &addCallout, NULL, NULL);
	if (!NT_SUCCESS(status)) {
		DebugTrace("FwpmCalloutAdd failed");
		goto Exit;
	}
	
	DebugTrace("%s Exit", __FUNCTION__);
Exit:
	return status;
}

NTSTATUS RegisterCalloutFilter(GUID filterKey, GUID layer, GUID subLayer, GUID calloutKey,
						wchar_t *displayName, wchar_t *displayDescription,
						FWPM_FILTER_CONDITION *filterConditions, UINT32 numFilterConditions) {
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_FILTER filter;

	DebugTrace("%s Entry", __FUNCTION__);

	memset(&filter, 0, sizeof(filter));
	filter.filterKey = filterKey;
	filter.displayData.name = displayName;
	filter.displayData.description = displayDescription;
	filter.layerKey = layer;
	filter.subLayerKey = subLayer;
	filter.weight.type = FWP_EMPTY;
	filter.numFilterConditions = numFilterConditions;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = calloutKey;
	if (filterConditions) {
		filter.filterCondition = filterConditions;
	}

	status = FwpmFilterAdd(hFilterEngine, &filter, NULL, NULL);
	if (!NT_SUCCESS(status)) {
		DebugTraceError("FwpmFilterAdd failed [%#x]", status);
		return status;
	}

	DebugTrace("%s Exit", __FUNCTION__);

	return status;
}


NTSTATUS RegisterDevice(WDFDRIVER driver, PDEVICE_OBJECT *deviceObject) {
	NTSTATUS status = STATUS_SUCCESS;

	DebugTrace("%s Entry", __FUNCTION__);

	PWDFDEVICE_INIT deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_KERNEL_ONLY);
	if (!deviceInit) {
		goto Exit;
	}

	WdfDeviceInitSetCharacteristics(deviceInit, FILE_DEVICE_SECURE_OPEN, FALSE);
	WdfDeviceInitSetDeviceType(deviceInit, FILE_DEVICE_NETWORK);
	WdfDeviceInitSetCharacteristics(deviceInit, FILE_AUTOGENERATED_DEVICE_NAME, TRUE);


	status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &wdfDevice);
	if (!NT_SUCCESS(status)) {
		goto Exit;
	}



	WdfControlFinishInitializing(wdfDevice);

	*deviceObject = WdfDeviceWdmGetDeviceObject(wdfDevice);
	if (!*deviceObject) {
		status = STATUS_UNSUCCESSFUL;
		goto Exit;
	}

	Exit:
	DebugTrace("%s Exit", __FUNCTION__);

	return status;
}

void FlowPacketAddRef(PFLOWPACKET pFlowPacket) {
	KIRQL irql;
	KeAcquireSpinLock(&pFlowPacket->spinLock, &irql);
	++pFlowPacket->refs;
	KeReleaseSpinLock(&pFlowPacket->spinLock, irql);
}

void FlowPacketDeleteSSL(PFLOWPACKET pFlowPacket) {
	KIRQL irql;
	int d = 0;
	KeAcquireSpinLock(&pFlowPacket->spinLock, &irql);
	--pFlowPacket->refs;
	if (!pFlowPacket->refs) {
		d = 1;
	}
	KeReleaseSpinLock(&pFlowPacket->spinLock, irql);

	if (!d) {
		return;
	}

	CQueueFlush(pFlowPacket->queueInjectionPackets);
	MHashMapRemoveByKey(PHashMapSSLFlows, pFlowPacket->flowId);
	ExFreePoolWithTag(pFlowPacket, 'pfp');
}

void FlowPacketDelete(PFLOWPACKET pFlowPacket) {
	KIRQL irql;
	int d = 0;
	KeAcquireSpinLock(&pFlowPacket->spinLock, &irql);
	--pFlowPacket->refs;
	if (!pFlowPacket->refs) {
		d = 1;
	}
	KeReleaseSpinLock(&pFlowPacket->spinLock, irql);

	if (!d) {
		return;
	}

	//CQueueFlush(pFlowPacket->queueInjectionPackets);
	MHashMapRemoveByKey(PHashMapHTTPFlows, pFlowPacket->flowId);
	ExFreeToNPagedLookasideList(&HttpFlowLookList, pFlowPacket);
}

void UnloadSSLFlows() {
	DebugTrace("%s Enter", __FUNCTION__);

	PMHASHMAPITERATOR i = MHashMapGetIterator(PHashMapSSLFlows);
	if (i) {
		do {
			PMHASHMAPELEMENT e = MHashMapIteratorNext(i);
			if (!e) {
				MHashMapFreeIterator(i);
				break;
			}

			PFLOWPACKET pFlowPacket = (PFLOWPACKET)e->data;

			DebugTraceError("removing SSL flow id[%ld]", pFlowPacket->flowId);
			FwpsFlowRemoveContext(pFlowPacket->flowId, FWPS_LAYER_STREAM_V4, SSLCalloutId);
		} while (1);
	}

	DebugTrace("%s Exit", __FUNCTION__);
}


void UnloadHTTPFlows() {
	DebugTrace("%s Enter", __FUNCTION__);
	PMHASHMAPITERATOR i = MHashMapGetIterator(PHashMapHTTPFlows);
	if (i)
	{
		do {
			PMHASHMAPELEMENT e = MHashMapIteratorNext(i);
			if (!e) {
				MHashMapFreeIterator(i);
				break;
			}

			PFLOWPACKET pFlowPacket = (PFLOWPACKET)e->data;

			//MHashMapFreeIterator(i);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "removing flow [%ld]...\r\n", pFlowPacket->flowId);
			FwpsFlowRemoveContext(pFlowPacket->flowId, FWPS_LAYER_STREAM_V4, CalloutId);
		} while (1);
	}
	DebugTrace("%s Exit", __FUNCTION__);
}

void CleanUp() {
	DebugTrace("%s Entry", __FUNCTION__);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "cleaning up...\r\n");

	IsDriverUnloading = TRUE;

	while (CountHttpWorkItems > 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CountHttpWorkItems [%ld]...\r\n", CountHttpWorkItems);
		LARGE_INTEGER li;
		li.QuadPart = RELATIVE(SECONDS(1));
		KeDelayExecutionThread(KernelMode, FALSE, &li);
	}

	ThreadHandleInjectionsSSLStop = TRUE;
	if (HThreadSSLFlows) {
		DebugTraceError("waiting till injection handler SSL thread exits...");
		ThreadHandleInjectionsSSLStop = TRUE;
		KeSetEvent(&KEventNewInjectionPacketsSSL, 0, FALSE);
		KeWaitForSingleObject(HThreadSSLFlows, Executive, KernelMode, FALSE, NULL);
	}

	
	DebugTraceError("CloseSockets");
	CloseSockets();

	DebugTraceError("UnloadSockets");
	UnloadSockets();
	ExDeleteNPagedLookasideList(&HttpFlowLookList);

	DebugTraceError("UnloadSSLConnectCallout");
	UnloadSSLConnectCallout();

	DebugTraceError("UnloadSSLCallout");
	UnloadSSLCallout();

	DebugTraceError("UnloadCallout");
	UnloadCallout();

	DebugTraceError("flush http flows");
	MHashMapFlush(PHashMapHTTPFlows);

	DebugTraceError("flush ssl flows");
	MHashMapFlush(PHashMapSSLFlows);

	DebugTraceError("flush queue ssl packets");
	CQueueFlush(QueueSSLPackets);
	
	NTSTATUS status = FwpmSubLayerDeleteByKey(hFilterEngine, &GuidHttpSubLayer);
	if (!NT_SUCCESS(status)) {
		DebugTraceError("FwpmSubLayerDeleteByKey0 failed[%x]", status);
	}

	status = FwpmProviderDeleteByKey(hFilterEngine, &GuidProvider);
	if (!NT_SUCCESS(status)) {
		DebugTraceError("FwpmProviderDeleteByKey0 failed[%x]", status);
	}

	if (gInjectionHandleIn) FwpsInjectionHandleDestroy(gInjectionHandleIn);
	if (gInjectionHandleOut) FwpsInjectionHandleDestroy(gInjectionHandleOut);

	if (gNetBufferListPool) NdisFreeNetBufferListPool(gNetBufferListPool);

	if (gNdisGenericObj) NdisFreeGenericObject(gNdisGenericObj);

	if (hFilterEngine) FwpmEngineClose(hFilterEngine);

	DebugTrace("%s Exit", __FUNCTION__);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "driver unloaded\r\n");
}

_Function_class_(EVT_WDF_DRIVER_UNLOAD)
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID Unload(IN WDFDRIVER Driver) {
	DebugTrace("%s Entry", __FUNCTION__);
	CleanUp();
	DebugTrace("%s Exit", __FUNCTION__);
}


void UnloadCallout() {
	NTSTATUS status;
	
	DebugTrace("%s Entry", __FUNCTION__);
	if (CalloutId) {
		for (;;) {
			status = FwpsCalloutUnregisterByKey(&GuidCallOut);
			if (!NT_SUCCESS(status)) {
				if (status == STATUS_DEVICE_BUSY) {
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpsCalloutUnregisterByKey failed it's busy\r\n");
					UnloadHTTPFlows();
					continue;
				}
				else {
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpsCalloutUnregisterByKey failed[%x]\r\n", status);
					break;
				}
				//return;
			}
			else {
				break;
			}
		}

		status = FwpmFilterDeleteByKey(hFilterEngine, &GuidCalloutFilter);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmFilterDeleteByKey failed %!STATUS!", status);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpmFilterDeleteByKey failed\r\n");
		}

		status = FwpmCalloutDeleteByKey(hFilterEngine, &GuidCallOut);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmCalloutDeleteByKey failed %!STATUS!", status);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpmCalloutDeleteByKey failed\r\n");
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);
}

void UnloadSSLCallout() {
	NTSTATUS status;
	
	DebugTrace("%s Entry", __FUNCTION__);

	if (SSLCalloutId) {
		for (;;) {
			status = FwpsCalloutUnregisterByKey(&GuidSSLCallOut);
			if (!NT_SUCCESS(status)) {
				if (status == STATUS_DEVICE_BUSY) {
					DebugTraceError("FwpsCalloutUnregisterByKey SSL failed, it's busy");
					UnloadSSLFlows();
					continue;
				}
				else {
					DebugTraceError("FwpsCalloutUnregisterByKey SSL failed %x", status);
					break;
				}
				//return;
			}
			else {
				break;
			}
		}

		status = FwpmFilterDeleteByKey(hFilterEngine, &GuidSSLCalloutFilter);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmFilterDeleteByKey SSL failed %d", status);
			//return;
		}
		
		status = FwpmCalloutDeleteByKey(hFilterEngine, &GuidSSLCallOut);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmCalloutDeleteByKey SSL failed %d", status);
			//return;
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);
}


#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
SSLClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  context,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
)

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI SSLClassifyFn(
	_In_     const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_     const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_        void                           *layerData,
	_In_opt_ const void                           *classifyContext,
	_In_     const FWPS_FILTER1                   *filter,
	_In_           UINT64                         context,
	_Inout_        FWPS_CLASSIFY_OUT             *classifyOut
	)

#else

void NTAPI SSLClassifyFn(
	_In_        const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         context,
	_Inout_           FWPS_CLASSIFY_OUT             *classifyOut
	)

#endif
{
	DebugTrace("%s Entry", __FUNCTION__);

	classifyOut->actionType = FWP_ACTION_PERMIT;

	FWPS_STREAM_CALLOUT_IO_PACKET *calloutPacket = (FWPS_STREAM_CALLOUT_IO_PACKET*)layerData;
	calloutPacket->streamAction = FWPS_STREAM_ACTION_NONE;
	calloutPacket->countBytesRequired = 0;
	calloutPacket->countBytesEnforced = calloutPacket->streamData->dataLength;

	if (IsDriverUnloading) {
		if (!context) {
			return;
		}

		classifyOut->actionType = FWP_ACTION_BLOCK;

		return;
	}

	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0) {
		DebugTraceError("no write rights");
		DebugTrace("%s Exit", __FUNCTION__);
		return;
	}

	if (!context) {
		//DebugTrace("the context was null");
		//DebugTrace("%s Exit", __FUNCTION__);
		return;
	}

	PFLOWPACKET pFlowPacket = (PFLOWPACKET)context;
	if (pFlowPacket->stopThread) {
		for (NET_BUFFER_LIST* pCurrentNBL = calloutPacket->streamData->netBufferListChain; pCurrentNBL;) {
			NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

			FwpsReferenceNetBufferList(pCurrentNBL, TRUE);

			pCurrentNBL = pNextNBL;
		}

		NET_BUFFER_LIST *clonedNetBufferList = NULL;
		FwpsCloneStreamData(calloutPacket->streamData, NULL, NULL, 0, &clonedNetBufferList);


		NTSTATUS ntStatus = FwpsStreamInjectAsync(gInjectionHandleIn, NULL, 0,
			inMetaValues->flowHandle, SSLCalloutId, inFixedValues->layerId, calloutPacket->streamData->flags,
			clonedNetBufferList, calloutPacket->streamData->dataLength, PacketClonedInjectionCompletion2, NULL);

		classifyOut->actionType = FWP_ACTION_BLOCK;

		DebugTrace("%s Exit", __FUNCTION__);
		return;
	}

	SIZE_T streamLength = calloutPacket->streamData->dataLength;
	char* stream = NULL;
	if (streamLength) {
		stream = (char*)ExAllocatePoolWithTag(NonPagedPool, streamLength, POOL_TAG_CALLOUT_STREAM);
		if (!stream) {
			DebugTraceError("ExAllocatePoolWithTag failed trying to allocate the data stream for the SSL bytes.");
			DebugTrace("%s Exit", __FUNCTION__);
			return;
		}

		RtlZeroMemory(stream, streamLength);
		SIZE_T streamBytesCopied = 0;
		FwpsCopyStreamDataToBuffer(calloutPacket->streamData, stream, streamLength, &streamBytesCopied);
	}

	for (NET_BUFFER_LIST* pCurrentNBL = calloutPacket->streamData->netBufferListChain; pCurrentNBL;) {
		NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

		FwpsReferenceNetBufferList(pCurrentNBL, TRUE);

		pCurrentNBL = pNextNBL;
	}

	NET_BUFFER_LIST *clonedNetBufferList = NULL;
	FwpsCloneStreamData(calloutPacket->streamData, NULL, NULL, 0, &clonedNetBufferList);

	PINJECTION_PACKET_SSL pInjectionPacket = (PINJECTION_PACKET_SSL)ExAllocatePoolWithTag(NonPagedPool, sizeof(INJECTION_PACKET_SSL), 'eee');
	pInjectionPacket->data_length = streamLength;
	pInjectionPacket->data = stream;
	pInjectionPacket->flow_id = inMetaValues->flowHandle;
	pInjectionPacket->port = inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_LOCAL_PORT].value.uint16;
	pInjectionPacket->layerId = inFixedValues->layerId;
	pInjectionPacket->clonedNetBufferList = clonedNetBufferList;
	pInjectionPacket->remoteIpAddress = inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_LOCAL_ADDRESS].value.uint32;
	pInjectionPacket->streamFlags = calloutPacket->streamData->flags;

	CQueuePush(pFlowPacket->queueInjectionPackets, (void*)pInjectionPacket);
	KeSetEvent(&pFlowPacket->eventQueue, 0, FALSE);

	classifyOut->actionType = FWP_ACTION_BLOCK;

	DebugTrace("%s Exit", __FUNCTION__);
}

NTSTATUS NTAPI
SSLNotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
) {
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	//PAGED_CODE();

	return STATUS_SUCCESS;
}

VOID NTAPI
SSLFlowDeleteFn(
IN UINT16  layerId,
IN UINT32  calloutId,
IN UINT64  flowContext
) {
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);

	//DebugTrace("%s Enter", __FUNCTION__);

	if (flowContext) {
		PFLOWPACKET pFlowPacket = (PFLOWPACKET)flowContext;
		
		pFlowPacket->stopThread = 1;
		KeSetEvent(&pFlowPacket->eventQueue, 0, FALSE);
		if (pFlowPacket->hPkThread) {
			KeWaitForSingleObject(pFlowPacket->hPkThread, Executive, KernelMode, FALSE, NULL);
			ObDereferenceObject(pFlowPacket->hPkThread);
		}

		FlowPacketDeleteSSL(pFlowPacket);
	}
	
	//DebugTrace("%s Exit", __FUNCTION__);
}
























#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
ClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  flowContext,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
)

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI ClassifyFn(
_In_     const FWPS_INCOMING_VALUES          *inFixedValues,
_In_     const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
_Inout_        void                           *layerData,
_In_opt_ const void                           *classifyContext,
_In_     const FWPS_FILTER1                   *filter,
_In_           UINT64                         flowContext,
_Inout_        FWPS_CLASSIFY_OUT             *classifyOut
)

#else

void NTAPI ClassifyFn(
	_In_        const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         flowContext,
	_Inout_           FWPS_CLASSIFY_OUT             *classifyOut
	)

#endif

{

	//DebugTrace("%s Entry", __FUNCTION__);
	classifyOut->actionType = FWP_ACTION_PERMIT;

	FWPS_STREAM_CALLOUT_IO_PACKET *calloutPacket = (FWPS_STREAM_CALLOUT_IO_PACKET*)layerData;
	calloutPacket->streamAction = FWPS_STREAM_ACTION_NONE;
	calloutPacket->countBytesRequired = 0;
	calloutPacket->countBytesEnforced = calloutPacket->streamData->dataLength;

	if (IsDriverUnloading) {
		if (!flowContext) {
			return;
		}

		classifyOut->actionType = FWP_ACTION_BLOCK;
		return;
	}
	/*
	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0) {
		//DebugTrace("%s Exit", __FUNCTION__);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		return;
	}*/

	
	if (classifyOut->flags & FWPS_CLASSIFY_OUT_FLAG_BUFFER_LIMIT_REACHED) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BUFFER LIMIT REACHED flow id[%ld]\r\n", inMetaValues->flowHandle);
		classifyOut->actionType = FWP_ACTION_BLOCK;
		return;
	}

	PFLOWPACKET pFlowPacket = NULL;
	if (!flowContext) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "NEW flow id[%ld]\r\n", inMetaValues->flowHandle);
		pFlowPacket = (PFLOWPACKET)ExAllocateFromNPagedLookasideList(&HttpFlowLookList);
		if (!pFlowPacket) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ExAllocateFromNPagedLookasideList failed flow id[%ld]\r\n", inMetaValues->flowHandle);
			calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
			classifyOut->actionType = FWP_ACTION_NONE;
			return;
		}

		pFlowPacket->flowId = inMetaValues->flowHandle;
		pFlowPacket->permitPacket = 0;
		pFlowPacket->foundHeader = 0;
		pFlowPacket->dataLength = -1;
		pFlowPacket->refs = 1;
		pFlowPacket->gotInFin = 0;
		pFlowPacket->gotOutFin = 0;
		pFlowPacket->calloutId = CalloutId;
		pFlowPacket->layerId = inFixedValues->layerId;
		KeInitializeEvent(&pFlowPacket->eventQueue, SynchronizationEvent, FALSE);
		KeInitializeSpinLock(&pFlowPacket->spinLock);

		NTSTATUS ntStatus = FwpsFlowAssociateContext(pFlowPacket->flowId, FWPS_LAYER_STREAM_V4, CalloutId, (UINT64)pFlowPacket);
		if (!NT_SUCCESS(ntStatus)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpsFlowAssociateContext failed flow id[%ld]\r\n", inMetaValues->flowHandle);

			ExFreeToNPagedLookasideList(&HttpFlowLookList, pFlowPacket);
			calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
			classifyOut->actionType = FWP_ACTION_NONE;
			return;
		}

		MHashMapInsert(PHashMapHTTPFlows, pFlowPacket->flowId, pFlowPacket);
	}
	else {
		pFlowPacket = (PFLOWPACKET)flowContext;
	}

	if (pFlowPacket->permitPacket) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PERMITING flow id[%ld]\r\n", inMetaValues->flowHandle);
		return;
	}

	if (pFlowPacket->foundHeader && (calloutPacket->streamData->flags & FWPS_STREAM_FLAG_SEND) && !(classifyOut->flags & FWPS_CLASSIFY_OUT_FLAG_NO_MORE_DATA)) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "NEW FLOW? flow id[%ld]\r\n", inMetaValues->flowHandle);
		pFlowPacket->foundHeader = 0;
		pFlowPacket->dataLength = -1;
		pFlowPacket->permitPacket = 0;
	}

	if (pFlowPacket->foundHeader || (calloutPacket->streamData->flags & FWPS_STREAM_FLAG_RECEIVE)) {
		for (NET_BUFFER_LIST* pCurrentNBL = calloutPacket->streamData->netBufferListChain; pCurrentNBL;) {
			NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

			FwpsReferenceNetBufferList(pCurrentNBL, TRUE);

			pCurrentNBL = pNextNBL;
		}

		NET_BUFFER_LIST *clonedNetBufferList = NULL;
		NTSTATUS s = FwpsCloneStreamData(calloutPacket->streamData, gNetBufferListPool, gNetBufferPool, 0, &clonedNetBufferList);
		if (!NT_SUCCESS(s)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpsCloneStreamData failed flow id[%ld]\r\n", inMetaValues->flowHandle);
			classifyOut->actionType = FWP_ACTION_BLOCK;
			return;
		}

		PIO_WORKITEM  pWorkItem = IoAllocateWorkItem(deviceObject);
		if (!pWorkItem) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR: IoAllocateWorkItem failed, flow id[%ld]\r\n", inMetaValues->flowHandle);
			classifyOut->actionType = FWP_ACTION_BLOCK;
			return;
		}

		PWORKITEMHTTPPACKET pWorkItemContext = (PWORKITEMHTTPPACKET)ExAllocatePoolWithTag(NonPagedPool, sizeof(WORKITEMHTTPPACKET), POOLTAGDRIVER);
		if (!pWorkItemContext) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR: ExAllocatePoolWithTag failed, flow id[%ld]\r\n", inMetaValues->flowHandle);
			classifyOut->actionType = FWP_ACTION_BLOCK;
			return;
		}

		pWorkItemContext->data = NULL;
		pWorkItemContext->dataLength = calloutPacket->streamData->dataLength;
		pWorkItemContext->flowId = inMetaValues->flowHandle;
		pWorkItemContext->layerId = inFixedValues->layerId;
		pWorkItemContext->clonedNetBufferList = clonedNetBufferList;
		pWorkItemContext->streamFlags = calloutPacket->streamData->flags;
		pWorkItemContext->pWorkItem = pWorkItem;
		pWorkItemContext->pFlowPacket = pFlowPacket;


		FlowPacketAddRef(pFlowPacket);

		IoQueueWorkItem(pWorkItem, WorkItemHttpPacket, DelayedWorkQueue, (PVOID)pWorkItemContext);

		classifyOut->actionType = FWP_ACTION_BLOCK;
		//classifyOut->flags |= FWPS_CLASSIFY_OUT_FLAG_ABSORB;
		return;
	}

	if (calloutPacket->streamData->dataLength < 3) {
		if ((classifyOut->flags & FWPS_CLASSIFY_OUT_FLAG_NO_MORE_DATA) || pFlowPacket->dataLength == calloutPacket->streamData->dataLength) {
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "NO HEADER FOUND flow id[%ld] length[%ld]\r\n", pFlowPacket->flowId, calloutPacket->streamData->dataLength);
			//calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
			//classifyOut->actionType = FWP_ACTION_NONE;
			return;
		}

		pFlowPacket->dataLength = (int)calloutPacket->streamData->dataLength;
		calloutPacket->streamAction = FWPS_STREAM_ACTION_NEED_MORE_DATA;
		calloutPacket->countBytesRequired = (UINT32)calloutPacket->streamData->dataLength + 1;
		calloutPacket->countBytesEnforced = 0;
		//classifyOut->actionType = FWP_ACTION_NONE;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FWPS_STREAM_ACTION_NEED_MORE_DATA flow id[%ld] length[%ld]\r\n", pFlowPacket->flowId, calloutPacket->streamData->dataLength);
		return;
	}

	char* stream = (char*)ExAllocatePoolWithTag(NonPagedPool, calloutPacket->streamData->dataLength, POOL_TAG_CALLOUT_STREAM);
	if (!stream) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR exalloate stream failed flow id[%ld]\r\n", pFlowPacket->flowId);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		return;
	}

	SIZE_T streamBytesCopied = 0;
	FwpsCopyStreamDataToBuffer(calloutPacket->streamData, stream, calloutPacket->streamData->dataLength, &streamBytesCopied);
	if (streamBytesCopied != calloutPacket->streamData->dataLength) {
		DebugTrace("streamBytesCopied != streamLength");
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR streambytes != streamlength flow id[%ld]\r\n", pFlowPacket->flowId);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
		return;
	}

	if (!(stream[0] == 'G' && stream[1] == 'E' && stream[2] == 'T')) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR get not found flow id[%ld]\r\n", pFlowPacket->flowId);
		
		pFlowPacket->permitPacket = 1;
		ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
		return;
	}

	for (int loopa = ((int)streamBytesCopied) - 1; loopa >= 0; loopa--) {
		if ((loopa - 3) < 0) {
			if (pFlowPacket->dataLength == streamBytesCopied) {
				calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
				classifyOut->actionType = FWP_ACTION_NONE;
				ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
				return;
			}

			pFlowPacket->dataLength = (int)streamBytesCopied;
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR need more data flow id[%ld]\r\n", pFlowPacket->flowId);
			calloutPacket->streamAction = FWPS_STREAM_ACTION_NEED_MORE_DATA;
			calloutPacket->countBytesRequired = (UINT32)streamBytesCopied + 1;
			classifyOut->actionType = FWP_ACTION_NONE;
			ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
			return;
		}

		if (stream[loopa] == '\n') {
			if (stream[loopa - 1] == '\r') {
				if (stream[loopa - 2] == '\n') {
					if (stream[loopa - 3] == '\r') {
						pFlowPacket->foundHeader = 1;
						break;
					}
				}
			}
		}
	}

	if (!pFlowPacket->foundHeader) {
		if (pFlowPacket->dataLength == streamBytesCopied) {
			calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
			classifyOut->actionType = FWP_ACTION_NONE;
			ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
			return;
		}

		pFlowPacket->dataLength = (int)streamBytesCopied;
		DebugTrace("header not found, need more data");
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR header not found, need more data flow id[%ld]\r\n", pFlowPacket->flowId);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_NEED_MORE_DATA;
		classifyOut->actionType = FWP_ACTION_NONE;
		calloutPacket->countBytesRequired = (UINT32)streamBytesCopied + 1;
		ExFreePoolWithTag(stream, POOL_TAG_CALLOUT_STREAM);
		return;
	}


	
	for (NET_BUFFER_LIST* pCurrentNBL = calloutPacket->streamData->netBufferListChain; pCurrentNBL;) {
		NET_BUFFER_LIST* pNextNBL = NET_BUFFER_LIST_NEXT_NBL(pCurrentNBL);

		FwpsReferenceNetBufferList(pCurrentNBL, TRUE);

		pCurrentNBL = pNextNBL;
	}


	NET_BUFFER_LIST *clonedNetBufferList = NULL;
	NTSTATUS s = FwpsCloneStreamData(calloutPacket->streamData, gNetBufferListPool, gNetBufferPool, 0, &clonedNetBufferList);
	if (!NT_SUCCESS(s)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FwpsCloneStreamData failed flow id[%ld]\r\n", inMetaValues->flowHandle);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		return;
	}

	PIO_WORKITEM  pWorkItem = IoAllocateWorkItem(deviceObject);
	if (!pWorkItem) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR: IoAllocateWorkItem failed, flow id[%ld]\r\n", inMetaValues->flowHandle);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		return;
	}

	PWORKITEMHTTPPACKET pWorkItemContext = (PWORKITEMHTTPPACKET)ExAllocatePoolWithTag(NonPagedPool, sizeof(WORKITEMHTTPPACKET), POOLTAGDRIVER);
	if (!pWorkItemContext) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR: ExAllocatePoolWithTag failed, flow id[%ld]\r\n", inMetaValues->flowHandle);
		calloutPacket->streamAction = FWPS_STREAM_ACTION_ALLOW_CONNECTION;
		classifyOut->actionType = FWP_ACTION_NONE;
		return;
	}

	pWorkItemContext->data = stream;
	pWorkItemContext->dataLength = calloutPacket->streamData->dataLength;
	pWorkItemContext->flowId = inMetaValues->flowHandle;
	pWorkItemContext->layerId = inFixedValues->layerId;
	pWorkItemContext->clonedNetBufferList = clonedNetBufferList;
	pWorkItemContext->streamFlags = calloutPacket->streamData->flags;
	pWorkItemContext->pWorkItem = pWorkItem;
	pWorkItemContext->pFlowPacket = pFlowPacket;
	FlowPacketAddRef(pFlowPacket);

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "HEADER FOUND flow id[%ld]\r\n", inMetaValues->flowHandle);
	IoQueueWorkItem(pWorkItem, WorkItemHttpPacket, DelayedWorkQueue, (PVOID)pWorkItemContext);
	KeSetEvent(&pFlowPacket->eventQueue, 0, FALSE);

	classifyOut->actionType = FWP_ACTION_BLOCK;
	//classifyOut->flags |= FWPS_CLASSIFY_OUT_FLAG_ABSORB;
	DebugTrace("%s Exit", __FUNCTION__);
	return;
}




VOID WorkItemHttpPacket(_In_  PDEVICE_OBJECT DeviceObject, _In_opt_ PVOID context) {
	if (!context) {
		return;
	}
	
	//int CountHttpWorkItemsGoing = 0;
	InterlockedIncrement(&CountHttpWorkItems);
	PWORKITEMHTTPPACKET pWorkItemContext = (PWORKITEMHTTPPACKET)context;
	IoFreeWorkItem(pWorkItemContext->pWorkItem);
	
	//KeWaitForSingleObject(&(pWorkItemContext->flowPacket->eventQueue), Executive, KernelMode, FALSE, NULL);

	pWorkItemContext->url = NULL;
	if (pWorkItemContext->data) {
		if (!GetUrlFromHttpPacket(pWorkItemContext->data, pWorkItemContext->dataLength, &pWorkItemContext->url)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ERROR: WorkItemHttpPacket  failed getting URL, flow id[%ld]\r\n", pWorkItemContext->flowId);
			ExFreePoolWithTag(pWorkItemContext->data, POOLTAGDRIVER);
			InterlockedDecrement(&CountHttpWorkItems);
			return;
		}

		ExFreePoolWithTag(pWorkItemContext->data, POOLTAGDRIVER);

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WorkItemHttpPacket flow id[%ld] url[%s]\r\n", pWorkItemContext->flowId, pWorkItemContext->url);
	}

	HandleInjectionPacket(pWorkItemContext);

	if (pWorkItemContext->url) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WorkItemHttpPacket RESULT flow id[%ld] url[%s]\r\n", pWorkItemContext->flowId, pWorkItemContext->url);
		ExFreePoolWithTag(pWorkItemContext->url, POOLTAGHTTPPARSER);
	}

	KeSetEvent(&(pWorkItemContext->pFlowPacket->eventQueue), 0, FALSE);
	FlowPacketDelete(pWorkItemContext->pFlowPacket);

	ExFreePoolWithTag(pWorkItemContext, POOLTAGDRIVER);
	InterlockedDecrement(&CountHttpWorkItems);
}























NTSTATUS NTAPI
NotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
) {
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	//PAGED_CODE();

	return STATUS_SUCCESS;
}

VOID NTAPI
FlowDeleteFn(
IN UINT16  layerId,
IN UINT32  calloutId,
IN UINT64  flowContext
) {
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);

	DebugTrace("%s Entry", __FUNCTION__);
	if (flowContext) {
		FlowPacketDelete((PFLOWPACKET)flowContext);
	}

	DebugTrace("%s Exit", __FUNCTION__);
}




















void UnloadSSLConnectCallout() {
	NTSTATUS status;

	DebugTrace("%s Entry", __FUNCTION__);

	if (SSLConnectCalloutId) {
		status = FwpsCalloutUnregisterByKey(&GuidSSLConnectCallOut);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpsCalloutUnregisterByKey failed");
			//return;
		}

		status = FwpmFilterDeleteByKey(hFilterEngine, &GuidSSLConnectCalloutFilter);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmFilterDeleteByKey failed");
		}

		status = FwpmCalloutDeleteByKey(hFilterEngine, &GuidSSLConnectCallOut);
		if (!NT_SUCCESS(status)) {
			DebugTrace("FwpmCalloutDeleteByKey failed");
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);
}

#if(NTDDI_VERSION < NTDDI_WIN7)

VOID NTAPI
SSLConnectClassifyFn(
IN const FWPS_INCOMING_VALUES  *inFixedValues,
IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
IN OUT VOID  *layerData,
IN const FWPS_FILTER  *filter,
IN UINT64  context,
IN OUT FWPS_CLASSIFY_OUT  *classifyOut
)

#elseif(NTDDI_VERSION == NTDDI_WIN7)

void NTAPI SSLConnectClassifyFn(
_In_     const FWPS_INCOMING_VALUES          *inFixedValues,
_In_     const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
_Inout_        void                           *layerData,
_In_opt_ const void                           *classifyContext,
_In_     const FWPS_FILTER1                   *filter,
_In_           UINT64                         context,
_Inout_        FWPS_CLASSIFY_OUT             *classifyOut
)

#else

void NTAPI SSLConnectClassifyFn(
	_In_        const FWPS_INCOMING_VALUES          *inFixedValues,
	_In_        const FWPS_INCOMING_METADATA_VALUES *inMetaValues,
	_Inout_opt_       void                           *layerData,
	_In_opt_    const void                           *classifyContext,
	_In_        const FWPS_FILTER2                   *filter,
	_In_              UINT64                         context,
	_Inout_           FWPS_CLASSIFY_OUT             *classifyOut
	)

#endif
{
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(filter);

	DebugTrace("%s Enter", __FUNCTION__);

	classifyOut->actionType = FWP_ACTION_PERMIT;

	if (IsDriverUnloading) {
		DebugTrace("%s Exit", __FUNCTION__);
		return;
	}
	if (!context) {
		DebugTraceError("new connecton---flow id[%ld]", inMetaValues->flowHandle);
		PFLOWPACKET pFlowPacket = (PFLOWPACKET)ExAllocatePoolWithTag(NonPagedPool, sizeof(FLOWPACKET), 'pfp');
		pFlowPacket->flowId = inMetaValues->flowHandle;
		pFlowPacket->stopThread = 0;
		pFlowPacket->queueInjectionPackets = CQueueCreate();
		pFlowPacket->processId = inMetaValues->processId;
		pFlowPacket->hPkThread = NULL;
		pFlowPacket->refs = 1;
		KeInitializeEvent(&(pFlowPacket->eventQueue), NotificationEvent, FALSE);
		KeInitializeSpinLock(&pFlowPacket->spinLock);

		MHashMapInsert(PHashMapSSLFlows, pFlowPacket->flowId, (void*)pFlowPacket);

		NTSTATUS ntStatus = FwpsFlowAssociateContext(pFlowPacket->flowId, FWPS_LAYER_STREAM_V4, SSLCalloutId, (UINT64)pFlowPacket);
		if (ntStatus != STATUS_SUCCESS) {
			DebugTraceError("FwpsFlowAssociateContext failed for SSL connect to stream later.");

			MHashMapRemoveByKey(PHashMapSSLFlows, pFlowPacket->flowId);
			ExFreePoolWithTag(pFlowPacket, 'pfp');
		}

		else {
			CQueuePush(QueueSSLPackets, (void*)pFlowPacket);
			KeSetEvent(&KEventNewInjectionPacketsSSL, 0, FALSE);
		}
	}

	DebugTrace("%s Exit", __FUNCTION__);
}

NTSTATUS NTAPI
SSLConnectNotifyFn(
IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
IN const GUID  *filterKey,
IN const FWPS_FILTER  *filter
) {
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	return STATUS_SUCCESS;
}

VOID NTAPI
SSLConnectFlowDeleteFn(IN UINT16  layerId, IN UINT32  calloutId, IN UINT64 flowContext) {
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);
}