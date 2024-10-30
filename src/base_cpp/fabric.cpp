
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

#include "base_cpp/fabric.hpp"
#include "base_cpp/debug.hpp"

namespace XPN
{

std::mutex fabric::s_mutex;
	
int set_hints( struct ::fi_info * hints)
{
	hints = fi_allocinfo();
	if (!hints)
		return -FI_ENOMEM;

	/*
	 * Request FI_EP_RDM (reliable datagram) endpoint which will allow us
	 * to reliably send messages to peers without having to
	 * listen/connect/accept.
	 */
	hints->ep_attr->type = FI_EP_RDM;

	/*
	 * Request basic messaging capabilities from the provider (no tag
	 * matching, no RMA, no atomic operations)
	 */
	hints->caps = FI_MSG;

	/*
	 * Default to FI_DELIVERY_COMPLETE which will make sure completions do
	 * not get generated until our message arrives at the destination.
	 * Otherwise, the client might get a completion and exit before the
	 * server receives the message. This is to make the test simpler.
	 */
	hints->tx_attr->op_flags = FI_DELIVERY_COMPLETE;

	/*
	 * Set the mode bit to 0. Mode bits are used to convey requirements
	 * that an application must adhere to when using the fabric interfaces.
	 * Modes specify optimal ways of accessing the reported endpoint or
	 * domain. On input to fi_getinfo, applications set the mode bits that
	 * they support.
	 */
	hints->mode = 0;

	/*
	 * Set mr_mode to 0. mr_mode is used to specify the type of memory
	 * registration capabilities the application requires. In this example
	 * we are not using memory registration so this bit will be set to 0.
	 */
	hints->domain_attr->mr_mode = 0;

	hints->domain_attr->threading = FI_THREAD_SAFE;

	/* Done setting hints */

	return 0;
}

int fabric::init ( domain &fabric )
{
	int ret;

  	debug_info("[FABRIC] [fabric_init] Start");
	
	std::unique_lock<std::mutex> lock(s_mutex);
	/*
	 * The first libfabric call to happen for initialization is fi_getinfo
	 * which queries libfabric and returns any appropriate providers that
	 * fulfill the hints requirements. Any applicable providers will be
	 * returned as a list of fi_info structs (&info). Any info can be
	 * selected. In this test we select the first fi_info struct. Assuming
	 * all hints were set appropriately, the first fi_info should be most
	 * appropriate. The flag FI_SOURCE is set for the server to indicate
	 * that the address/port refer to source information. This is not set
	 * for the client because the fields refer to the server, not the
	 * caller (client).
	 */
	set_hints(fabric.hints);
	
	ret = fi_getinfo(fi_version(), NULL, NULL, 0,
			 fabric.hints, &fabric.info);
			
  	debug_info("[FABRIC] [fabric_init] fi_getinfo = "<<ret);
	if (ret) {
		printf("fi_getinfo error (%d)\n", ret);
		return ret;
	}

	#ifdef DEBUG
		debug_info("[FABRIC] [fabric_init] "<<fi_tostr(fabric.info, FI_TYPE_INFO));
	#endif
	/*
	 * Initialize our fabric. The fabric network represents a collection of
	 * hardware and software resources that access a single physical or
	 * virtual network. All network ports on a system that can communicate
	 * with each other through their attached networks belong to the same
	 * fabric.
	 */

	ret = fi_fabric(fabric.info->fabric_attr, &fabric.fabric, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_fabric = "<<ret);
	if (ret) {
		printf("fi_fabric error (%d)\n", ret);
		return ret;
	}

	/*
	 * Initialize our domain (associated with our fabric). A domain defines
	 * the boundary for associating different resources together.
	 */

	ret = fi_domain(fabric.fabric, fabric.info, &fabric.domain, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_domain = "<<ret);
	if (ret) {
		printf("fi_domain error (%d)\n", ret);
		return ret;
	}

	return 0;
}

int fabric::new_comm ( domain &domain, comm &out_fabric_comm )
{
    struct fi_cq_attr cq_attr = {};
	struct fi_av_attr av_attr = {};
	int ret;

  	debug_info("[FABRIC] [fabric_new_comm] Start");
	std::unique_lock<std::mutex> lock(s_mutex);

	// First asing the domain to the fabric_comm
	out_fabric_comm.fabric_domain = &domain;

	/*
	 * Initialize our endpoint. Endpoints are transport level communication
	 * portals which are used to initiate and drive communication. There
	 * are three main types of endpoints:
	 * FI_EP_MSG - connected, reliable
	 * FI_EP_RDM - unconnected, reliable
	 * FI_EP_DGRAM - unconnected, unreliable
	 * The type of endpoint will be requested in hints/fi_getinfo.
	 * Different providers support different types of endpoints.
	 */

	ret = fi_endpoint(out_fabric_comm.fabric_domain->domain, out_fabric_comm.fabric_domain->info, &out_fabric_comm.ep, NULL);
  	debug_info("[FABRIC] [fabric_new_comm] fi_endpoint = "<<ret);
	if (ret) {
		printf("fi_endpoint error (%d)\n", ret);
		return ret;
	}

	/*
	 * Initialize our completion queue. Completion queues are used to
	 * report events associated with data transfers. In this example, we
	 * use one CQ that tracks sends and receives, but often times there
	 * will be separate CQs for sends and receives.
	 */

	cq_attr.size = 128;
	cq_attr.format = FI_CQ_FORMAT_MSG;
	cq_attr.wait_obj = FI_WAIT_UNSPEC;
	ret = fi_cq_open(out_fabric_comm.fabric_domain->domain, &cq_attr, &out_fabric_comm.cq, NULL);
  	debug_info("[FABRIC] [fabric_new_comm] fi_cq_open = "<<ret);
	if (ret) {
		printf("fi_cq_open error (%d)\n", ret);
		return ret;
	}

	/*
	 * Bind our CQ to our endpoint to track any sends and receives that
	 * come in or out on that endpoint. A CQ can be bound to multiple
	 * endpoints but one EP can only have one send CQ and one receive CQ
	 * (which can be the same CQ).
	 */

	ret = fi_ep_bind(out_fabric_comm.ep, &out_fabric_comm.cq->fid, FI_SEND | FI_RECV);
  	debug_info("[FABRIC] [fabric_new_comm] fi_ep_bind = "<<ret);
	if (ret) {
		printf("fi_ep_bind cq error (%d)\n", ret);
		return ret;
	}

	/*
	 * Initialize our address vector. Address vectors are used to map
	 * higher level addresses, which may be more natural for an application
	 * to use, into fabric specific addresses. An AV_TABLE av will map
	 * these addresses to indexed addresses, starting with fi_addr 0. These
	 * addresses are used in data transfer calls to specify which peer to
	 * send to/recv from. Address vectors are only used for FI_EP_RDM and
	 * FI_EP_DGRAM endpoints, allowing the application to avoid connection
	 * management. For FI_EP_MSG endpoints, the AV is replaced by the
	 * traditional listen/connect/accept steps.
	 */

	av_attr.type = FI_AV_TABLE;
	av_attr.count = 1;
	ret = fi_av_open(out_fabric_comm.fabric_domain->domain, &av_attr, &out_fabric_comm.av, NULL);
  	debug_info("[FABRIC] [fabric_new_comm] fi_av_open = "<<ret);
	if (ret) {
		printf("fi_av_open error (%d)\n", ret);
		return ret;
	}

	/*
	 * Bind the AV to the EP. The EP can only send data to a peer in its
	 * AV.
	 */

	ret = fi_ep_bind(out_fabric_comm.ep, &out_fabric_comm.av->fid, 0);
  	debug_info("[FABRIC] [fabric_new_comm] fi_ep_bind = "<<ret);
	if (ret) {
		printf("fi_ep_bind av error (%d)\n", ret);
		return ret;
	}

	/*
	 * Once we have all our resources initialized and ready to go, we can
	 * enable our EP in order to send/receive data.
	 */

	ret = fi_enable(out_fabric_comm.ep);
  	debug_info("[FABRIC] [fabric_new_comm] fi_enable = "<<ret);
	if (ret) {
		printf("fi_enable error (%d)\n", ret);
		return ret;
	}

	return 0;
}

int fabric::get_addr( comm &fabric_comm, char * out_addr, size_t &size_addr )
{
	int ret = -1;
  	debug_info("[FABRIC] [fabric_get_addr] Start");
	ret = fi_getname(&fabric_comm.ep->fid, out_addr, &size_addr);
	if (ret) {
		printf("fi_getname error %d\n", ret);
		return ret;
	}
  	debug_info("[FABRIC] [fabric_get_addr] End = "<<ret);
	return ret;
}

int fabric::register_addr( comm &fabric_comm, char * addr_buf )
{
	int ret = -1;
  	debug_info("[FABRIC] [fabric_register_addr] Start");
	ret = fi_av_insert(fabric_comm.av, addr_buf, 1, &fabric_comm.fi_addr, 0, NULL);
	if (ret != 1) {
		printf("av insert error\n");
		return -FI_ENOSYS;
	}
  	debug_info("[FABRIC] [fabric_register_addr] End = "<<ret);
	return ret;
}

int fabric::wait ( comm &fabric_comm )
{
	struct fi_cq_err_entry comp;
	int ret;

  	debug_info("[FABRIC] [fabric_wait] Start");
	// ret = fi_cq_sreadfrom(fabric_comm.cq, &comp, 1, &fabric_comm.fi_addr, NULL, -1);
	ret = fi_cq_sread(fabric_comm.cq, &comp, 1, NULL, -1);
	if (ret < 0){
		printf("error reading cq (%d)\n", ret);
	}
  	debug_info("[FABRIC] [fabric_wait] End = "<<ret);

	return ret;
}

int fabric::send ( comm &fabric_comm, const void * buffer, size_t size )
{
	int ret;

  	debug_info("[FABRIC] [fabric_send] Start");
	do { 
		ret = fi_send(fabric_comm.ep, buffer, size, NULL, fabric_comm.fi_addr, NULL);
		
		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(fabric_comm.cq, NULL, 0);
	} while (ret == -FI_EAGAIN);
	
	if (ret){
		printf("error posting send buffer (%d)\n", ret);
		return -1;
	} 
    ret = fabric::wait(fabric_comm);
	if (ret < 0){
		printf("error waiting send buffer (%d)\n", ret);
		return -1;
	} 
	
  	debug_info("[FABRIC] [fabric_send] End = "<<size);
	return size;
}

int fabric::recv ( comm &fabric_comm, void * buffer, size_t size )
{
	int ret;

  	debug_info("[FABRIC] [fabric_recv] Start");
	do { 
		ret = fi_recv(fabric_comm.ep, buffer, size, NULL, fabric_comm.fi_addr, NULL);

		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(fabric_comm.cq, NULL, 0);
	} while (ret == -FI_EAGAIN);
	
	if (ret){
		printf("error posting recv buffer (%d)\n", ret);
		return -1;
	}
    ret = fabric::wait(fabric_comm);
	if (ret < 0){
		printf("error waiting recv buffer (%d)\n", ret);
		return -1;
	} 

  	debug_info("[FABRIC] [fabric_recv] End = "<<size);
	return size;
}

int fabric::close ( comm &fabric_comm )
{
	int ret;
	debug_info("[FABRIC] [fabric_close_comm] Start");

	std::unique_lock<std::mutex> lock(s_mutex);

	debug_info("[FABRIC] [fabric_close_comm] Close endpoint");
	ret = fi_close(&fabric_comm.ep->fid);
	if (ret)
		printf("warning: error closing EP (%d)\n", ret);

	debug_info("[FABRIC] [fabric_close_comm] Close address vector");
	ret = fi_close(&fabric_comm.av->fid);
	if (ret)
		printf("warning: error closing AV (%d)\n", ret);

	debug_info("[FABRIC] [fabric_close_comm] Close completion queue");
	ret = fi_close(&fabric_comm.cq->fid);
	if (ret)
		printf("warning: error closing CQ (%d)\n", ret);
		
	debug_info("[FABRIC] [fabric_close_comm] End = "<<ret);
	
	return ret;
}

int fabric::destroy ( domain &domain )
{
	int ret;

	debug_info("[FABRIC] [fabric_destroy] Start");

	std::unique_lock<std::mutex> lock(s_mutex);
  	
	debug_info("[FABRIC] [fabric_destroy] Close domain");
	ret = fi_close(&domain.domain->fid);
	if (ret)
		printf("warning: error closing domain (%d)\n", ret);

	debug_info("[FABRIC] [fabric_destroy] Close fabric");
	ret = fi_close(&domain.fabric->fid);
	if (ret)
		printf("warning: error closing fabric (%d)\n", ret);

	debug_info("[FABRIC] [fabric_destroy] Free hints ");
	if (domain.hints)
		fi_freeinfo(domain.hints);

	debug_info("[FABRIC] [fabric_destroy] Free info ");
	if (domain.info)
		fi_freeinfo(domain.info);

	debug_info("[FABRIC] [fabric_destroy] End = "<<ret);

	return ret;
}

} // namespace XPN