
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

// #define DEBUG
#include "base_cpp/fabric.hpp"
#include "base_cpp/debug.hpp"

namespace XPN
{

std::mutex fabric::s_mutex;


void print_flags(uint64_t flags) {
    debug_info("  Flags set:");

    if (flags & FI_MSG) { debug_info("    FI_MSG"); }
    if (flags & FI_RMA) { debug_info("    FI_RMA"); }
    if (flags & FI_TAGGED) { debug_info("    FI_TAGGED"); }
    if (flags & FI_ATOMIC) { debug_info("    FI_ATOMIC"); }
    if (flags & FI_MULTICAST) { debug_info("    FI_MULTICAST"); }
    if (flags & FI_COLLECTIVE) { debug_info("    FI_COLLECTIVE"); }
    
    if (flags & FI_READ) { debug_info("    FI_READ"); }
    if (flags & FI_WRITE) { debug_info("    FI_WRITE"); }
    if (flags & FI_RECV) { debug_info("    FI_RECV"); }
    if (flags & FI_SEND) { debug_info("    FI_SEND"); }
    if (flags & FI_REMOTE_READ) { debug_info("    FI_REMOTE_READ"); }
    if (flags & FI_REMOTE_WRITE) { debug_info("    FI_REMOTE_WRITE"); }
    
    if (flags & FI_MULTI_RECV) { debug_info("    FI_MULTI_RECV"); }
    if (flags & FI_REMOTE_CQ_DATA) { debug_info("    FI_REMOTE_CQ_DATA"); }
    if (flags & FI_MORE) { debug_info("    FI_MORE"); }
    if (flags & FI_PEEK) { debug_info("    FI_PEEK"); }
    if (flags & FI_TRIGGER) { debug_info("    FI_TRIGGER"); }
    if (flags & FI_FENCE) { debug_info("    FI_FENCE"); }
    // if (flags & FI_PRIORITY) { debug_info("    FI_PRIORITY"); }

    if (flags & FI_COMPLETION) { debug_info("    FI_COMPLETION"); }
    if (flags & FI_INJECT) { debug_info("    FI_INJECT"); }
    if (flags & FI_INJECT_COMPLETE) { debug_info("    FI_INJECT_COMPLETE"); }
    if (flags & FI_TRANSMIT_COMPLETE) { debug_info("    FI_TRANSMIT_COMPLETE"); }
    if (flags & FI_DELIVERY_COMPLETE) { debug_info("    FI_DELIVERY_COMPLETE"); }
    if (flags & FI_AFFINITY) { debug_info("    FI_AFFINITY"); }
    if (flags & FI_COMMIT_COMPLETE) { debug_info("    FI_COMMIT_COMPLETE"); }
    if (flags & FI_MATCH_COMPLETE) { debug_info("    FI_MATCH_COMPLETE"); }

    if (flags & FI_HMEM) { debug_info("    FI_HMEM"); }
    if (flags & FI_VARIABLE_MSG) { debug_info("    FI_VARIABLE_MSG"); }
    if (flags & FI_RMA_PMEM) { debug_info("    FI_RMA_PMEM"); }
    if (flags & FI_SOURCE_ERR) { debug_info("    FI_SOURCE_ERR"); }
    if (flags & FI_LOCAL_COMM) { debug_info("    FI_LOCAL_COMM"); }
    if (flags & FI_REMOTE_COMM) { debug_info("    FI_REMOTE_COMM"); }
    if (flags & FI_SHARED_AV) { debug_info("    FI_SHARED_AV"); }
    if (flags & FI_PROV_ATTR_ONLY) { debug_info("    FI_PROV_ATTR_ONLY"); }
    if (flags & FI_NUMERICHOST) { debug_info("    FI_NUMERICHOST"); }
    if (flags & FI_RMA_EVENT) { debug_info("    FI_RMA_EVENT"); }
    if (flags & FI_SOURCE) { debug_info("    FI_SOURCE"); }
    if (flags & FI_NAMED_RX_CTX) { debug_info("    FI_NAMED_RX_CTX"); }
    if (flags & FI_DIRECTED_RECV) { debug_info("    FI_DIRECTED_RECV"); }
}

void print_fi_cq_err_entry(const fi_cq_err_entry& entry) {
    debug_info("fi_cq_err_entry:");
    debug_info("  op_context: " << entry.op_context);
	print_flags(entry.flags);
    // debug_info("  flags: " << entry.flags);
    debug_info("  len: " << entry.len);
    debug_info("  buf: " << entry.buf);
    debug_info("  data: " << entry.data);
    debug_info("  tag: " << entry.tag);
    debug_info("  olen: " << entry.olen);
    debug_info("  err: " << entry.err);
    debug_info("  prov_errno: " << entry.prov_errno);
    debug_info("  err_data: " << entry.err_data);
    debug_info("  err_data_size: " << entry.err_data_size);
}
	
int fabric::set_hints( fabric_ep &fabric_ep )
{
	fabric_ep.hints = fi_allocinfo();
	if (!fabric_ep.hints)
		return -FI_ENOMEM;

	/*
	 * Request FI_EP_RDM (reliable datagram) endpoint which will allow us
	 * to reliably send messages to peers without having to
	 * listen/connect/accept.
	 */
	fabric_ep.hints->ep_attr->type = FI_EP_RDM;

	/*
	 * Request basic messaging capabilities from the provider (no tag
	 * matching, no RMA, no atomic operations)
	 */
	fabric_ep.hints->caps = FI_MSG | FI_TAGGED;

	/*
	 * Default to FI_DELIVERY_COMPLETE which will make sure completions do
	 * not get generated until our message arrives at the destination.
	 * Otherwise, the client might get a completion and exit before the
	 * server receives the message. This is to make the test simpler.
	 */
	fabric_ep.hints->tx_attr->op_flags = FI_DELIVERY_COMPLETE;

	/*
	 * Set the mode bit to 0. Mode bits are used to convey requirements
	 * that an application must adhere to when using the fabric interfaces.
	 * Modes specify optimal ways of accessing the reported endpoint or
	 * domain. On input to fi_getinfo, applications set the mode bits that
	 * they support.
	 */
	fabric_ep.hints->mode = FI_CONTEXT;

	/*
	 * Set mr_mode to 0. mr_mode is used to specify the type of memory
	 * registration capabilities the application requires. In this example
	 * we are not using memory registration so this bit will be set to 0.
	 */
	// hints->domain_attr->mr_mode = 0;

	// hints->domain_attr->threading = FI_THREAD_SAFE;

	/* Done setting hints */

	return 0;
}

int fabric::init ( fabric_ep &fabric_ep )
{
	int ret; 
	struct fi_cq_attr cq_attr = {};
	struct fi_av_attr av_attr = {};

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
	set_hints(fabric_ep);
	
	ret = fi_getinfo(fi_version(), NULL, NULL, 0,
			 fabric_ep.hints, &fabric_ep.info);
			
  	debug_info("[FABRIC] [fabric_init] fi_getinfo = "<<ret);
	if (ret) {
		printf("fi_getinfo error (%d)\n", ret);
		return ret;
	}

	#ifdef DEBUG
		debug_info("[FABRIC] [fabric_init] "<<fi_tostr(fabric_ep.info, FI_TYPE_INFO));
	#endif
	/*
	 * Initialize our fabric. The fabric network represents a collection of
	 * hardware and software resources that access a single physical or
	 * virtual network. All network ports on a system that can communicate
	 * with each other through their attached networks belong to the same
	 * fabric.
	 */

	ret = fi_fabric(fabric_ep.info->fabric_attr, &fabric_ep.fabric, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_fabric = "<<ret);
	if (ret) {
		printf("fi_fabric error (%d)\n", ret);
		return ret;
	}

	/*
	 * Initialize our domain (associated with our fabric). A domain defines
	 * the boundary for associating different resources together.
	 */

	ret = fi_domain(fabric_ep.fabric, fabric_ep.info, &fabric_ep.domain, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_domain = "<<ret);
	if (ret) {
		printf("fi_domain error (%d)\n", ret);
		return ret;
	}

	
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

	ret = fi_endpoint(fabric_ep.domain, fabric_ep.info, &fabric_ep.ep, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_endpoint = "<<ret);
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
	cq_attr.format = FI_CQ_FORMAT_TAGGED;
	cq_attr.wait_obj = FI_WAIT_UNSPEC;
	ret = fi_cq_open(fabric_ep.domain, &cq_attr, &fabric_ep.cq, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_cq_open = "<<ret);
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

	ret = fi_ep_bind(fabric_ep.ep, &fabric_ep.cq->fid, FI_SEND | FI_RECV);
  	debug_info("[FABRIC] [fabric_init] fi_ep_bind = "<<ret);
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

	av_attr.type = FI_AV_MAP;
	// av_attr.count = 1;
	ret = fi_av_open(fabric_ep.domain, &av_attr, &fabric_ep.av, NULL);
  	debug_info("[FABRIC] [fabric_init] fi_av_open = "<<ret);
	if (ret) {
		printf("fi_av_open error (%d)\n", ret);
		return ret;
	}

	/*
	 * Bind the AV to the EP. The EP can only send data to a peer in its
	 * AV.
	 */

	ret = fi_ep_bind(fabric_ep.ep, &fabric_ep.av->fid, 0);
  	debug_info("[FABRIC] [fabric_init] fi_ep_bind = "<<ret);
	if (ret) {
		printf("fi_ep_bind av error (%d)\n", ret);
		return ret;
	}

	/*
	 * Once we have all our resources initialized and ready to go, we can
	 * enable our EP in order to send/receive data.
	 */

	ret = fi_enable(fabric_ep.ep);
  	debug_info("[FABRIC] [fabric_init] fi_enable = "<<ret);
	if (ret) {
		printf("fi_enable error (%d)\n", ret);
		return ret;
	}

	// Create FABRIC_ANY_RANK
	fabric::any_comm(fabric_ep);

	fabric::init_thread_cq(fabric_ep);

	return 0;
}

int fabric::init_thread_cq(fabric_ep &fabric_ep)
{
  	debug_info("[FABRIC] [init_thread_cq] Start");
    fabric_ep.thread_cq = std::thread([&fabric_ep](){
		run_thread_cq(fabric_ep);
	});
  	debug_info("[FABRIC] [init_thread_cq] End");
	return 0;
}

int fabric::destroy_thread_cq(fabric_ep &fabric_ep)
{
  	debug_info("[FABRIC] [destroy_thread_cq] Start");
	{
		std::lock_guard<std::mutex> lock(fabric_ep.thread_cq_mutex);
		fabric_ep.thread_cq_is_running = false;
	}
	fabric_ep.thread_cq_cv.notify_one();
	fabric_ep.thread_cq.join();
  	debug_info("[FABRIC] [destroy_thread_cq] End");
	return 0;
}

int fabric::run_thread_cq(fabric_ep &fabric_ep)
{
	int ret = 0;
	struct fi_cq_err_entry comp = {};
	std::unique_lock<std::mutex> lock(fabric_ep.thread_cq_mutex);

	while (fabric_ep.thread_cq_is_running) {
		if (fabric_ep.thread_cq_cv.wait_for(lock, std::chrono::nanoseconds(1), [&fabric_ep]{ return !fabric_ep.thread_cq_is_running; })) {
			break;
		}

		if (fabric_ep.subs_to_wait == 0) { continue; }
		{
			std::unique_lock<std::mutex> lock(fabric_ep.thread_fi_mutex);
			ret = fi_cq_read(fabric_ep.cq, &comp, 1);
		}
		if (ret == -FI_EAGAIN){ continue; }

		fabric_context* context = static_cast<fabric_context*>(comp.op_context);
		fabric_comm &comm = fabric_ep.m_comms[context->rank];
		context->entry = comp;

		{
			std::unique_lock<std::mutex> lock(comm.comm_mutex);
			if (comp.flags & FI_SEND) {
				debug_info("[FABRIC] [run_thread_cq] Send cq of rank_peer "<<context->rank);
			}
			if (comp.flags & FI_RECV) {
				debug_info("[FABRIC] [run_thread_cq] Recv cq of rank_peer "<<context->rank);
			} 
			
			// print_fi_cq_err_entry(comp);
			fabric_ep.subs_to_wait--;
			comm.wait_context = false;
			comm.comm_cv.notify_one();
		}
	}
	return ret;
}

fabric::fabric_comm& fabric::new_comm ( fabric_ep &fabric_ep )
{
	static uint32_t rank_counter = 0;

  	debug_info("[FABRIC] [fabric_new_comm] Start");
	std::unique_lock<std::mutex> lock(s_mutex);
	
	auto[key, inserted] = fabric_ep.m_comms.emplace(std::piecewise_construct,
												std::forward_as_tuple(rank_counter),
												std::forward_as_tuple());
	key->second.m_ep = &fabric_ep;
	key->second.rank_peer = rank_counter;
	rank_counter++;
  	debug_info("[FABRIC] [fabric_new_comm] rank_peer "<<key->second.rank_peer);
  	debug_info("[FABRIC] [fabric_new_comm] End");
	return key->second;
}

fabric::fabric_comm& fabric::any_comm ( fabric_ep &fabric_ep )
{
  	debug_info("[FABRIC] [any_comm] Start");
	// std::unique_lock<std::mutex> lock(s_mutex);
	
	auto[key, inserted] = fabric_ep.m_comms.emplace(std::piecewise_construct,
												std::forward_as_tuple(FABRIC_ANY_RANK),
												std::forward_as_tuple());
	key->second.m_ep = &fabric_ep;
	key->second.rank_peer = FABRIC_ANY_RANK;
	key->second.fi_addr = FI_ADDR_UNSPEC;
  	debug_info("[FABRIC] [any_comm] End");
	return key->second;
}

fabric::fabric_comm& fabric::get_any_rank_comm(fabric_ep &fabric_ep)
{
	return fabric_ep.m_comms[FABRIC_ANY_RANK];
}


int fabric::get_addr( fabric_ep &fabric_ep, char * out_addr, size_t &size_addr )
{
	int ret = -1;
  	debug_info("[FABRIC] [fabric_get_addr] Start");
	ret = fi_getname(&fabric_ep.ep->fid, out_addr, &size_addr);
	if (ret) {
		printf("fi_getname error %d\n", ret);
		return ret;
	}
  	debug_info("[FABRIC] [fabric_get_addr] End = "<<ret);
	return ret;
}

int fabric::register_addr( fabric_ep &fabric_ep, fabric_comm& fabric_comm, char * addr_buf )
{
	int ret = -1;
	fi_addr_t fi_addr;
  	debug_info("[FABRIC] [fabric_register_addr] Start");
	ret = fi_av_insert(fabric_ep.av, addr_buf, 1, &fi_addr, 0, NULL);
	if (ret != 1) {
		printf("av insert error %d\n", ret);
		return ret;
	}

	fabric_comm.fi_addr = fi_addr;

  	debug_info("[FABRIC] [fabric_register_addr] End = "<<ret);
	return ret;
}

int fabric::remove_addr(fabric_ep &fabric_ep, fabric_comm& fabric_comm)
{
	int ret = -1;
  	debug_info("[FABRIC] [fabric_remove_addr] Start");
	ret = fi_av_remove(fabric_ep.av, &fabric_comm.fi_addr, 1, 0);	
	if (ret < 0) {
		printf("av remove error %d\n", ret);
		return ret;
	}
  	debug_info("[FABRIC] [fabric_remove_addr] End = "<<ret);
	return ret;
}

// fabric::fabric_msg fabric::wait ( fabric_ep &fabric_ep )
// {
	// struct fi_cq_err_entry comp = {};
	// int ret;
	// fabric_msg msg = {};

  	// debug_info("[FABRIC] [fabric_wait] Start");
	// // ret = fi_cq_sreadfrom(fabric_ep.cq, &comp, 1, &fabric_ep.fi_addr, NULL, -1);
	// // ret = fi_cq_sread(fabric_ep.cq, &comp, 1, NULL, -1);
	// do {
	// 	ret = fi_cq_read(fabric_ep.cq, &comp, 1);
	// 	if (ret < 0 && ret != -FI_EAGAIN) {
	// 		printf("error reading cq (%d)\n", ret);
	// 		msg.error = ret;
	// 		return msg;
	// 	}
	// } while (ret != 1);
	// if (ret < 0){
	// 	printf("error reading cq (%d)\n", ret);
	// }
	// print_fi_cq_err_entry(comp);
	// msg.size = comp.len;
	// msg.tag = comp.tag & 0x00000000FFFFFFFF;
	// msg.rank = comp.tag >> 32;
	// msg.error = ret;
  	// debug_info("[FABRIC] [fabric_wait] End = "<<ret);

	// return msg;
// }

fabric::fabric_msg fabric::send ( fabric_ep &fabric_ep, fabric_comm& fabric_comm, const void * buffer, size_t size, uint32_t tag )
{
	int ret;
	fabric_msg msg = {};

	// tag format 24 bits rank_peer 24 bits rank_self_in_peer 16 bits tag
	uint64_t aux_rank_peer = fabric_comm.rank_peer;
	uint64_t aux_rank_self_in_peer = fabric_comm.rank_self_in_peer;
	uint64_t aux_tag = tag;
	uint64_t tag_send = (aux_rank_peer << 40) | (aux_rank_self_in_peer << 16) | aux_tag;

	fabric_comm.context.rank = fabric_comm.rank_peer;

  	debug_info("[FABRIC] [fabric_send] Start size "<<size<<" rank_peer "<<fabric_comm.rank_peer<<" rank_self_in_peer "<<fabric_comm.rank_self_in_peer<<" tag "<<tag<<" send_context "<<(void*)&fabric_comm.context);

	do {
		std::unique_lock<std::mutex> lock(fabric_ep.thread_fi_mutex);
		ret = fi_tsend(fabric_ep.ep, buffer, size, NULL, fabric_comm.fi_addr, tag_send, &fabric_comm.context);
		
		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(fabric_ep.cq, NULL, 0);

		// debug_info("fi_tsend "<<ret);
	} while (ret == -FI_EAGAIN);
	
	if (ret){
		printf("error posting send buffer (%d)\n", ret);
		msg.error = -1;
		return msg;
	}

	debug_info("[FABRIC] [fabric_send] Waiting on mutex of rank_peer "<<fabric_comm.rank_peer);
	{
		std::unique_lock<std::mutex> lock(fabric_comm.comm_mutex);
		fabric_ep.subs_to_wait++;
		fabric_comm.comm_cv.wait(lock, [&fabric_comm]{ return !fabric_comm.wait_context; });
		fabric_comm.wait_context = true;
	}
	
	msg.size = size;
	msg.error = fabric_comm.context.entry.err;
	
	msg.tag = tag_send & 0x0000'0000'0000'FFFF;
	msg.rank_peer = (tag_send & 0xFFFF'FF00'0000'0000) >> 40;
	msg.rank_self_in_peer = (tag_send & 0x0000'00FF'FFFF'0000) >> 16;
	
  	debug_info("[FABRIC] [fabric_send] fabric_comm.context.entry.tag "<<fabric_comm.context.entry.tag);
  	debug_info("[FABRIC] [fabric_send] msg size "<<msg.size<<" rank_peer "<<msg.rank_peer<<" rank_self_in_peer "<<msg.rank_self_in_peer<<" tag "<<msg.tag<<" error "<<msg.error);
  	debug_info("[FABRIC] [fabric_send] End = "<<size);
	return msg;
}

fabric::fabric_msg fabric::recv ( fabric_ep &fabric_ep, fabric_comm& fabric_comm, void * buffer, size_t size, uint32_t tag )
{
	int ret;
	fabric_msg msg = {};

	uint64_t mask = 0;	
	uint64_t aux_rank_peer = fabric_comm.rank_peer;
	uint64_t aux_rank_self_in_peer = fabric_comm.rank_self_in_peer;
	uint64_t aux_tag = tag;
	uint64_t tag_recv = (aux_rank_self_in_peer << 40) | (aux_rank_peer << 16) | aux_tag;

	fabric_comm.context.rank = fabric_comm.rank_peer;

	if (fabric_comm.rank_peer == FABRIC_ANY_RANK){
		// mask = 0x0000'00FF'FFFF'0000;
		// mask = 0xFFFF'FF00'0000'0000;
		mask = 0xFFFF'FFFF'FFFF'0000;
	}

  	debug_info("[FABRIC] [fabric_recv] Start size "<<size<<" rank_peer "<<fabric_comm.rank_peer<<" rank_self_in_peer "<<fabric_comm.rank_self_in_peer<<" tag "<<tag<<" recv_context "<<(void*)&fabric_comm.context);
	do { 
		std::unique_lock<std::mutex> lock(fabric_ep.thread_fi_mutex);
		ret = fi_trecv(fabric_ep.ep, buffer, size, NULL, fabric_comm.fi_addr, tag_recv, mask, &fabric_comm.context);

		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(fabric_ep.cq, NULL, 0);
		// debug_info("fi_trecv "<<ret);
	} while (ret == -FI_EAGAIN);
	
	if (ret){
		printf("error posting recv buffer (%d)\n", ret);
		msg.error = -1;
		return msg;
	}
	
	debug_info("[FABRIC] [fabric_recv] Waiting on mutex of rank_peer "<<fabric_comm.rank_peer);
	{
		std::unique_lock<std::mutex> lock(fabric_comm.comm_mutex);
		fabric_ep.subs_to_wait++;
		fabric_comm.comm_cv.wait(lock, [&fabric_comm]{ return !fabric_comm.wait_context; });
		fabric_comm.wait_context = true;
	}

	msg.size = size;
	msg.error = fabric_comm.context.entry.err;
	
	msg.tag = fabric_comm.context.entry.tag & 0x0000'0000'0000'FFFF;
	msg.rank_self_in_peer = (fabric_comm.context.entry.tag & 0xFFFF'FF00'0000'0000) >> 40;
	msg.rank_peer = (fabric_comm.context.entry.tag & 0x0000'00FF'FFFF'0000) >> 16;
	
  	debug_info("[FABRIC] [fabric_recv] fabric_comm.context.entry.tag "<<fabric_comm.context.entry.tag);
  	debug_info("[FABRIC] [fabric_recv] msg size "<<msg.size<<" rank_peer "<<msg.rank_peer<<" rank_self_in_peer "<<msg.rank_self_in_peer<<" tag "<<msg.tag<<" error "<<msg.error);
  	debug_info("[FABRIC] [fabric_recv] End = "<<size);
	return msg;
}

int fabric::close ( fabric_ep& fabric_ep, fabric_comm &fabric_comm )
{
	int ret = 0;
	debug_info("[FABRIC] [fabric_close_comm] Start");

	std::unique_lock<std::mutex> lock(s_mutex);

	remove_addr(fabric_ep, fabric_comm);

	fabric_ep.m_comms.erase(fabric_comm.rank_peer);
		
	debug_info("[FABRIC] [fabric_close_comm] End = "<<ret);
	
	return ret;
}

int fabric::destroy ( fabric_ep &fabric_ep )
{
	int ret = 0;

	debug_info("[FABRIC] [fabric_destroy] Start");

	std::unique_lock<std::mutex> lock(s_mutex);

	destroy_thread_cq(fabric_ep);
  	
	debug_info("[FABRIC] [fabric_close_comm] Close endpoint");
	if (fabric_ep.ep){
		ret = fi_close(&fabric_ep.ep->fid);
		if (ret)
			printf("warning: error closing EP (%d)\n", ret);
		fabric_ep.ep = nullptr;
	}

	debug_info("[FABRIC] [fabric_close_comm] Close address vector");
	if (fabric_ep.av){
		ret = fi_close(&fabric_ep.av->fid);
		if (ret)
			printf("warning: error closing AV (%d)\n", ret);
		fabric_ep.av = nullptr;
	}
	
	debug_info("[FABRIC] [fabric_close_comm] Close completion queue");
	if (fabric_ep.cq){
		ret = fi_close(&fabric_ep.cq->fid);
		if (ret)
			printf("warning: error closing CQ (%d)\n", ret);
		fabric_ep.cq = nullptr;
	}
	
	debug_info("[FABRIC] [fabric_destroy] Close domain");
	if (fabric_ep.domain){
		ret = fi_close(&fabric_ep.domain->fid);
		if (ret)
			printf("warning: error closing domain (%d)\n", ret);
		fabric_ep.domain = nullptr;
	}

	debug_info("[FABRIC] [fabric_destroy] Close fabric");
	if (fabric_ep.fabric){
		ret = fi_close(&fabric_ep.fabric->fid);
		if (ret)
			printf("warning: error closing fabric (%d)\n", ret);
		fabric_ep.fabric = nullptr;
	}

	debug_info("[FABRIC] [fabric_destroy] Free hints ");
	if (fabric_ep.hints){
		fi_freeinfo(fabric_ep.hints);
		fabric_ep.hints = nullptr;
	}

	debug_info("[FABRIC] [fabric_destroy] Free info ");
	if (fabric_ep.info){
		fi_freeinfo(fabric_ep.info);
		fabric_ep.info = nullptr;
	}


	debug_info("[FABRIC] [fabric_destroy] End = "<<ret);

	return ret;
}

} // namespace XPN