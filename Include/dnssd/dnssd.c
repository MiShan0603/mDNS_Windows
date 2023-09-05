/**
 *  Copyright (C) 2011-2012  Juho Vähä-Herttua
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

/* These defines allow us to compile on iOS */ 
#ifndef __has_feature
# define __has_feature(x) 0
#endif
#ifndef __has_extension
# define __has_extension __has_feature
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
 
#include "hashmap.h"
#include "dnssd.h"    

#include "threads.h"

#define MAX_DEVICEID 18
#define MAX_SERVNAME 256

struct hashmap  dnssd_receiver_map;
thread_mutex_t  dnssd_map_mutex;

#define USE_LIBDL (defined(HAVE_LIBDL) && !defined(__APPLE__))

#if defined(WIN32) || USE_LIBDL
# ifdef WIN32
#  include <windows.h>
#  include <stdint.h>
#  if !defined(EFI32) && !defined(EFI64)
#   define DNSSD_STDCALL __stdcall
#  else
#   define DNSSD_STDCALL
#  endif
# else
#  include <dlfcn.h>
#  define DNSSD_STDCALL
# endif

typedef struct _DNSServiceRef_t *DNSServiceRef;
typedef union _TXTRecordRef_t { char PrivateData[16]; char *ForceNaturalAlignment; } TXTRecordRef;

typedef uint32_t DNSServiceFlags;
typedef uint32_t DNSServiceProtocol;
typedef int32_t  DNSServiceErrorType;
 
#define kDNSServiceFlagsShareConnection  0x4000
#define kDNSServiceInterfaceIndexAny	 0
#define kDNSServiceErr_NoError			 0

#define kDNSServiceFlagsAdd              0x2
#define kDNSServiceFlagsLongLivedQuery   0x100
#define kDNSServiceFlagsTimeout			 0x10000

#define kDNSServiceMaxDomainName		 1009
#define kDNSServiceProtocol_IPv4		 0x01

#else
# include <dns_sd.h>
# define DNSSD_STDCALL
#endif

typedef void (DNSSD_STDCALL *DNSServiceBrowseReply)
(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		inName,
	const char *		inType,
	const char *		inDomain,
	void *				inContext
	);

typedef void (DNSSD_STDCALL *DNSServiceResolveReply)
(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		fullname,
	const char *		hosttarget,
	uint16_t			opaqueport,
	uint16_t			txtLen,
	const unsigned char *txt,
	void *				context
);

//1 
typedef void (DNSSD_STDCALL *DNSServiceGetAddrInfoReply)
(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		hostname,
	const struct sockaddr *address,
	uint32_t			ttl,
	void *				context
);

//////
typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceCreateConnection_t)
(
	DNSServiceRef *sdRef
);

typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceBrowse_t)
(
	DNSServiceRef                       *sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	const char                          *regtype,
	const char                          *domain,    /* may be NULL */
	DNSServiceBrowseReply               callBack,
	void                                *context    /* may be NULL */
);

typedef int (DNSSD_STDCALL *DNSServiceRefSockFD_t)
(
	DNSServiceRef sdRef
);

typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceProcessResult_t)
(
	DNSServiceRef sdRef
);

typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceResolve_t)
(
	DNSServiceRef                       *sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	const char                          *name,
	const char                          *regtype,
	const char                          *domain,
	DNSServiceResolveReply              callBack,
	void                                *context  /* may be NULL */
);
 
typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceGetAddrInfo_t)
(
	DNSServiceRef                    *sdRef,
	DNSServiceFlags                  flags,
	uint32_t                         interfaceIndex,
	DNSServiceProtocol               protocol,
	const char                       *hostname,
	DNSServiceGetAddrInfoReply       callBack,
	void                             *context          /* may be NULL */
);

typedef void (DNSSD_STDCALL *DNSServiceRefDeallocate_t)(DNSServiceRef sdRef);


typedef void * (DNSSD_STDCALL *TXTRecordGetValuePtr_t)
(
	uint16_t         txtLen,
	const void       *txtRecord,
	const char       *key,
	uint8_t          *valueLen
);

typedef uint16_t(DNSSD_STDCALL* TXTRecordGetCount_t)
(
	uint16_t txtLen,
	const void* txtRecord
);

