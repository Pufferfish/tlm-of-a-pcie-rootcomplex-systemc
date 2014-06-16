//
//  pci_endpoint.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "pcie_endpoint.h"

void PcieEndPoint::PrintEndpointSubSystemMemory() {
	cout << "\t" << "- - -Printing PCIe Endpoint Subsystem Memory- - -" << endl;
	for (int i = 0; i
			< (socket->configuration_register.base_address_registers_segment_limit[0]
					- socket->configuration_register.base_address_registers_segment_startpoints[0]); i
			= i + 30) { //Assumes that the confreg base register starts at 0 in the EP's memory

		cout << "\t\t" << endpoint_memory[i] << " " << endpoint_memory[i + 1]
				<< " " << endpoint_memory[i + 2] << " " << endpoint_memory[i
				+ 3] << " " << endpoint_memory[i + 4] << " "
				<< endpoint_memory[i + 5] << " " << endpoint_memory[i + 6]
				<< " " << endpoint_memory[i + 7] << " " << endpoint_memory[i
				+ 8] << " " << endpoint_memory[i + 9] << "  "
				<< endpoint_memory[i + 10] << " " << endpoint_memory[i + 11]
				<< " " << endpoint_memory[i + 12] << " " << endpoint_memory[i
				+ 13] << " " << endpoint_memory[i + 14] << " "
				<< endpoint_memory[i + 15] << " " << endpoint_memory[i + 16]
				<< " " << endpoint_memory[i + 17] << " " << endpoint_memory[i
				+ 18] << " " << endpoint_memory[i + 19] << "  "
				<< endpoint_memory[i + 20] << " " << endpoint_memory[i + 21]
				<< " " << endpoint_memory[i + 22] << " " << endpoint_memory[i
				+ 23] << " " << endpoint_memory[i + 24] << " "
				<< endpoint_memory[i + 25] << " " << endpoint_memory[i + 26]
				<< " " << endpoint_memory[i + 27] << " " << endpoint_memory[i
				+ 28] << " " << endpoint_memory[i + 29] << "  "<<endl;

		if (i > 200) {
			break;
		}
	}
}

void PcieEndPoint::SendQueueHandeling() { //make into a process that runs as long as there are data to be sent, if not, then chil and be sensitive to an event
	while (true) {
		wait();
		while (!send_data_to_software_layer_queue.empty()) { //It is very rare that this queue is bigger than one. when it is, one modelled nanosecond extra would be acceptable
			send_data_to_software_layer_queue_mutex.lock();

			if (print_data_sent_only_simulation_information) {
				cout << endl
						<< "__________________________________________________________________________"
						<< endl;
				cout << "|  ENDPOINT is sending data to transaction layer @ "
						<< sc_time_stamp() << "\t\t" << "         " << endl;
				cout << send_data_to_software_layer_queue.front() << endl;
			}

			if (send_data_to_software_layer_queue.front().type == MWr
					|| send_data_to_software_layer_queue.front().type == MRd) {

				requests_sent_counter_mutex.lock(); //lock for sent-counter, it is also used in the receive-action to compare sent vs received.
				*(this->requests_sent_counter) = *(this->requests_sent_counter)
						+ 1;
				if (print_full_simulation_information) {
					/*	cout << endl
					 << "--ENDPOINT SENDS: The number of request packets sendt are now "
					 << *(this->requests_sent_counter) << " @"
					 << sc_time_stamp() << endl;*/
				}
				sent_requests_timestamp.push_back(sc_time_stamp());
				requests_sent_counter_mutex.unlock();
			} else if (send_data_to_software_layer_queue.front().type == Cpl
					|| send_data_to_software_layer_queue.front().type == CplD) {

				wait(800, SC_NS);//Instead of the delay model in the RC, this will be the variable nature of the PCIe endpoint under test.

				completions_sent_counter_mutex.lock(); //lock for sent-counter, it is also used in the receive-action to compare sent vs received.
				*(this->completions_sent_counter)
						= *(this->completions_sent_counter) + 1;
				if (print_full_simulation_information) {
					cout << endl
							<< "--ENDPOINT SENDS: The number of completion packets sendt are now "
							<< *(this->completions_sent_counter) << " @"
							<< sc_time_stamp() << endl;
				}
				sent_completions_timestamp.push_back(sc_time_stamp());
				completions_sent_counter_mutex.unlock();
			}

			/////////////
			apiFromDevCore = send_data_to_software_layer_queue.front(); //Sending
			/////////////

			send_data_to_software_layer_queue.pop();
			send_data_to_software_layer_queue_mutex.unlock();

			//			cout << "lololol"<<endl;

			wait(1, SC_NS);

		}
	}
}

