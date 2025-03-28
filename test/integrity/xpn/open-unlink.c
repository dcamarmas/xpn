
#include "all_system.h"
#include "xpn.h"

#define BUFF_SIZE (1024*1024)

int main ( int argc, char *argv[] )
{
	int  ret ;
	int  fd1 ;
	//char buffer[BUFF_SIZE] ;

	printf("env XPN_CONF=./xpn.conf %s\n", argv[0]);

	// xpn-init
	ret = xpn_init();
	printf("%d = xpn_init()\n", ret);
	if (ret < 0) {
	    printf("ERROR: xpn_init()\n");
	    return -1;
	}

	fd1 = xpn_creat("/P1/test_1", 00777);
	printf("%d = xpn_creat('%s', %o)\n", fd1, "/P1/test_1", 00777);

	ret = xpn_unlink("/P1/test_1");
	printf("%d = xpn_unlink(%s)\n", ret, "/P1/test_1") ;

	// xpn_destroy
	ret = xpn_destroy();
	printf("%d = xpn_destroy()\n", ret);
	if (ret < 0) {
	    printf("ERROR: xpn_destroy()\n");
	    return -1;
	}

	return 0;
}
