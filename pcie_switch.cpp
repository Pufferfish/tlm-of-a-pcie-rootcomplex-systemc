//
//  pcie_switch.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//


#include "pcie_switch.h"

void PcieSwitch::PrintSendVCB() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < SWITCH_VCB_SIZE; j++) {
			cout << virtual_channel_upstream_send_buffers[i][j] << " ";

		}
		cout << endl;
	}
}

int PcieSwitch::ConvertPortToBusNr(int port_number_) {
	return downstream_port_map_to_bus_number[port_number_];
}

int PcieSwitch::SendToThisPort(int bus_number_) {

	for (unsigned port = 0; port < downstream_port_map_to_bus_number.size(); port++) {
		if (downstream_port_map_to_bus_number[port] == bus_number_) {
			return int(port);
		}
	}
	//past here means that the busnumber is located elsewhere

	for (unsigned port = 0; port < downstream_port_map_to_bus_number.size(); port++) { //Assume that bus numbers are put into downstream_port_map_to_bus_number with increasingly amount.
		if (downstream_port_map_to_bus_number[port] > bus_number_) { //We just went past it, assuming that all childs are greater in bus-number.
			return int(port - 1);
		}
	}
	//cout << "her er jeg"<<endl;
	return -1;
}

void PcieSwitch::SendFirstTLPAndUpdateVCBuffers(int traffic_class) { //Calls send and removes TLP from buffer

	TLP * tempBuffer = 0;
	if (print_full_simulation_information) {
		cout << endl
				<< "_______________________________SWITCHING_TLP_UPSTREAM_____________________"
				<< endl;

		cout << "|  PciSwitch ID " << conf_reg.busNumber
				<< conf_reg.deviceNumber << conf_reg.functionNumber
				<< " is recieving a TLP @ " << sc_time_stamp() << "\t\t\t\t"
				<< "  " << endl;

		cout << *virtual_channel_upstream_send_buffers[traffic_class][0]
				<< endl;
	}
	upstream_out = *virtual_channel_upstream_send_buffers[traffic_class][0]; //	SendTLP(); //sending

	//Count packets sent upstream
	packets_sent_upstream_counter_mutex.lock();
	*(this->packets_sent_upstream_counter)
			= *(this->packets_sent_upstream_counter) + 1;
	packets_sent_upstream_counter_mutex.unlock();

	delete virtual_channel_upstream_send_buffers[traffic_class][0];

	for (int i = 1; i < (SWITCH_VCB_SIZE); i++) { //collapse
		tempBuffer = virtual_channel_upstream_send_buffers[traffic_class][i];
		virtual_channel_upstream_send_buffers[traffic_class][i] = 0;
		virtual_channel_upstream_send_buffers[traffic_class][i - 1]
				= tempBuffer;
	}

}

//Help empty the vcb whenever there are packets in the queue and no notify from AddToSendVCB is sent
void PcieSwitch::SendVCBufferHandelingNotifyer() {
	while (true) {
		wait(CHECK_SEND_UPSTREAM_VCB_DELAY, SC_NS);
		packet_sent_to_send_vcb_event.notify();
	}
}

bool PcieSwitch::VirtualChannelBufferIsEmpty() {
	for (int traffic_class = 7; traffic_class >= 0; traffic_class--) {
		if (virtual_channel_upstream_send_buffers[traffic_class][0] != 0) {
			return false;
		}
	}
	return true;
}

void PcieSwitch::VirtualChannelBufferHandeling() {
	while (true) {
		wait(packet_sent_to_send_vcb_event);
		while (!VirtualChannelBufferIsEmpty()) {
			for (int traffic_class = 7; traffic_class >= 0; traffic_class--) { //VCB 7 is served first
				if (upstream_virtual_channel_buffer_mutex.trylock() != -1) {
					if (virtual_channel_upstream_send_buffers[traffic_class][0]
							!= 0) {
						SendFirstTLPAndUpdateVCBuffers(traffic_class);
					}
					upstream_virtual_channel_buffer_mutex.unlock();
				}
				//if(){ //fix problems with multiple packets in various queues
				wait(SEND_UPSTREAM_VCB_DELAY, SC_NS);//Note: NS! small delay to deal with bottleneck	//BOKMERKE What bottleneck? moved it down
				//}
			}
			//wait(SEND_UPSTREAM_VCB_DELAY, SC_NS);//Note: NS! small delay to deal with bottleneck
		}

	}

}