typedef DNSServiceErrorType(DNSSD_STDCALL* TXTRecordGetItemAtIndex_t)
(
	uint16_t txtLen,
	const void* txtRecord,
	uint16_t itemIndex,
	uint16_t keyBufLen,
	char* key,
	uint8_t* valueLen,
	const void** value
);


typedef void (DNSSD_STDCALL *DNSServiceQueryRecordReply)
(
	DNSServiceRef                       sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	DNSServiceErrorType                 errorCode,
	const char                          *fullname,
	uint16_t                            rrtype,
	uint16_t                            rrclass,
	uint16_t                            rdlen,
	const void                          *rdata,
	uint32_t                            ttl,
	void                                *context
	);

typedef DNSServiceErrorType (DNSSD_STDCALL *DNSServiceQueryRecord_t)
(
	DNSServiceRef                       *sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	const char                          *fullname,
	uint16_t                            rrtype,
	uint16_t                            rrclass,
	DNSServiceQueryRecordReply          callBack,
	void                                *context  /* may be NULL */
);


static void DNSSD_API DNSServiceBrowserCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		inName,
	const char *		inType,
	const char *		inDomain,
	void *				inContext);

static void DNSSD_API DNSServiceResolveCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		fullName,
	const char *		hostTarget,
	uint16_t			opaquePort,
	uint16_t			txtLen,
	const unsigned char *txt,
	void *				inContext);


static void DNSSD_API DNSServiceGetAddrInfoCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		hostname,
	const struct sockaddr *address,
	uint32_t			ttl,
	void *				inContext);


static void DNSSD_API DNSServiceQueryRecordCallback
(
	DNSServiceRef                       sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	DNSServiceErrorType                 errorCode,
	const char                          *fullname,
	uint16_t                            rrtype,
	uint16_t                            rrclass,
	uint16_t                            rdlen,
	const void                          *rdata,
	uint32_t                            ttl,
	void                                *context
);




//////////////////////////////////////////////////////////////////////////
typedef void (DNSSD_STDCALL *DNSServiceRegisterReply)
(
	DNSServiceRef                       sdRef,
	DNSServiceFlags                     flags,
	DNSServiceErrorType                 errorCode,
	const char                          *name,
	const char                          *regtype,
	const char                          *domain,
	void                                *context
	);

typedef DNSServiceErrorType(DNSSD_STDCALL *DNSServiceRegister_t)
(
	DNSServiceRef                       *sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	const char                          *name,
	const char                          *regtype,
	const char                          *domain,
	const char                          *host,
	uint16_t                            port,
	uint16_t                            txtLen,
	const void                          *txtRecord,
	DNSServiceRegisterReply             callBack,
	void                                *context
	);

typedef void (DNSSD_STDCALL *TXTRecordCreate_t)
(
	TXTRecordRef     *txtRecord,
	uint16_t         bufferLen,
	void             *buffer
	);

typedef void (DNSSD_STDCALL *TXTRecordDeallocate_t)(TXTRecordRef *txtRecord);
typedef DNSServiceErrorType(DNSSD_STDCALL *TXTRecordSetValue_t)
(
	TXTRecordRef     *txtRecord,
	const char       *key,
	uint8_t          valueSize,
	const void       *value
	);
typedef uint16_t(DNSSD_STDCALL *TXTRecordGetLength_t)(const TXTRecordRef *txtRecord);
typedef const void * (DNSSD_STDCALL *TXTRecordGetBytesPtr_t)(const TXTRecordRef *txtRecord);

static void __stdcall MyRegisterServiceReply
(
	DNSServiceRef                       sdRef,
	DNSServiceFlags                     flags,
	DNSServiceErrorType                 errorCode,
	const char* name,
	const char* regtype,
	const char* domain,
	void* context
)
{
}

struct dnssd_s {
#ifdef WIN32
	HMODULE module;
#elif USE_LIBDL
	void *module;
#endif

	DNSServiceRegister_t				DNSServiceRegister;
	TXTRecordCreate_t					TXTRecordCreate;
	TXTRecordSetValue_t					TXTRecordSetValue;
	TXTRecordGetLength_t				TXTRecordGetLength;
	TXTRecordGetBytesPtr_t				TXTRecordGetBytesPtr;
	TXTRecordDeallocate_t				TXTRecordDeallocate;

	


