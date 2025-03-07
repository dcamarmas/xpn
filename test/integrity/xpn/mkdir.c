
#include "all_system.h"
#include "xpn.h"

#define BUFF_SIZE (1024*1024)

int main ( int argc, char *argv[] )
{
	int  ret ;
	//char buffer[BUFF_SIZE] ;

	printf("env XPN_CONF=./xpn.conf %s\n", argv[0]);

	// xpn-init
	ret = xpn_init();
	printf("%d = xpn_init()\n", ret);
	if (ret < 0) {
	    return -1;
	}

	// test 1
	ret = xpn_mkdir("/P1/d4", 00777);
	printf("%d = xpn_mkdir('%s', %o)\n", ret, "/P1/d4", 00777);

	// xpn-destroy
	ret = xpn_destroy();
	printf("%d = xpn_destroy()\n", ret);
	if (ret < 0) {
	    printf("ERROR: xpn_destroy()\n");
	    return -1;
	}

	return 0;
}

