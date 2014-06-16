//
//  root_complex.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "root_complex.h"

void RootComplex::SendQueueHandeling() { //make into a process that runs as long as there are data to be sent, if not, then chil and be sensitive to an event
	while (true) {
		wait();
		while (!send_data_to_software_layer_queue.empty()) { //It is very rare that this queue is bigger than one. when it is, one modelled nanosecond extra would be acceptable
			send_data_to_software_layer_queue_mutex.lock();
			apiFromDevCore = send_data_to_software_layer_queue.front(); //Sending
			if (print_data_sent_only_simulation_information) {
				cout << endl
						<< "__________________________________________________________________________"
						<< endl;
				cout
						<< "|  ROOT COMPLEX is sending data to transaction layer @ "
						<< sc_time_stamp() << "\t\t" << "         " << endl;
				cout << send_data_to_software_layer_queue.front() << endl;
			}

			if (send_data_to_software_layer_queue.front().type == MRd
					|| send_data_to_software_layer_queue.front().type == MWr) {
				*(this->requests_sent_counter) = *(this->requests_sent_counter)
						+ 1;
				requests_sent_timestamps_vector.push_back(sc_time_stamp());

			} else if (send_data_to_software_layer_queue.front().type == Cpl
					|| send_data_to_software_layer_queue.front().type == CplD) {
				*(this->completions_sent_counter)
						= *(this->completions_sent_counter) + 1;
				completions_sent_timestamps_vector.push_back(sc_time_stamp());

			}

			send_data_to_software_layer_queue.pop();
			send_data_to_software_layer_queue_mutex.unlock();

			wait(1, SC_NS);

		}
	}
}

void RootComplex::InitializePacketDataToSocket(SoftwareData& data_to_sw_layer,
		PacketType packet_send_type) {
	//TODO Need to implement endpoint that is receiver

	data_to_sw_layer.type = rootcomplex_spammer_type;//MWr;//MRd;
	data_to_sw_layer.address1 = 0;
	data_to_sw_layer.address2 = -1;
	data_to_sw_layer.length = endpoint_request_read_length;
	data_to_sw_layer.trafficClass = requests_traffic_class;
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

	if (data_to_sw_layer.type == MWr) { //add data to write
		data_to_sw_layer.length = endpoint_request_write_length;
		for (int i = 0; i < data_to_sw_layer.length; i++) {
			data_to_sw_layer.data.push_back(1);
		}
	}
}

//Process runs all the time.
void RootComplex::RootComplexInitiateSendAction() { //TODO If RC wants to initiate anything on behalf of the cpu
	int address_counter =
			(socket->configuration_register.base_address_registers_segment_limit[0]) + 1; //first request is equal to the rc's memory address +1
	PacketType packet_send_type = rootcomplex_spammer_type;
	wait(1, SC_NS);
	//Creating an arbitrary collection of data from SW layer
	SoftwareData data_to_socket;
	InitializePacketDataToSocket(data_to_socket, packet_send_type);

	for (int i = 0; i < total_amount_of_requests; i++) {

		simulation_tlp_number_counter_mutex.lock();
		simulation_tlp_number_counter++;
		data_to_socket.simulation_TLP_number=simulation_tlp_number_counter;
		simulation_tlp_number_counter_mutex.unlock();

		data_to_socket.address1 = 20 + address_counter;

		//mutex
		send_data_to_software_layer_queue_mutex.lock();
		send_data_to_software_layer_queue.push(data_to_socket);
		send_data_to_software_layer_queue_mutex.unlock();
		send_data_to_software_layer_queue_event.notify();
		//apiFromDevCore = data_to_socket;
		address_counter = address_counter + data_to_socket.length; //preparing next address_memory request

		wait(send_request_period, SC_NS);
	}
	//}
	while (true) {
		wait(100000, SC_NS); //Or quit thread?
	}

}

void RootComplex::MemoryDataEventSetter() {//Notify that new data is available on the memory connection
	while (true) {

		wait();
		//cout <<"NotifyingRead @ "<< sc_time_stamp() <<endl;
		if (data_from_memory != -1) {
			data_read_from_memory_available_event.notify();
		}
	}
}

