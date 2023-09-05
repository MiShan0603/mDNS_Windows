#ifndef DNSSD_H
#define DNSSD_H



#include "platform.h"

#if defined(WIN32) && defined(DLL_EXPORT)
# define DNSSD_API __declspec(dllexport)
#elif defined(WIN32)
# define DNSSD_API __stdcall
#else
# define DNSSD_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DNSSD_ERROR_NOERROR       0
#define DNSSD_ERROR_HWADDRLEN     1
#define DNSSD_ERROR_OUTOFMEM      2
#define DNSSD_ERROR_LIBNOTFOUND   3
#define DNSSD_ERROR_PROCNOTFOUND  4



typedef struct dnssd_s dnssd_t;

typedef struct dnssd_record_kv_s
{
	char key[16];
	char val[128];
}dnssd_record_kv_t;

typedef struct dnssd_host_s
{
	char*		name;
	char*		address[4];
	u32_t		aport; // airplay port
	u32_t		rport; // raop port

	dnssd_record_kv_t *records;
	int recordsCount;
}dnssd_host_t;




/// for server
dnssd_t *dnssd_init_srv(int *error);
int dnssd_register_server(dnssd_t* dnssd, const char* name, unsigned short port, const char* regtype, dnssd_record_kv_t *records, int recordsCount);
void dnssd_unregister_server(dnssd_t *dnssd);


/// for Client
dnssd_t *dnssd_init_client(int *error, const char* regtype);
void	 dnssd_destroy(dnssd_t *dnssd);

int		 dnssd_get_hosts_info(dnssd_t *dnssd, dnssd_host_t** hosts);
void	 dnssd_print();

#ifdef __cplusplus
}
#endif
#endif