	DNSServiceCreateConnection_t       DNSServiceCreateConnection;
	DNSServiceBrowse_t				   DNSServiceBrowse;
	DNSServiceRefSockFD_t			   DNSServiceRefSockFD;
	DNSServiceRefDeallocate_t          DNSServiceRefDeallocate;
	DNSServiceResolve_t				   DNSServiceResolve;
	DNSServiceGetAddrInfo_t            DNSServiceGetAddrInfo;
	DNSServiceProcessResult_t		   DNSServiceProcessResult;


	DNSServiceQueryRecord_t				DNSServiceQueryRecord;
	TXTRecordGetValuePtr_t				TXTRecordGetValuePtr;
	TXTRecordGetCount_t					TXTRecordGetCount_;
	TXTRecordGetItemAtIndex_t			TXTRecordGetItemAtIndex_;
	
	DNSServiceRef					    mainSrvRef;
	DNSServiceRef						raopService;
	DNSServiceRef						airplayService;

	thread_handle_t					   thread;
	u32_t							   thread_running;	


	char								registerName[64];
	char								customName[64];
};


void * dnssd_thread(void *args);



// https://android.googlesource.com/platform/external/mdnsresponder/+/jb-dev/mDNSShared/dns_sd.h 

dnssd_t *dnssd_init_srv(int *error)
{
	dnssd_t *dnssd;

	if (error) *error = DNSSD_ERROR_NOERROR;

	dnssd = calloc(1, sizeof(dnssd_t));
	if (!dnssd) {
		if (error) *error = DNSSD_ERROR_OUTOFMEM;
		return NULL;
	}


	dnssd->module = LoadLibraryA("dnssd.dll");
	if (!dnssd->module) {
		if (error) *error = DNSSD_ERROR_LIBNOTFOUND;
		free(dnssd);
		return NULL;
	}
	dnssd->DNSServiceRegister = (DNSServiceRegister_t)GetProcAddress(dnssd->module, "DNSServiceRegister");
	dnssd->DNSServiceRefDeallocate = (DNSServiceRefDeallocate_t)GetProcAddress(dnssd->module, "DNSServiceRefDeallocate");
	dnssd->TXTRecordCreate = (TXTRecordCreate_t)GetProcAddress(dnssd->module, "TXTRecordCreate");
	dnssd->TXTRecordSetValue = (TXTRecordSetValue_t)GetProcAddress(dnssd->module, "TXTRecordSetValue");
	dnssd->TXTRecordGetLength = (TXTRecordGetLength_t)GetProcAddress(dnssd->module, "TXTRecordGetLength");
	dnssd->TXTRecordGetBytesPtr = (TXTRecordGetBytesPtr_t)GetProcAddress(dnssd->module, "TXTRecordGetBytesPtr");
	dnssd->TXTRecordDeallocate = (TXTRecordDeallocate_t)GetProcAddress(dnssd->module, "TXTRecordDeallocate");

	if (!dnssd->DNSServiceRegister || !dnssd->DNSServiceRefDeallocate || !dnssd->TXTRecordCreate ||
		!dnssd->TXTRecordSetValue || !dnssd->TXTRecordGetLength || !dnssd->TXTRecordGetBytesPtr ||
		!dnssd->TXTRecordDeallocate) {
		if (error) *error = DNSSD_ERROR_PROCNOTFOUND;
		FreeLibrary(dnssd->module);
		free(dnssd);
		return NULL;
	}

	return dnssd;
}

int utils_hwaddr_airplay(char *str, int strlen, const char *hwaddr, int hwaddrlen)
{
	int i, j;

	/* Check that our string is long enough */
	if (strlen == 0 || strlen < 2 * hwaddrlen + hwaddrlen)
		return -1;

	/* Convert hardware address to hex string */
	for (i = 0, j = 0; i < hwaddrlen; i++) {
		int hi = (hwaddr[i] >> 4) & 0x0f;
		int lo = hwaddr[i] & 0x0f;

		if (hi < 10) str[j++] = '0' + hi;
		else         str[j++] = 'a' + hi - 10;
		if (lo < 10) str[j++] = '0' + lo;
		else         str[j++] = 'a' + lo - 10;

		str[j++] = ':';
	}

	/* Add string terminator */
	if (j != 0) j--;
	str[j++] = '\0';
	return j;
}


