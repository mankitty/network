#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define MAX_INTERFACE_NUMBER	4

typedef unsigned int SOCKET;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(~0)
#endif

enum INTERFACE_STATUS
{
	ERROR_ON_SUCCESS = 0,
	ERROR_ON_NOT_FOUND_INTERFACE = 1,
	ERROR_ON_CREATE_SOCKET = 2,
	ERROR_ON_MALLOC_MEMORY = 3,
	ERROR_ON_CALL_IOCTOL = 4,
	ERROR_ON_INTERFACE_IS_NOT_UP = 5,
	ERROR_ON = 2
};

std::string GetCharIpByInt(unsigned int cp)
{
	unsigned int *pp = &cp;

	int a1 = *((unsigned char *)pp + 3);
	int a2 = *((unsigned char *)pp + 2);
	int a3 = *((unsigned char *)pp + 1);
	int a4 = *((unsigned char *)pp + 0);

	return std::to_string(a1) + '.' +
		std::to_string(a2) + '.' +
		std::to_string(a3) + '.' + 
		std::to_string(a4);
}

INTERFACE_STATUS interfaceIsNormal(const char *interfaceName)
{
	int len, iLoop;
	SOCKET sock = INVALID_SOCKET;
	char *buf = NULL;
	struct ifreq * ifr = NULL;
	struct ifconf ifc;
	struct sockaddr_in *sIpPtr,*sMaskPtr;

	INTERFACE_STATUS status = ERROR_ON_SUCCESS;
	
	if (interfaceName == nullptr)
	{
		fprintf(stderr, "interfaceName is nullpter.\n");
		status = ERROR_ON_NOT_FOUND_INTERFACE;
		goto error;
	}

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr,"create socket failed.\n");
    	status = ERROR_ON_CREATE_SOCKET;
		goto error;
	}
	
	len = MAX_INTERFACE_NUMBER * sizeof(struct ifreq);

	if ((buf = (char *)malloc(len)) == NULL)
	{
		fprintf(stderr, "malloc for buf by %d bytes.\n",len);
		status = ERROR_ON_MALLOC_MEMORY;
		goto error;
	}

    ifc.ifc_len = len;
	ifc.ifc_buf = (char *)buf;

    if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0)
    {
        fprintf(stderr,"ioctl call SIOCGIFCONF for all interface failed.\n");
		status = ERROR_ON_CALL_IOCTOL;
		goto error;
    }

    ifr = (struct ifreq *)buf;
    for (iLoop = 0; iLoop < ifc.ifc_len / sizeof(struct ifreq); iLoop++)
    {
        if(ioctl(sock, SIOCGIFFLAGS, ifr) < 0)
        {
            ifr++;
            fprintf(stderr, "ioctl call SIOCGIFFLAGS failed.\n");
			status = ERROR_ON_CALL_IOCTOL;
			goto error;
    	}
        if (strcmp(ifr->ifr_name, interfaceName) != 0)
    	{
            ifr++;
            continue;
        }
        if(!(IFF_UP & ifr->ifr_flags))
        {
            ifr++;
            fprintf(stderr, "%s card is not up.\n", ifr->ifr_name);
			status = ERROR_ON_INTERFACE_IS_NOT_UP;
			goto error;
        }
        if(ioctl(sock, SIOCGIFADDR, ifr) < 0)
    	{
            ifr++;
            fprintf(stderr, "ioctl call SIOCGIFADDR failed for interface %s.\n", ifr->ifr_name);
            status = ERROR_ON_CALL_IOCTOL;
			goto error;
        }
        sIpPtr = (struct sockaddr_in *) &ifr->ifr_addr;
        fprintf(stderr, "net ip %s\n", GetCharIpByInt(ntohl(sIpPtr->sin_addr.s_addr)).c_str());
		if(ioctl(sock, SIOCGIFNETMASK, ifr) < 0)
        {
        	ifr++;
            fprintf(stderr, "ioctl call SIOCGIFNETMASK failed.\n");
            status = ERROR_ON_CALL_IOCTOL;
			goto error;
		}
		sMaskPtr = (struct sockaddr_in *) &ifr->ifr_addr;
		fprintf(stderr, "net mask %s\n", GetCharIpByInt(ntohl(sMaskPtr->sin_addr.s_addr)).c_str());
        //ifr++;
		break;
    }
error:
	if (sock == INVALID_SOCKET) close(sock);
	if (buf)
	{
		free(buf);
		buf == NULL;
	}
	return status;
}

int main(int argc, char *argv[])
{
	while(1)
	{
		interfaceIsNormal("ens33");
		sleep(1);
	}
	return 0;
}