void PcieSwitch::InsertInUpstreamVirtualChannelBuffer(
		TLP* tlp_from_down_stream_ports) {

	int traffic_class = (tlp_from_down_stream_ports->get_header()->TC);
	for (int i = 0; i < SWITCH_VCB_SIZE; i++) {
		if (virtual_channel_upstream_send_buffers[traffic_class][i] == 0) {

			//cout << "SWITCH: inserting TLP at TC: " << traffic_class<< " i: " << i <<":"<< endl;
			upstream_virtual_channel_buffer_mutex.lock();

			//cout << "SWITCH: VCB Locked" << endl;
			virtual_channel_upstream_send_buffers[traffic_class][i]
					= tlp_from_down_stream_ports;

			upstream_virtual_channel_buffer_mutex.unlock();
			packet_sent_to_send_vcb_event.notify();
			break;
		}
	}
}

void PcieSwitch::PortBufferHandler() { //bokmerke ROUND ROBIN handler
	while (true) {
		//wait();		//remove?
		receive_downstream_port_buffer_mutex.lock(); //change with regular lock?

		for (unsigned i = 0; i < downstream_in_port_vector.size(); i++) {
			if (port_buffers_for_arbitration[i][0] != 0) { //Put in upstream VCB and colapse this buffer;
				InsertInUpstreamVirtualChannelBuffer(
						port_buffers_for_arbitration[i][0]);
				port_buffers_for_arbitration[i][0] = 0;
				for (int j = 1; j < PORT_ARBITRATION_BUFFER_SIZE; j++) { //Collapse Port arbitration buffers
					port_buffers_for_arbitration[i][j - 1]
							= port_buffers_for_arbitration[i][j]; //0 is equal to one, one is equeal to to etc..
				}
				port_buffers_for_arbitration[i][PORT_ARBITRATION_BUFFER_SIZE
						- 1] = 0;
			}

		}
		receive_downstream_port_buffer_mutex.unlock();

		wait(ROUND_ROBIN_DELAY, SC_NS); //Note NS! small delay to deal with bottleneck


	}

}

void PcieSwitch::receiveFromDownStreamAction() { //assuming that all downstream ports with incoming packets wants to send packets upstream, creats the nedd for port arbitration before a VCB arbitration at the upstream port

	//	cout <<"SWITCH receives @ : "<< sc_time_stamp()<<endl;

	receive_downstream_port_buffer_mutex.lock();
	for (unsigned i = 0; i < downstream_in_port_vector.size(); ++i) {

		//count packets received from downstream connections

		if (downstream_in_port_vector[i]->event()) {
			TLP * received_tlp = new TLP; //new header is generated in assignment consturctor


			*received_tlp = downstream_in_port_vector[i];
			packets_received_from_downstream_counter_mutex.lock();
			*(this->packets_received_from_downstream_counter)
					= *(this->packets_received_from_downstream_counter) + 1;
			packets_received_from_downstream_counter_mutex.unlock();

			for (int j = 0; j < PORT_ARBITRATION_BUFFER_SIZE; j++) { //Add to first available buffer location in the fifo buffer for each port, this is served by rr
				if (port_buffers_for_arbitration[i][j] == 0) {

					receive_downstream_port_buffer_mutex.lock();
					port_buffers_for_arbitration[i][j] = received_tlp; //Array buffers for ports outside the range are never used
					//cout <<"TLP is stored in port_buffers_for_arbitration"<<endl;
					receive_downstream_port_buffer_mutex.unlock();
					//cout << "HEY! @ "<<sc_time_stamp()<<endl;

					break;//break on first available bufferslot
				}
			}
		}
	}
	receive_downstream_port_buffer_mutex.unlock();
}