#define MAX_HWADDR_LEN 6

int dnssd_register_server(dnssd_t* dnssd, const char* name, unsigned short port,
	const char* regtype, dnssd_record_kv_t* records, int recordsCount)
{
	TXTRecordRef txtRecord;
	int ret;

	assert(dnssd);
	assert(name);
	assert(regtype);


	dnssd->TXTRecordCreate(&txtRecord, 0, NULL);
	for (int recordIndex = 0; recordIndex < recordsCount && records; recordIndex++)
	{
		assert(records[recordIndex].key);
		assert(records[recordIndex].val);

		dnssd->TXTRecordSetValue(&txtRecord, records[recordIndex].key, strlen(records[recordIndex].val), records[recordIndex].val);
	}
	
	/* Register the service */
	ret = dnssd->DNSServiceRegister(&dnssd->airplayService, 0, 0,
		name,			// 
		regtype,		// eg. "_airplay._tcp",
		NULL, NULL,
		htons(port),
		dnssd->TXTRecordGetLength(&txtRecord),
		dnssd->TXTRecordGetBytesPtr(&txtRecord),
		NULL, NULL);

	/* Deallocate TXT record */
	dnssd->TXTRecordDeallocate(&txtRecord);

	return 0;
}

void dnssd_unregister_server(dnssd_t *dnssd)
{
	assert(dnssd);

	if (!dnssd->airplayService) {
		return;
	}

	dnssd->DNSServiceRefDeallocate(dnssd->airplayService);
	dnssd->airplayService = NULL;
}

dnssd_t *dnssd_init_client(int *error, const char* regtype)
{
	dnssd_t *dnssd;

	if (error) *error = DNSSD_ERROR_NOERROR;

	dnssd = calloc(1, sizeof(dnssd_t));
	if (!dnssd) {
		if (error) *error = DNSSD_ERROR_OUTOFMEM;
		return NULL;
	}

	dnssd->module = LoadLibraryA("dnssd.dll");
	if (!dnssd->module) {
		if (error) *error = DNSSD_ERROR_LIBNOTFOUND;
		free(dnssd);
		return NULL;
	}
	dnssd->DNSServiceCreateConnection = (DNSServiceCreateConnection_t)GetProcAddress(dnssd->module, "DNSServiceCreateConnection");
	dnssd->DNSServiceRefSockFD = (DNSServiceRefSockFD_t)GetProcAddress(dnssd->module, "DNSServiceRefSockFD");
	dnssd->DNSServiceBrowse = (DNSServiceBrowse_t)GetProcAddress(dnssd->module, "DNSServiceBrowse");
	dnssd->DNSServiceResolve = (DNSServiceResolve_t)GetProcAddress(dnssd->module, "DNSServiceResolve");
	dnssd->DNSServiceGetAddrInfo = (DNSServiceGetAddrInfo_t)GetProcAddress(dnssd->module, "DNSServiceGetAddrInfo");
	dnssd->DNSServiceRefDeallocate = (DNSServiceRefDeallocate_t)GetProcAddress(dnssd->module, "DNSServiceRefDeallocate");
	dnssd->DNSServiceProcessResult = (DNSServiceProcessResult_t)GetProcAddress(dnssd->module, "DNSServiceProcessResult");
	dnssd->DNSServiceQueryRecord = (DNSServiceQueryRecord_t)GetProcAddress(dnssd->module, "DNSServiceQueryRecord");

	dnssd->TXTRecordGetValuePtr = (TXTRecordGetValuePtr_t)GetProcAddress(dnssd->module, "TXTRecordGetValuePtr");
	dnssd->TXTRecordGetCount_ = (TXTRecordGetCount_t)GetProcAddress(dnssd->module, "TXTRecordGetCount");
	dnssd->TXTRecordGetItemAtIndex_ = (TXTRecordGetItemAtIndex_t)GetProcAddress(dnssd->module, "TXTRecordGetItemAtIndex");

	if (!dnssd->DNSServiceCreateConnection || !dnssd->DNSServiceBrowse || !dnssd->DNSServiceResolve ||
		!dnssd->DNSServiceGetAddrInfo || !dnssd->DNSServiceRefDeallocate || !dnssd->DNSServiceRefSockFD ||
		!dnssd->DNSServiceProcessResult || !dnssd->DNSServiceQueryRecord || !dnssd->TXTRecordGetValuePtr) {
		if (error) *error = DNSSD_ERROR_PROCNOTFOUND;
		FreeLibrary(dnssd->module);
		free(dnssd);
		return NULL;
	}

	strcpy(dnssd->registerName, regtype);

	// init hashmap
	MUTEX_CREATE(dnssd_map_mutex);
	hashmap_init(&dnssd_receiver_map, hashmap_hash_string, hashmap_compare_string, 0);

	// 
	DNSServiceErrorType		err;
	err = dnssd->DNSServiceCreateConnection(&dnssd->mainSrvRef);
	dnssd->airplayService = dnssd->mainSrvRef;
	err = dnssd->DNSServiceBrowse(
		&dnssd->airplayService, 				// Receives reference to Bonjour browser object.
		kDNSServiceFlagsShareConnection,	// No flags.
		kDNSServiceInterfaceIndexAny,		// Browse on all network interfaces.
		regtype,						// Browse for HTTP service types.
		NULL,								// Browse on the default domain (e.g. local.).
		DNSServiceBrowserCallBack, 			// Callback function when Bonjour events occur.
		dnssd);							// No callback context needed.


	dnssd->thread_running = true;
	THREAD_CREATE(dnssd->thread, (LPTHREAD_START_ROUTINE)dnssd_thread, (void*)dnssd);

	return dnssd;
}