void PcieEndPoint::InitializePacketDataToSocket(SoftwareData& data_to_sw_layer,
		PacketType packet_send_type) {

	data_to_sw_layer.type = packet_send_type;//MWr;//MRd;
	data_to_sw_layer.address1 = 0;
	data_to_sw_layer.address2 = -1;
	data_to_sw_layer.length = endpoint_request_read_length;
	data_to_sw_layer.trafficClass = traffic_class_to_use;
	data_to_sw_layer.firstDWBE[0] = 1;
	data_to_sw_layer.firstDWBE[1] = 1;
	data_to_sw_layer.firstDWBE[2] = 1;
	data_to_sw_layer.firstDWBE[3] = 1;
	data_to_sw_layer.lastDWBE[0] = 0;
	data_to_sw_layer.lastDWBE[1] = 0;
	data_to_sw_layer.lastDWBE[2] = 0;
	data_to_sw_layer.lastDWBE[3] = 1;
	data_to_sw_layer.noSnoop = true;
	data_to_sw_layer.relaxedOrdering = true;
	data_to_sw_layer.EP = 0;

	if(packet_send_type==MWr){
		data_to_sw_layer.length = endpoint_request_write_length;
	}

}

void PcieEndPoint::SendRequestAction() {
	int address_counter = 0;
	PacketType packet_send_type = endpoint_spammer_type;
	//Creating an arbitrary collection of data from SW layer
	SoftwareData request_data_to_socket;
	InitializePacketDataToSocket(request_data_to_socket, packet_send_type);

	while (true) { //TODO change to for
		if ((*(this->requests_sent_counter)) == total_amount_of_requests) { //If enough packets are sent, dont send any more.
			if (total_amount_of_requests == 0) {
				break;
			}
			while (true) {
				wait(100000, SC_NS); //Or quit thread?
			}
		}

		simulation_tlp_number_counter_mutex.lock();
		simulation_tlp_number_counter++;
		request_data_to_socket.simulation_TLP_number=simulation_tlp_number_counter;
		simulation_tlp_number_counter_mutex.unlock();
		sent_request_tlp_number_vector.push_back(request_data_to_socket.simulation_TLP_number);


		if (request_data_to_socket.type == MWr) { //add data to write	 Computes PI with n nr_of_decimals

			if (endpoint_request_write_length > 1) {
				//send n decimals
				int nr_of_decimals = endpoint_request_write_length;//9 * endpoint_request_write_length;
				request_data_to_socket.length = nr_of_decimals;
				for(int i =0;i<endpoint_request_write_length;i++){
					request_data_to_socket.data.push_back(1);
				}
				//compute_n_decimals_of_pi(request_data_to_socket.data,	nr_of_decimals);
			} else if (endpoint_request_write_length == 1) {
				//send decimal nr n
				int decimal_number = *requests_sent_counter + 1;
				request_data_to_socket.length = 1;
				//cout << "getting data"<<endl;
				if (request_data_to_socket.data.size() == 0) {
					request_data_to_socket.data.push_back(
							compute_the_nth_decimal(decimal_number));
				} else {
					request_data_to_socket.data[0] = compute_the_nth_decimal(
							decimal_number);
				}
			}
		}
			request_data_to_socket.address1 = 48 + address_counter;
		send_data_to_software_layer_queue_mutex.lock();
		send_data_to_software_layer_queue.push(request_data_to_socket);
		send_data_to_software_layer_queue_mutex.unlock();
		send_data_to_software_layer_queue_event.notify();

		if (print_data_sent_only_simulation_information) {
			cout << endl
					<< "_________________________________________________________________________"
					<< endl;
			cout << "|  Endpoint: " << socket->configuration_register.busNumber
					<< socket->configuration_register.deviceNumber
					<< socket->configuration_register.functionNumber
					<< " is sending data to transaction layer @ "
					<< sc_time_stamp() << "\t" << "         " << endl;
			cout << request_data_to_socket << endl;
		}

		if (address_counter + 48 + request_data_to_socket.length > 1000) {
			address_counter = 0;
		}

		address_counter = address_counter + request_data_to_socket.length;
		//Send a new packet every, .... ns, set in Endpoint constructor
		wait(send_memory_read_request_period, SC_NS);
	}
}

