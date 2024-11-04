
/*
 *  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
 *
 *  This file is part of Expand.
 *
 *  Expand is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Expand is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Expand.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <base_cpp/fabric.hpp>
#include <base_cpp/timer.hpp>

using namespace XPN;

#define BUF_SIZE 1024 * 1024

char *src_addr = NULL, *dst_addr = NULL;
const char *oob_port = "9228";
int listen_sock, oob_sock;

std::vector<char> buf(BUF_SIZE);
std::vector<char> msg(BUF_SIZE);

fabric::comm fabric_comm;
fabric::domain fabric_domain;

static int sock_listen(char *node, const char *service)
{
	struct addrinfo *ai, hints;
	int val, ret;

	memset(&hints, 0, sizeof hints);
	hints.ai_flags = AI_PASSIVE;

	ret = getaddrinfo(node, service, &hints, &ai);
	if (ret) {
		printf("getaddrinfo() %s\n", gai_strerror(ret));
		return ret;
	}

	listen_sock = socket(ai->ai_family, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		printf("socket error");
		ret = listen_sock;
		goto out;
	}

	val = 1;
	ret = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
			 (void *) &val, sizeof val);
	if (ret) {
		printf("setsockopt SO_REUSEADDR");
		goto out;
	}

	ret = bind(listen_sock, ai->ai_addr, ai->ai_addrlen);
	if (ret) {
		printf("bind");
		goto out;
	}

	ret = listen(listen_sock, 0);
	if (ret)
		printf("listen error");

out:
	if (ret && listen_sock >= 0)
		close(listen_sock);
	freeaddrinfo(ai);
	return ret;
}

static int sock_setup(int sock)
{
	int ret, op;
	long flags;

	op = 1;
	ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
			  (void *) &op, sizeof(op));
	if (ret)
		return ret;

	flags = fcntl(sock, F_GETFL);
	if (flags < 0)
		return -errno;

	if (fcntl(sock, F_SETFL, flags))
		return -errno;

	return 0;
}

static int init_oob(void)
{
	struct addrinfo *ai = NULL;
	int ret;

	if (!dst_addr) {
		ret = sock_listen(src_addr, oob_port);
		if (ret)
			return ret;

		oob_sock = accept(listen_sock, NULL, 0);
		if (oob_sock < 0) {
			printf("accept error");
			ret = oob_sock;
			return ret;
		}

		close(listen_sock);
	} else {
		ret = getaddrinfo(dst_addr, oob_port, NULL, &ai);
		if (ret) {
			printf("getaddrinfo error");
			return ret;
		}

		oob_sock = socket(ai->ai_family, SOCK_STREAM, 0);
		if (oob_sock < 0) {
			printf("socket error");
			ret = oob_sock;
			goto free;
		}

		ret = connect(oob_sock, ai->ai_addr, ai->ai_addrlen);
		if (ret) {
			printf("connect error");
			close(oob_sock);
			goto free;
		}
		sleep(1);
	}

	ret = sock_setup(oob_sock);

free:
	if (ai)
		freeaddrinfo(ai);
	return ret;
}

static int sock_send(int fd, void *msg, size_t len)
{
	size_t sent;
	ssize_t ret, err = 0;

	for (sent = 0; sent < len; ) {
		ret = send(fd, ((char *) msg) + sent, len - sent, 0);
		if (ret > 0) {
			sent += ret;
		} else {
			err = -errno;
			break;
		}
	}

	return err ? err: 0;
}

static int sock_recv(int fd, void *msg, size_t len)
{
	size_t rcvd;
	ssize_t ret, err = 0;

	for (rcvd = 0; rcvd < len; ) {
		ret = recv(fd, ((char *) msg) + rcvd, len - rcvd, 0);
		if (ret > 0) {
			rcvd += ret;
		} else if (ret == 0) {
			err = -FI_ENOTCONN;
			break;
		} else {
			err = -errno;
			break;
		}
	}

	return err ? err: 0;
}

static int exchange_addresses(void)
{
	#define BUF_SIZE_AUX 64
	char addr_buf[BUF_SIZE_AUX];
	int ret;
	size_t addrlen = BUF_SIZE_AUX;

    ret = fabric::get_addr(fabric_comm, addr_buf, addrlen);
	if (ret) {
		printf("fi_getname error %d\n", ret);
		return ret;
	}

	ret = sock_send(oob_sock, addr_buf, BUF_SIZE_AUX);
	if (ret) {
		printf("sock_send error %d\n", ret);
		return ret;
	}

	memset(addr_buf, 0, BUF_SIZE_AUX);
	ret = sock_recv(oob_sock, addr_buf, BUF_SIZE_AUX);
	if (ret) {
		printf("sock_recv error %d\n", ret);
		return ret;
	}

    ret = fabric::register_addr(fabric_comm, addr_buf);
	if (ret != 1) {
		printf("av insert error\n");
		return -FI_ENOSYS;
	}

	return 0;
}

static int post_recv(void)
{
	int ret;

    ret = fabric::recv(fabric_comm, buf.data(), buf.size()*sizeof(buf[0]));

	return ret;
}

static int post_send(void)
{

	static int count = 0;
	if (dst_addr){
		sprintf(msg.data(), "Hello, server! I am the client you've been waiting for! %d", count++);
	}else{
		sprintf(msg.data(), "Hello, client! I am the server you've been waiting for! %d", count++);
	}
	int ret;

    ret = fabric::send(fabric_comm, msg.data(), msg.size()*sizeof(msg[0]));

	return ret;
}

static int run(void)
{
	int ret;
	timer timer;

	if (dst_addr) {
		printf("Client: send to server %s\n", dst_addr);

		for (int i = 0; i < 10; i++)
		{
			printf("Client: send buffer and wait for the server to recv\n");
			ret = post_send();
			if (ret<0)
				return ret;
			std::cout<<"Send "<<BUF_SIZE<<" in "<<std::fixed<<std::setprecision(3)<<timer.elapsedMilli()<<" ms"<<std::endl;
			timer.reset();
			printf("Client: post buffer and wait for message from server\n");
			ret = post_recv();
			if (ret<0)
				return ret;
			std::cout<<"Recv "<<BUF_SIZE<<" in "<<std::fixed<<std::setprecision(3)<<timer.elapsedMilli()<<" ms"<<std::endl;
			timer.reset();

			printf("This is the message I received: %s\n", buf.data());
		}
		

	} else {

		printf("Server: send to client data\n");
		for (int i = 0; i < 10; i++)
		{
			printf("Server: post buffer and wait for message from client\n");
			ret = post_recv();
			if (ret<0)
				return ret;

			std::cout<<"Recv "<<BUF_SIZE<<" in "<<std::fixed<<std::setprecision(3)<<timer.elapsedMilli()<<" ms"<<std::endl;
			timer.reset();
			printf("This is the message I received: %s\n", buf.data());

			printf("Server: send buffer and wait for the client to recv\n");
			ret = post_send();
			if (ret<0)
				return ret;
			std::cout<<"Send "<<BUF_SIZE<<" in "<<std::fixed<<std::setprecision(3)<<timer.elapsedMilli()<<" ms"<<std::endl;
			timer.reset();
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	/*
	 * Server run with no args, client has server's address as an
	 * argument.
	 */
    if (argc > 1)
	    dst_addr = argv[1];

	/* Init out-of-band addressing */
	ret = init_oob();
	if (ret)
		return ret;

	/*
	 * Hints are used to request support for specific features from a
	 * provider.
	 */

    ret = fabric::init(fabric_domain);
	if (ret)
		goto out;
	
	std::cout << "[FABRIC] [fabric_init] "<<fi_tostr(fabric_domain.info, FI_TYPE_INFO)<<std::endl;

    ret = fabric::new_comm(fabric_domain, fabric_comm);
	if (ret)
		goto out;
        
	ret = exchange_addresses();
	if (ret)
		goto out;

	ret = run();
out:
    fabric::close(fabric_comm);
    fabric::destroy(fabric_domain);
	return ret;
}