void
dnssd_destroy(dnssd_t *dnssd)
{
	if (dnssd) {

		dnssd->DNSServiceRefDeallocate(dnssd->mainSrvRef);
		dnssd->DNSServiceRefDeallocate(dnssd->raopService);
		dnssd->DNSServiceRefDeallocate(dnssd->airplayService);

		if (dnssd->thread)
		{
			dnssd->thread_running = false;
			// THREAD_JOIN(dnssd->thread);
#if 1
			while (WaitForSingleObject(dnssd->thread, 0) != WAIT_OBJECT_0)
			{
				MSG msg;
				if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

			}

			CloseHandle(dnssd->thread);
			dnssd->thread = NULL;
#else
			TerminateThread(dnssd->thread, 0);
			CloseHandle(dnssd->thread);
			dnssd->thread = NULL;
#endif

		}

#ifdef WIN32
		FreeLibrary(dnssd->module);
#elif USE_LIBDL
		dlclose(dnssd->module);
#endif
		free(dnssd);
	}

	MUTEX_DESTROY(dnssd_map_mutex);
	hashmap_destroy(&dnssd_receiver_map);

}
     

void * dnssd_thread(void *args)
{
	dnssd_t *dnssd = (dnssd_t*)args;
	DNSServiceErrorType err;

	while (dnssd->thread_running)
	{
		err = dnssd->DNSServiceProcessResult(dnssd->mainSrvRef);
		if (err != kDNSServiceErr_NoError)
		{
			break;
		}


		sleepms(5);
	}


	dnssd->thread_running = false;
	return 0;
}