void RootComplex::WriteMemoryConfirmationWatcher() {
	while (true) {
		wait();
		//Notify the function writing data to memory, in order to let it progress
		//cout <<"NotifyingWriteSuccess @ "<< sc_time_stamp()<<endl;
		data_was_successfully_written_event.notify();
	}
}

//All mem writes are posted transactions, no cpl packets are returned to sender
void RootComplex::WriteMemory(
		MemoryRequestHeader incoming_memory_request_header,
		std::vector<int> data_to_write) {

	enable_memory = true; //Turn on memory
	write_memory = true; //true is write false is read, select write

	for (int i = 0; i < incoming_memory_request_header.length; i++) { //Read length-doublewords and add them to the temporary vector when they become available on the data-bus
		data_to_memory = data_to_write[i];
		address_memory = (incoming_memory_request_header.address1 + i);
		//cout <<"derp0"<<endl;
		//if(i!=incoming_memory_request_header.length-1){
		wait(data_was_successfully_written_event);//Imperative to have a delay here, or use a reading-completed signal to let the memory read off the tomembus
		//}
		//wait(1,SC_NS);
		//cout <<"derp"<<endl;
	}

	enable_memory = false; //Turn off memory
	//wait(MEMORY_READ_DELAY_PER_WORD, SC_NS);	//TODO Delay! Memory read delay per length, not word per word should be inside memory-block
}

int RootComplex::WaitModeledRandomSample() {
	int xRan;
	xRan = rand() % delay_samples_vector.size(); // Randomizing the number between 0 and the size of the sample-vector.
	//cout << xRan<<endl;
	return delay_samples_vector[xRan];
}

bool RootComplex::ReadMemory(int address, int length, SoftwareData * completionData) {

	std::vector<int> tempVectorInt;
	bool memTransferSuccess;

	enable_memory = true; //Turn on memory
	write_memory = false; //true is write false is read, select read
	for (int i = 0; i < length; i++) { //Read length-doublewords and add them to the temporary vector when they become available on the data-bus
		address_memory = (address + i);
		//cout << "Read"<<endl;
		wait(data_read_from_memory_available_event); //Event: There is data from the read request on the mem-bus
		tempVectorInt.push_back(data_from_memory);
		//cout << tempVectorInt[i]<<endl;
	}

	if ((int) (tempVectorInt.size()) == (length)) { //Checks whether the length of the data-vector corresponds to the requested length
		memTransferSuccess = true; //TODO implement a more advanced mem-chcek?
		completionData->length = tempVectorInt.size();
		completionData->data = tempVectorInt;
		completionData->type = CplD; //If memcheck is okey, return the dataFromSWLayer-formated object, marked with the type Completion with data.

	} else {
		completionData->type = Cpl;
		memTransferSuccess = false;
	}
	enable_memory = false; //Turn on memory

	//wait(MEMORY_READ_DELAY_PER_WORD, SC_NS);	//TODO Delay! Memory read delay per length, not word per word should be inside memory-block
	return memTransferSuccess;
}

void RootComplex::HandleTlpQueue() {
	//cout<<"derp1"<<endl;
	while (true) {
		wait(); //trigged by new_tlp_in_incoming_tlp_queue
		while (!incoming_tlp_queue.empty()) { //empty the queue

			handle_tlp_in_incoming_tlp_queue.notify(); //handle the first element in the queue
			wait(tlp_is_handeled); //once the previous tlp is handeled, run one more check, and handle if not empty.
		}
	}
}