void PcieEndPoint::receivePacketAction() { //TODO
	//while (true) {
	//wait();


	TLP * received_tlp = new TLP;
	*received_tlp = apiToDevCore;
	if (received_tlp->get_header()->type == MWr
			|| received_tlp->get_header()->type == MRd) {

		*(this->requests_received_counter) = *(this->requests_received_counter)
				+ 1;
		if (print_full_simulation_information) {
			cout << "--ENDPOINT" << socket->configuration_register.busNumber
					<< socket->configuration_register.deviceNumber
					<< socket->configuration_register.functionNumber
					<< "receiveS: The number of request packets received are now "
					<< *(this->requests_received_counter) << " @"
					<< sc_time_stamp() << endl;
		}
		received_requests_timestamp.push_back(sc_time_stamp());

	} else if (received_tlp->get_header()->type == Cpl
			|| received_tlp->get_header()->type == CplD) {
		//If byteCount is equal to the length of the packet, then it is the final packet, and its timestamp can be stored.
		total_received_completions_timestamp.push_back(sc_time_stamp());
		received_completion_tlp_number_vector.push_back(received_tlp->simulation_TLP_number);

		*(this->total_completions_received_counter)= *(this->total_completions_received_counter) + 1;
		if (dynamic_cast<CompletionHeader*> (received_tlp->get_header())->byteCount== dynamic_cast<CompletionHeader*> (received_tlp->get_header())->length) {
			received_completions_timestamp.push_back(sc_time_stamp());
			*(this->completions_received_counter)= *(this->completions_received_counter) + 1;
		}

		if (print_full_simulation_information) {
			cout << "--ENDPOINT " << socket->configuration_register.busNumber
					<< socket->configuration_register.deviceNumber
					<< socket->configuration_register.functionNumber
					<< " receiveS: The number of completion packets received are now "
					<< *(this->completions_received_counter) << " @"
					<< sc_time_stamp() << endl;
		}
	}

	if (print_data_sent_only_simulation_information) {
		cout << endl
				<< "_______________________________RECIEVING_TLP______________________________"
				<< endl;
		cout << "|  Endpoint is recieving a TLP @ " << sc_time_stamp()
				<< "\t\t\t\t" << "         " << endl;
		cout << *received_tlp << endl;
	}

	if ((received_tlp->get_header()->type == CplD)
			|| (received_tlp->get_header()->type == Cpl)) {
		if (!(received_tlp->get_data().empty())) {
			for (unsigned i = 0; i < (received_tlp->get_data()).size(); i++) {
				endpoint_received_data_vector.push_back(
						(received_tlp->get_data())[i]);
			}
		}
	} else if ((received_tlp->get_header()->type == MRd)) {//TODO add functionality for 64bit address, if desired, not required tho
		MemoryRequestHeader* incoming_memory_request_header =
				static_cast<MemoryRequestHeader*> (received_tlp->get_header());

		int system_memory_address = incoming_memory_request_header->address1;
		int read_length = incoming_memory_request_header->length;

		int
				endpoint_memory_address =
						system_memory_address
								- (socket->configuration_register.base_address_registers_segment_startpoints[0]);

		std::vector<int> return_data_from_mrd_request;
		for (int i = endpoint_memory_address; i < endpoint_memory_address
				+ read_length; i++) {
			return_data_from_mrd_request.push_back(endpoint_memory[i]);
		}

		SoftwareData completion_data_to_socket;
		completion_data_to_socket.simulation_TLP_number=received_tlp->simulation_TLP_number;
		completion_data_to_socket.cplStatus = SUCCESSFUL_COMPLETION;
		completion_data_to_socket.type = CplD;
		completion_data_to_socket.data = return_data_from_mrd_request;
		completion_data_to_socket.length = return_data_from_mrd_request.size();
		completion_data_to_socket.byteCount
				= (incoming_memory_request_header->length
						- completion_data_to_socket.length);
		completion_data_to_socket.tag = incoming_memory_request_header->tag;
		completion_data_to_socket.trafficClass
				= incoming_memory_request_header->TC;
		completion_data_to_socket.reqIDBusNr
				= incoming_memory_request_header->reqIDBusNr;
		completion_data_to_socket.reqIDDevNr
				= incoming_memory_request_header->reqIDDevNr;
		completion_data_to_socket.reqIDFuncNr
				= incoming_memory_request_header->reqIDFuncNr;
		send_data_to_software_layer_queue.push(completion_data_to_socket);
		send_data_to_software_layer_queue_event.notify();

	} else if ((received_tlp->get_header()->type == MWr)) { //TODO add functionality for 64bit address, if desired, not required tho

		MemoryRequestHeader* incoming_memory_request_header =
				static_cast<MemoryRequestHeader*> (received_tlp->get_header());
		//Convert to endpoint memory address, only a subset of the endpoints memory is mapped to the system memory
		int system_memory_address = incoming_memory_request_header->address1;
		int write_length = incoming_memory_request_header->length;
		int
				endpoint_memory_address =
						system_memory_address
								- (socket->configuration_register.base_address_registers_segment_startpoints[0]);
		//WRITE to memory
		for (int i = endpoint_memory_address; i < endpoint_memory_address
				+ write_length; i++) {
			//cout << i << "WRITING to EP memory"<<endl;
			endpoint_memory[i] = received_tlp->get_data()[i
					- endpoint_memory_address];
		}

	}
	delete received_tlp;
	//}
}
bool PcieEndPoint::TimeStampsAreSorted(){
	for(unsigned i=0;i<sent_request_tlp_number_vector.size();i++){
		if(sent_request_tlp_number_vector[i]!=received_completion_tlp_number_vector[i]){
			//move timestamp and index to the back of the list
			return false;
		}
	}
	return true;
}