static void DNSSD_API DNSServiceBrowserCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		inName,
	const char *		inType,
	const char *		inDomain,
	void *				inContext)
{
	dnssd_t *dnssd = (dnssd_t*)inContext;

	if (inError != kDNSServiceErr_NoError)
		return;

	if (inFlags & kDNSServiceFlagsAdd)
	{
		DNSServiceRef *resoveSrvRef = (DNSServiceRef*)malloc(sizeof(DNSServiceRef));
		*resoveSrvRef = dnssd->mainSrvRef;
		DNSServiceErrorType error = dnssd->DNSServiceResolve(resoveSrvRef, kDNSServiceFlagsShareConnection,
			inIFI, inName, inType, inDomain, DNSServiceResolveCallBack, inContext);
	}
	else
	{
		// remove by dns-name
		char* dns_name;
		if (strstr(inName, "@"))
			dns_name = strstr(inName, "@") + 1;
		else
			dns_name = inName;

		MUTEX_LOCK(dnssd_map_mutex);

		struct hashmap_iter * it = hashmap_iter(&dnssd_receiver_map);
		while (it != NULL)
		{
			char* key = hashmap_iter_get_key(it);
			dnssd_host_t* host_info = (dnssd_host_t*)hashmap_iter_get_data(it);
			if (!strcmp(host_info->name, dns_name))
			{
				void* key = hashmap_iter_get_key(it);
				hashmap_remove(&dnssd_receiver_map, key);

				for (int i = 0; i < 4; i++) {
					if (host_info->address[i] != NULL)
						free(host_info->address[i]);
				}

				if (host_info->records)
				{
					free(host_info->records);
				}

				free(host_info);

				break;
			}

			it = hashmap_iter_next(&dnssd_receiver_map, it);
		}


		MUTEX_UNLOCK(dnssd_map_mutex);
	}


}

static void DNSSD_API DNSServiceResolveCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		fullName,
	const char *		hostTarget,
	uint16_t			opaquePort,
	uint16_t			txtLen,
	const unsigned char *txt,
	void *				inContext)
{
	dnssd_t *dnssd = (dnssd_t*)inContext;

	union { uint16_t s; u_char b[2]; } port = { opaquePort };
	uint16_t PortAsNumber = ((uint16_t)port.b[0]) << 8 | port.b[1];
 
	//////////  
	MUTEX_LOCK(dnssd_map_mutex);

	dnssd_host_t* host_info = (dnssd_host_t*)hashmap_get(&dnssd_receiver_map, hostTarget);
	if (!host_info)
	{
		char* dns_name;
		if (strstr(fullName, "@"))
			dns_name = strstr(fullName, "@") + 1;
		else 
			dns_name = fullName;

		int name_len = strstr(dns_name, ".") - dns_name;

		host_info = (dnssd_host_t*)calloc(1, sizeof(dnssd_host_t));
		host_info->name = (char*) calloc(1, name_len+1);
		strncpy(host_info->name, dns_name, name_len);
		  
		hashmap_put(&dnssd_receiver_map, _strdup(hostTarget), host_info);
	}

	if (strstr(fullName, dnssd->registerName))
		host_info->aport = PortAsNumber;
	else
		host_info->rport = PortAsNumber;

	// get address
	DNSServiceRef *addrSrvRef = (DNSServiceRef*)malloc(sizeof(DNSServiceRef));
	*addrSrvRef = dnssd->mainSrvRef;

	DNSServiceErrorType re1 = dnssd->DNSServiceGetAddrInfo(addrSrvRef,
		kDNSServiceFlagsShareConnection | kDNSServiceFlagsTimeout,
		0, kDNSServiceProtocol_IPv4, hostTarget, DNSServiceGetAddrInfoCallBack, inContext);


	uint8_t valLen = 0;
	const void* val = NULL;
	int len = txtLen;
	int count = dnssd->TXTRecordGetCount_(len, txt);
	if (count > 0)
	{
		host_info->recordsCount = count;
		host_info->records = (dnssd_record_kv_t*)malloc(sizeof(dnssd_record_kv_t) * count);
		memset(host_info->records, 0, count * sizeof(dnssd_record_kv_t));
		for (int index = 0; index < count; index++)
		{
			valLen = 0;
			DNSServiceErrorType errType = dnssd->TXTRecordGetItemAtIndex_(len, txt, index, 16, host_info->records[index].key, &valLen, &val);
			if (errType == kDNSServiceErr_NoError)
			{
				if (valLen < 128)
				{
					strncpy(host_info->records[index].val, val, valLen);
				}
				else
				{
					strncpy(host_info->records[index].val, val, 127);
				}

			}
		}
	}
	
	
	MUTEX_UNLOCK(dnssd_map_mutex);

	dnssd->DNSServiceRefDeallocate(inServiceRef);
}