void PcieSwitch::receiveFromUpStreamAction() { //receive from upstream and route downstream in one function and process
	//assuming that the upstream port wants to send a packet downstreams(selvsagt?). No need for buffer, but needs a port arbitration
	//Have to address_memory the ports somehow. TODO Note: Lowest bus number is tied to the first element in the vector with signals
	//if mem req routing, address_memory routing, if cpl routing ID routing, since RC in this program only supports cpl ID routing is implemented here

	TLP * received_tlp = new TLP; //new header is generated in assignment consturctor
	*received_tlp = upstream_in; //Recieving form upstream

	//cout << *received_tlp<<endl;


	packets_received_from_upstream_counter_mutex.lock();
	*(this->packets_received_from_upstream_counter)
			= *(this->packets_received_from_upstream_counter) + 1;
	packets_received_from_upstream_counter_mutex.unlock();

	/*cout << *received_tlp<<endl;
	if (received_tlp->get_header()->type == CplD){
		cout<<*(dynamic_cast<CompletionHeader*> (received_tlp->get_header()))<<endl;
	}*/
	if (received_tlp->get_header()->type == CplD
			|| received_tlp->get_header()->type == Cpl) {
		//cout << *received_tlp<<endl;
		int
				forward_to_bus_nr =
						(dynamic_cast<CompletionHeader*> (received_tlp->get_header()))->reqIDBusNr; //2
		downstream_out_port_vector[SendToThisPort(forward_to_bus_nr)]
				= *received_tlp;

		packets_sent_downstream_counter_mutex.lock();
		*(this->packets_sent_downstream_counter)
				= *(this->packets_sent_downstream_counter) + 1;
		packets_sent_downstream_counter_mutex.unlock();

		if (print_full_simulation_information) {
			cout << endl
					<< "_______________________________SWITCHING_TLP_DOWNSTREAM___________________"
					<< endl;
			cout << "|  PciSwitch ID " << conf_reg.busNumber
					<< conf_reg.deviceNumber << conf_reg.functionNumber
					<< " forwards a packet to bus nr:" << forward_to_bus_nr
					<< " @ " << sc_time_stamp() << "\t\t\t\t" << "  " << endl;

			cout << *received_tlp << endl;
		}
		delete received_tlp;

	} else if (received_tlp->get_header()->type == MRd) { //Addressrouting forward or consume packet if intended receiver

		if (!conf_reg.IsAddressInBaseAddressRegisters(
				dynamic_cast<MemoryRequestHeader*> (received_tlp->get_header())->address1)) {

			int
					forward_to_port_nr =
							conf_reg.CheckBaseAndLimitThenForwardToPortNumber(
									dynamic_cast<MemoryRequestHeader*> (received_tlp->get_header())->address1);
			if (forward_to_port_nr != -1) {
				downstream_out_port_vector[forward_to_port_nr] = *received_tlp;

				packets_sent_downstream_counter_mutex.lock();
				*(this->packets_sent_downstream_counter)
						= *(this->packets_sent_downstream_counter) + 1;
				packets_sent_downstream_counter_mutex.unlock();

				if (print_full_simulation_information) {
					cout << endl
							<< "_______________________________SWITCHING_TLP_DOWNSTREAM___________________"
							<< endl;
					cout << "|  PciSwitch ID " << conf_reg.busNumber
							<< conf_reg.deviceNumber << conf_reg.functionNumber
							<< " forwards a packet to bus nr:"
							<< ConvertPortToBusNr(forward_to_port_nr) << " @ "
							<< sc_time_stamp() << "\t\t\t\t" << "  " << endl;

					cout << *received_tlp << endl;
				}
			}

			delete received_tlp;
		}

	} else if (received_tlp->get_header()->type == MWr) {	//TODO Might aswell be included in the previous if-else
		if (!conf_reg.IsAddressInBaseAddressRegisters(
				dynamic_cast<MemoryRequestHeader*> (received_tlp->get_header())->address1)) {
			int
					forward_to_port_nr =
							conf_reg.CheckBaseAndLimitThenForwardToPortNumber(
									dynamic_cast<MemoryRequestHeader*> (received_tlp->get_header())->address1);
			if (forward_to_port_nr != -1) {
				downstream_out_port_vector[forward_to_port_nr] = *received_tlp;
				packets_sent_downstream_counter_mutex.lock();
				*(this->packets_sent_downstream_counter)
						= *(this->packets_sent_downstream_counter) + 1;
				packets_sent_downstream_counter_mutex.unlock();
				if (print_full_simulation_information) {
					cout << endl
							<< "_______________________________SWITCHING_TLP_DOWNSTREAM___________________"
							<< endl;
					cout << "|  PciSwitch ID " << conf_reg.busNumber
							<< conf_reg.deviceNumber << conf_reg.functionNumber
							<< " forwards a packet to bus nr:"
							<< ConvertPortToBusNr(forward_to_port_nr) << " @ "
							<< sc_time_stamp() << "\t\t\t\t" << "  " << endl;
					cout << *received_tlp << endl;
				}
			}
			delete received_tlp;
		}
	}
}