void RootComplex::InsertTlpInQueue() { //receive insert and notify
	TLP * incoming_tlp = new TLP;
	*incoming_tlp = apiToDevCore;
	if (print_data_sent_only_simulation_information) {
		cout << endl
				<< "__________________________________________________________________________"
				<< endl;
		cout << "|  ROOT COMPLEX is recieving a TLP @ " << sc_time_stamp()
				<< "\t\t\t" << "         " << endl;
		cout << *incoming_tlp << endl;
	}

	//cout << "NOTIFYING QUEUE";

	if (incoming_tlp->get_header()->type == MRd
			|| incoming_tlp->get_header()->type == MWr) {
		*(this->requests_received_counter) = *(this->requests_received_counter)
				+ 1;
		requests_received_timestamps_vector.push_back(sc_time_stamp());

	} else if (incoming_tlp->get_header()->type == Cpl
			|| incoming_tlp->get_header()->type == CplD) {
		*(this->completions_received_counter)
				= *(this->completions_received_counter) + 1;
		completions_received_timestamps_vector.push_back(sc_time_stamp());

	}

	incoming_tlp_queue.push(incoming_tlp);
	new_tlp_in_incoming_tlp_queue.notify();
}

int RootComplex::NumberOfCompletions(MemoryRequestHeader request){
	//If completion crosses a cacheline, perform completion splitting, ReadCompletionBoundary
	//Or if request is above the maximum payload size //This is sort of built into the previous one as the RCB is allays less than the MPS
	int completion_number;
	if((((request.address1 + request.length)/read_completion_boundary) !=(request.address1)/read_completion_boundary) ){
		completion_number= 1 + ((request.address1 + request.length)/read_completion_boundary)-((request.address1)/read_completion_boundary);
	}else{
		completion_number= 1;
	}
	return completion_number;
}

bool RootComplex::IsInSendTimeVector(sc_time check_time_stamp){
	if(std::find(send_time_vector.begin(), send_time_vector.end(), check_time_stamp) != send_time_vector.end()) {
	    /* v contains x */ return true;
	} else {
	    /* v does not contain x */return false;
	}
}
int RootComplex::ReturnIndexOfData(sc_time check_time_stamp){
	  std::vector<sc_time>::iterator iter;
	  //while(iter!=send_time_vector.size){ //can be multiple scheduled sends to one time_stamp
		  iter = std::find(send_time_vector.begin(), send_time_vector.end(), check_time_stamp);
			size_t index = std::distance(send_time_vector.begin(), iter);

			if(index == send_time_vector.size())
		   {
				return -1;
		   }
	  //}
	    return index;
}

void RootComplex::SendCplDRecieveMultitaskingQueueHandling(){ //Allows multitasking of the recieverthread

	while (true) {
		wait(elements_in_multitasking_queue);
		while(!(send_cpl_data_to_software_layer_multitasking_vector.empty())){
			wait(1,SC_NS);	//evaluate every 1 ns
			if(IsInSendTimeVector(sc_time_stamp())){		//				wait(WaitModeledRandomSample(), SC_NS); //no need for removing it
				send_cpl_data_to_software_layer_multitasking_vector_mutex.lock();
				//std::vector<sc_time>::iterator iter=send_time_vector.begin();
				//while(iter!=std::end(send_time_vector)){ //can be multiple scheduled sends to one time_stamp
				for(unsigned i =0;i <send_time_vector.size() ;i++){
					if(send_time_vector[i]==sc_time_stamp()){
						send_data_to_software_layer_queue_mutex.lock();
						//cout << send_cpl_data_to_software_layer_multitasking_vector.front()<<endl;
						send_data_to_software_layer_queue.push(send_cpl_data_to_software_layer_multitasking_vector[i]);//ReturnIndexOfData(sc_time_stamp())]);		//her
						send_data_to_software_layer_queue_mutex.unlock();
						send_data_to_software_layer_queue_event.notify();
					}
				}
				send_cpl_data_to_software_layer_multitasking_vector_mutex.unlock();
			}
		}
	}

}