static void DNSSD_API DNSServiceGetAddrInfoCallBack(
	DNSServiceRef		inServiceRef,
	DNSServiceFlags		inFlags,
	uint32_t			inIFI,
	DNSServiceErrorType	inError,
	const char *		hostname, 
	const struct sockaddr *address, 
	uint32_t			ttl, 
	void *				inContext)
{
	dnssd_t *dnssd = (dnssd_t*)inContext;

	//char *op = (inFlags & kDNSServiceFlagsAdd) ? "Add" : "Rmv";
	char addr[64] = "";
  
	if (address && address->sa_family == AF_INET)
	{
		const unsigned char *b = (const unsigned char *) &((struct sockaddr_in *)address)->sin_addr;
		snprintf(addr, sizeof(addr), "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
	}
	 
	MUTEX_LOCK(dnssd_map_mutex);
	dnssd_host_t* host_info = (dnssd_host_t*) hashmap_get(&dnssd_receiver_map, hostname);
	if (host_info) 
	{
		for (int i = 0; i < 4; i++) {
			if (host_info->address[i] == NULL) {
				host_info->address[i] = _strdup(addr);
				break;
			}
			else if (!strcmp(host_info->address[i], addr))
				break;
		}
	}

	MUTEX_UNLOCK(dnssd_map_mutex);


	dnssd->DNSServiceRefDeallocate(inServiceRef);
}


static void DNSSD_API DNSServiceQueryRecordCallback
(
	DNSServiceRef                       sdRef,
	DNSServiceFlags                     flags,
	uint32_t                            interfaceIndex,
	DNSServiceErrorType                 errorCode,
	const char                          *fullname,
	uint16_t                            rrtype,
	uint16_t                            rrclass,
	uint16_t                            rdlen,
	const void                          *rdata,
	uint32_t                            ttl,
	void                                *context
	)
{
	
}

int dnssd_get_hosts_info(dnssd_t *dnssd, dnssd_host_t** hosts)
{
	int ncount = 0, nStep = 0;

	MUTEX_LOCK(dnssd_map_mutex);

	ncount = hashmap_size(&dnssd_receiver_map);
	if (ncount > 0)
	{
		dnssd_host_t** pList = (dnssd_host_t**) calloc(ncount, sizeof(dnssd_host_t*));

		struct hashmap_iter * it = hashmap_iter(&dnssd_receiver_map);
		while (it != NULL)
		{
			char* key = hashmap_iter_get_key(it);
			dnssd_host_t* host_info = (dnssd_host_t*)hashmap_iter_get_data(it);
			
			// copy 
			dnssd_host_t* cp_info = calloc(1, sizeof(dnssd_host_t));
			cp_info->name = _strdup(host_info->name);
			for (int i = 0; i<4 && host_info->address[i]; i++)
				cp_info->address[i] = _strdup(host_info->address[i]);
			cp_info->aport = host_info->aport;
			cp_info->rport = host_info->rport;

			if (host_info->recordsCount > 0)
			{
				cp_info->recordsCount = host_info->recordsCount;
				cp_info->records = (dnssd_record_kv_t*)malloc(sizeof(dnssd_record_kv_t) * host_info->recordsCount);
				memcpy(cp_info->records, host_info->records, sizeof(dnssd_record_kv_t) * host_info->recordsCount);
			}
 
			pList[nStep++] = cp_info;

			it = hashmap_iter_next(&dnssd_receiver_map, it);
		}

		*hosts = pList;
	}


	MUTEX_UNLOCK(dnssd_map_mutex);

	return  ncount;
}

void dnssd_print()
{
	MUTEX_LOCK(dnssd_map_mutex);

	struct hashmap_iter * it = hashmap_iter(&dnssd_receiver_map);
	while (it != NULL)
	{
		char* key = hashmap_iter_get_key(it);
		dnssd_host_t* host_info = (dnssd_host_t*) hashmap_iter_get_data(it);

		printf("============= %s =============\r\n", key);

		printf("name: %s \r\n", host_info->name);
		printf("airplay port: %d \r\n", host_info->aport);
		printf("raop port: %d \r\n", host_info->rport);

		for (int i = 0; i < 4; i++) {
			printf("address[%d]: %s \r\n", i, host_info->address[i]);
		}

		it = hashmap_iter_next(&dnssd_receiver_map, it);
	}


	MUTEX_UNLOCK(dnssd_map_mutex);
}