void RootComplex::RootComplexreceiveAction() { //TODO handle other packettypes than Mrd
	//cout<<"derp3"<<endl;
	while (true) {
		wait(handle_tlp_in_incoming_tlp_queue); //Sensitive to TLP on apiToDevCore, stripped from ECRC		//TODO Are we in transaction-layer still? or sw layer

		TLP* incoming_tlp = incoming_tlp_queue.front();
		incoming_tlp_queue.pop();

		MemoryRequestHeader* incMemReqHeader;
		CompletionHeader* incCplHeader;

		int headerType = incoming_tlp->get_header()->type;//incTLPHeader->type;		//Find the packet type


		switch (headerType) //cannot switch on an enum, it is considered to be a const
		{
		case MRd: //Reply with a completion package
		{
			//cout << "--Incoming TLP is a Memory Read request" << endl;

			incMemReqHeader
					= static_cast<MemoryRequestHeader*> (incoming_tlp->get_header());
			//cout << *incMemReqHeader<<endl;
			if (incMemReqHeader == 0) {
				cout << "Casting To IncMemReqHeader failed" << endl;
			}
			//cout << incMemReqHeader << endl;


			//cout <<incMemReqHeader->address1<<" "<<incMemReqHeader->length<<endl;

			int remaining_completion_bytes=incMemReqHeader->length;
			int completion_bytes_sent=0;
			int read_address;
			int read_length;
			for(int i =0;i<NumberOfCompletions(*incMemReqHeader);i++){

				SoftwareData completionData;

				if (print_data_sent_only_simulation_information||print_full_simulation_information) {
					cout
							<< "--//////////////////////////////////////////////// COMMENCING MEMORYREADING ////////////////////////////////////////////////"
							<< endl;
				}



				if(i==0){
					read_address=incMemReqHeader->address1;
					if(NumberOfCompletions(*incMemReqHeader)==1){	//no more iterations
						read_length=incMemReqHeader->length;
					}else{ //more iterations
						//Finds the next read boundary and finds the length between the read boundary and the read address.
						read_length= read_completion_boundary*((read_address /read_completion_boundary)+1)-read_address;
						//cout <<endl<<read_length<<endl;
						//read_length=remaining_completion_bytes; //until next rcb
					}
				}else{
					//read_address=incMemReqHeader->address1 + i*read_completion_boundary;
					read_address = read_address+read_length;

					//cout <<endl<<endl<< read_address<<endl<<endl;
					if((remaining_completion_bytes/read_completion_boundary) > 0){	//pass another boundary

						read_length= read_completion_boundary*((read_address /read_completion_boundary)+1)-read_address;
					}else{//length is equal to the remaining length
						read_length=remaining_completion_bytes;
					}
				}

				bool memSuccess = ReadMemory(read_address, read_length, &completionData);
				//wait(MEMORY_ACCESS_DELAY, SC_NS);
				if (print_data_sent_only_simulation_information||print_full_simulation_information) {
					cout
							<< "--//////////////////////////////////////////////// DONE WITH MEMORYREADING  ////////////////////////////////////////////////"
							<< endl;
				}
				if (!memSuccess) {
					cout << "Memory failure" << endl; //TODO handle, send it anyways, if a failure then type is set to be Cpl within the memread-function
				} else {
					completionData.cplStatus = SUCCESSFUL_COMPLETION;

				}


				completionData.tag = incMemReqHeader->tag;
				//cout <<endl<<"***************************************************"<< " DATAFROMDEVCORE TAG IS EQUAL TO  "<<incMemReqHeader->tag<< completionData.tag<<endl;
				completionData.trafficClass = incMemReqHeader->TC;
				completionData.reqIDBusNr = incMemReqHeader->reqIDBusNr;
				//cout << incMemReqHeader->reqIDBusNr;
				completionData.reqIDDevNr = incMemReqHeader->reqIDDevNr;
				completionData.reqIDFuncNr = incMemReqHeader->reqIDFuncNr;
				completionData.byteCount =  remaining_completion_bytes;
				completionData.simulation_TLP_number=incoming_tlp->simulation_TLP_number;	//Cpl and CPLd are assigned their requests' simulation tlp number

				 remaining_completion_bytes= remaining_completion_bytes-completionData.length;
				 completion_bytes_sent=completion_bytes_sent+completionData.length;



				 if(i==0){
					 if(!rc_multitasking_is_enabled){
						 wait(WaitModeledRandomSample(), SC_NS); //DELAY from tracedata
					 }else{	//allows the RC recievehandler to multitask time_sc = sc_time((double)WaitModeledRandomSample(),time_u)
						 int wait_sample=WaitModeledRandomSample();
						 wait_sample_vector_for_printouts.push_back(wait_sample);
						 sc_time wait_time = sc_time((double)wait_sample,SC_NS);
						 sc_time send_time = wait_time+ sc_time_stamp();

						 send_time_vector.push_back(send_time);

						 send_cpl_data_to_software_layer_multitasking_vector_mutex.lock();
						 send_cpl_data_to_software_layer_multitasking_vector.push_back(completionData); //each data is associated with a timestamp at the corresponding vector index
						 send_cpl_data_to_software_layer_multitasking_vector_mutex.unlock();

						 elements_in_multitasking_queue.notify();
					 }
				 }else{
					 wait(1,SC_NS);
				 }

				 if(!rc_multitasking_is_enabled){
					//mutex
					send_data_to_software_layer_queue_mutex.lock();
					send_data_to_software_layer_queue.push(completionData);
					send_data_to_software_layer_queue_mutex.unlock();
					send_data_to_software_layer_queue_event.notify();
					//apiFromDevCore = data_to_socket;
				 }
			}
			delete incoming_tlp;

		}
			break;

		case MRdLk: //////////WRITE/////////
			cout << "Memory Read Lock" << endl; // le croy kan kjoere paa egen pc

			break;

		case MWr: { //TODO
			//cout << "Memory Write" << endl;
			incMemReqHeader
					= static_cast<MemoryRequestHeader*> (incoming_tlp->get_header());
			if (incMemReqHeader == 0) {
				cout << "Casting To IncMemReqHeader failed" << endl;
			}

			std::vector<int> incoming_data = incoming_tlp->get_data(); //Get data to write
			if (print_data_sent_only_simulation_information||print_full_simulation_information) {
				cout
						<< "--//////////////////////////////////////////////// COMMENCING MEMORYWRITING ////////////////////////////////////////////////"
						<< endl;
			}
			WriteMemory(*incMemReqHeader, incoming_data);
			//wait(MEMORY_ACCESS_DELAY, SC_NS);
			if (print_data_sent_only_simulation_information||print_full_simulation_information) {
				cout
						<< "--//////////////////////////////////////////////// DONE WITH MEMORYWRITING  ////////////////////////////////////////////////"
						<< endl;
			}

			//*(this->requests_sent_counter)=*(this->requests_sent_counter)+1;
			delete incoming_tlp;

		}
			break;

		case IORd: //IO requests Not implemented
			cout << "IO read" << endl;
			break;
		case IOWr:
			cout << "IO Write" << endl;
			break;

		case CfgRd0: //Configuration requests Not implemented
			cout << "Configure Read 0" << endl;
			break;
		case CfgRd1:
			cout << "Configure Read 1" << endl;
			break;
		case CfgWr0:
			cout << "Configure Write 0" << endl;
			break;
		case CfgWr1:
			cout << "Configure Write 1" << endl;
			break;

		case Msg: //Messages are not implemented
			cout << "Message" << endl;
			break;
		case MsgD:
			cout << "Message with Data" << endl;
			break;

		case Cpl: {
			incCplHeader
					= dynamic_cast<CompletionHeader*> (incoming_tlp->get_header());
			cout << "Completion without Data @: " << sc_time_stamp() << endl;
			cout << *incCplHeader << endl;
		}
			break;
		case CplD: {

			incCplHeader
					= dynamic_cast<CompletionHeader*> (incoming_tlp->get_header());
			std::vector<int> data_received_from_tlp = incoming_tlp->get_data();

			for (unsigned i = 0; i < data_received_from_tlp.size(); i++) {
				received_data_vector.push_back(data_received_from_tlp[i]);
			}
			if (print_data_sent_only_simulation_information) {
				cout << "Completion with Data @: " << sc_time_stamp() << endl;
				cout << *incCplHeader << endl;
			}
		}
			break;
		case CplLk:
			cout << "Completion lock" << endl;
			break;
		case CplDLk:
			cout << "Completion lock with Data" << endl;
			break;
		default:
			cout << "--Incoming TLP is defaulted in switch-case" << endl;
			break;
		}
		// wait(1,SC_NS);
		tlp_is_handeled.notify();
	}

}
