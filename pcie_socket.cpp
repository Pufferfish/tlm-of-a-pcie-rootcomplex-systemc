//
//  pcie_socket.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "pcie_socket.h"

#define CHECK_SEND_VCB_DELAY 400							//Check the sendVCbuffer every X ns				//TODO move defines into PCIeAPI.h
#define CHECK_receive_VCB_DELAY 400							//A new TLP is withdrawn from the VCB every X ns
#define RENEW_FIRST_REQUEST_IN_OUTBOUNDQUEUE 4000000		//Request expires after X ns

///////////////////////////////////
////////////GENERAL////////////////
///////////////////////////////////
bool PcieSocket::ContainsData(SoftwareData data) { //New after implementing PCIeEP
	int TLPType = data.type;
	if ((TLPType == MWr) || (TLPType == IOWr) || (TLPType == CfgWr0)
			|| (TLPType == CfgWr1) || (TLPType == MsgD) || (TLPType == CplD)
			|| (TLPType == CplDLk)) {
		return true;
	} else {
		return false;
	}
}

bool PcieSocket::IsRequestPacket(int TLPType) {
	if ((TLPType == MRd) || (TLPType == MRdLk)  || (TLPType
			== Msg) || (TLPType == MsgD)) { //TODO are these all of them? || (TLPType == MWr)
		return true;
	} else {
		return false;
	}
}

void PcieSocket::PrintSendVCB() {
	for (int i = 0; i < 8; i++) {
		cout << "TC= " << i << ": ";
		for (int j = 0; j < SEND_TLP_BUFFER_SIZE; j++) {
			cout << virtual_channel_send_buffers[i][j] << " ";
		}
		cout << endl;
	}
}

void PcieSocket::PrintreceiveVcb() {
	for (int i = 0; i < 8; i++) {
		cout << "TC= " << i << ": ";
		for (int j = 0; j < receive_TLP_BUFFER_SIZE; j++) {
			cout << virtual_channel_receive_buffers[i][j] << " ";
		}
		cout << endl;
	}

}

void PcieSocket::PrintOutboundBuffer() {

	cout << "********************PRINTING OUTBOUND QUEUE***********************"<< endl;
	for (int i = 0; i < 32; i++) { //BOKMERKE TODO Change to 32
		//if(outbound_requests[i]!=0){

		if (outbound_requests[i] != 0) {
			cout << "Index: " << i << ", TLP adress: " << outbound_requests[i]
					<< ", Tag: ";
			cout
					<< (dynamic_cast<MemoryRequestHeader*> (outbound_requests[i]->get_header()))->tag;
			cout << endl;
		}

	}
	cout << endl;
	cout << "*****************************************************************"<<endl;
}

///////////////////////////////////
////////////SENDER///////////////
///////////////////////////////////

//Tied directly to sendVCBufferHandeling//
void PcieSocket::SendTLP(TLP * TLPToBeSent) {


	send_tlp_port = *TLPToBeSent; //SENDING to sc_out
	*(this->packets_sent_counter) = *(this->packets_sent_counter) + 1;

	if (print_full_simulation_information) {
		cout << endl
				<< "_______________________________SENDING_TLP________________________________"
				<< endl;

		if (configuration_register.busNumber == 0 && configuration_register.deviceNumber == 0
				&& configuration_register.functionNumber == 0) {//API owner is root complex
			cout << "|  ROOT COMPLEX is sending a TLP @ " << sc_time_stamp()
					<< "\t\t\t" << "          ";
		} else {
			cout << "|  PCIdevice ID " << configuration_register.busNumber
					<< configuration_register.deviceNumber << configuration_register.functionNumber
					<< " is sending a TLP @ " << sc_time_stamp() << "\t\t\t"
					<< "         |";
		}
		cout << endl;
	}
	if(print_full_simulation_information){
		cout << *TLPToBeSent<<endl;
	}

	if(TLPToBeSent->get_header()->type == MWr||TLPToBeSent->get_header()->type == CplD ||TLPToBeSent->get_header()->type == Cpl){
		delete TLPToBeSent; //Delete TLP if it is a Cpl packet, because its not put in outbound request buffer.
	}
}

void PcieSocket::SendFirstTLPAndUpdateVCBuffers(int trafficClass) { //Calls send and removes TLP from buffer

	TLP * tempBuffer = 0;
	SendTLP(virtual_channel_send_buffers[trafficClass][0]); //sending
	virtual_channel_send_buffers[trafficClass][0] = 0;

	for (int i = 1; i < (SEND_TLP_BUFFER_SIZE); i++) {
		tempBuffer = virtual_channel_send_buffers[trafficClass][i];
		virtual_channel_send_buffers[trafficClass][i] = 0;
		virtual_channel_send_buffers[trafficClass][i - 1] = tempBuffer;
	}

}

//Help empty the vcb whenever there are packets in the queue and no notify from AddToSendVCB is sent
void PcieSocket::SendVCBufferHandelingNotifyer() {
	while (true) {
		wait(CHECK_SEND_VCB_DELAY, SC_NS);
		packet_sent_to_send_vcb_event.notify();
	}
}

//TODO* Credits
void PcieSocket::SendVCBufferHandeling() { //Thread for sending continiously from buffers
	while (true) {
		wait(packet_sent_to_send_vcb_event);
		for (int tC = 7; tC >= 0; tC--) { //VCB 7 is served first
			send_buffer_mutex.lock();
			if (virtual_channel_send_buffers[tC][0] != 0) {

				SendFirstTLPAndUpdateVCBuffers(tC);
			}
			send_buffer_mutex.unlock();
			//wait(SEND_VCB_DELAY, SC_NS);
		}
		//wait(CHECK_SEND_VCB_DELAY, SC_NS);
	}
}

//Adds Packet to the send-virtual-channel-buffer
bool PcieSocket::AddToSendVCB(TLP * TLPPointer) {
	int TC = (TLPPointer->get_header()->TC);

	for (int i = 0; i < SEND_TLP_BUFFER_SIZE; i++) {
		if (virtual_channel_send_buffers[TC][i] == 0) {
			//cout << "PCI-Socket: -inserting TLP at TC: " << TC << " i: " << i << endl;
			send_buffer_mutex.lock();
			virtual_channel_send_buffers[TC][i] = TLPPointer;
			send_buffer_mutex.unlock();
			packet_sent_to_send_vcb_event.notify();
			return true;
		}
	}
	return false; //TODO handle full buffers
}

int PcieSocket::InsertOutboundRequests(TLP * TLPpointer) {


	//split transaction model several requests.....
	for (int requestNumber = 0; requestNumber < 32; requestNumber++) {
		if ((outbound_requests[requestNumber]) == 0) {
			outbound_requests[requestNumber] = TLPpointer; //store the pointer of the tlp under construction
			//cout << "Inserting request - packet in outbound request buffer"<<endl;
			add_tlp_to_outbound_request_buffer_event.notify();
			return requestNumber; //Tag is equal to its inserted address_memory in the outbound buffer
		}
	}
	return 33; //TODO handle full outbound buffer
}

TLP* PcieSocket::BuildTLP(SoftwareData dataFromDevCore) {




	Format fmt;
	//Set the format
	if ((dataFromDevCore.address2 == -1) && (!ContainsData(dataFromDevCore))) { //Use 32bit addressing
		fmt = DW_3_WO_Data;
	} else if ((dataFromDevCore.address2 == -1) && (ContainsData(
			dataFromDevCore))) {
		fmt = DW3_W_Data;
	} else if ((dataFromDevCore.address2 != -1) && (ContainsData(
			dataFromDevCore))) {
		fmt = DW_4_W_Data;
	} else {
		fmt = DW_4_WO_Data;
	}
	int packet_tag = -1; //TODO remove this sentence, and minus 1 starting off as a non-requestpackage


	//Create the new TLP
	TLP * outgoingTLP = new TLP;
	if (IsRequestPacket(dataFromDevCore.type)) { //Insert in outbound requests and get the tag
		packet_tag = InsertOutboundRequests(outgoingTLP); //Tag is the location in the outboundbuffer		//TODO need to create another copy of the TLP for this purpose, or delete the Cpl one later on.
	}


	//TODO handle if tag is equal to 33, that is the outboundbuffer is full

	//	TLPHeader header;
	if (dataFromDevCore.type == MRd || dataFromDevCore.type == MWr) {
		MemoryRequestHeader* memReqHeader = new MemoryRequestHeader;//BOKMERKE

		memReqHeader->SetMemoryRequestHeader(dataFromDevCore.length,
				dataFromDevCore.noSnoop, dataFromDevCore.relaxedOrdering,
				dataFromDevCore.EP, configuration_register.useECRCWhenSending,
				dataFromDevCore.trafficClass, dataFromDevCore.type, fmt,
				dataFromDevCore.firstDWBE, dataFromDevCore.lastDWBE,
				packet_tag, configuration_register.busNumber, configuration_register.deviceNumber,
				configuration_register.functionNumber, dataFromDevCore.address1,
				dataFromDevCore.address2);
		outgoingTLP->set_header(memReqHeader); //upcasts by setting the header of the outgoing TLP to the MemReqHeader

	} else if (dataFromDevCore.type == CplD || dataFromDevCore.type == Cpl) {

		//cout << "building completion header!"<<endl;

		CompletionHeader* cplHeader = new CompletionHeader;
		//void setCplHeader(int slength, bool snoSnoop, bool srelaxedOrdering, bool sEP, bool sTD, int sTC, PacketType stype, Format sfmt, int sbyteCount, bool sbcm,CplStatus scplStatus, int scplIDBusNr,int scplIDDevNr,int scplIDuncNr,int slowerAddress,int stag,int sreqIDBusNr,int sreqIDDevNr,int sreqIDFuncNr);
		cplHeader->SetCompletionHeader(dataFromDevCore.length,
				dataFromDevCore.noSnoop, dataFromDevCore.relaxedOrdering,
				dataFromDevCore.EP, configuration_register.useECRCWhenSending,
				dataFromDevCore.trafficClass, dataFromDevCore.type,
				DW_3_WO_Data, dataFromDevCore.byteCount, dataFromDevCore.bcm,
				dataFromDevCore.cplStatus, configuration_register.busNumber,
				configuration_register.deviceNumber, configuration_register.functionNumber,
				dataFromDevCore.lowerAddress, dataFromDevCore.tag,
				dataFromDevCore.reqIDBusNr, dataFromDevCore.reqIDDevNr,
				dataFromDevCore.reqIDFuncNr);
		outgoingTLP->set_header(cplHeader);
	}

	//TODO add ECRC also
	outgoingTLP->set_data(dataFromDevCore.data);
	outgoingTLP->simulation_TLP_number=dataFromDevCore.simulation_TLP_number;

	return outgoingTLP;
}

void PcieSocket::SendAction() { //Is triggered by data from the SWlayer or by
	//TODO
	//Transmitter keeps track of time and removes the packet from the input-port//set it high for the amount of time required to transmit 4dws
	while (true) {
		wait(); //Sensitive to and Triggered by data on from_device_core
		SoftwareData incData = from_device_core;

		if(incData.type==MWr){ //check write length vs max_payload_size
			int number_of_packets=1;
			int requested_transfer_length=incData.length;
			int requested_transfer_address=incData.address1;
			std::vector <int> requested_data_to_write=incData.data;
			number_of_packets=((incData.length) / max_payload_size)+1; //integer division
			int bytes_sent=0;

			for(int i =0;i<number_of_packets;i++){
				if(i==0){
					incData.address1=incData.address1;
				}else{
					incData.address1=incData.address1+max_payload_size;
				}
				if(i<(number_of_packets-1)){ //packets inbetween start and end
					incData.length=max_payload_size;
				}else{
					incData.length=requested_transfer_length-i*max_payload_size;
				}
				cout << incData.length<<" "<<incData.address1<<endl;

				if(number_of_packets>1){
					incData.data.clear();
					for(int j =0;j<incData.length;j++){
						//cout << j<<endl;
						cout << requested_data_to_write[(incData.address1-requested_transfer_address) + j];
						incData.data.push_back(requested_data_to_write[(incData.address1-requested_transfer_address) + j]);
					}
					cout <<endl;
				}

				TLP *outgoingTLP = BuildTLP(incData); //build the TLP and store it in the Outbound if its a nonposted
				AddToSendVCB(outgoingTLP); //ADD TO SENDQUEUE Send down to linklayer and away


				bytes_sent=bytes_sent+incData.length;
			}

		}else{
			TLP *outgoingTLP = BuildTLP(incData); //build the TLP and store it in the Outbound if its a nonposted

			AddToSendVCB(outgoingTLP); //ADD TO SENDQUEUE Send down to linklayer and away

		}


	}
}

///////////////////////////////////
////////////receiveR///////////////
///////////////////////////////////

void PcieSocket::RenewRequest() {
	TLP * renewedTLP = new TLP;
	*renewedTLP = *(outbound_requests[0]); //TODO add semaphore eller er det vits naar det er en traad ? er det en traad?
	RemoveOutboundRequest(0); //removes the first instance of the TLP and colapse the fifo


	int newTag = InsertOutboundRequests(renewedTLP); //Inserts at the back of the fifo

	MemoryRequestHeader* tempHeader =dynamic_cast<MemoryRequestHeader*> (renewedTLP->get_header());
		tempHeader->tag = newTag; //TODO RENAME TEMPHEADER		//tag cant be 33 since this is supposed to be semaphored
		outbound_requests[0]->set_header(tempHeader);
	AddToSendVCB(renewedTLP);

}

void PcieSocket::OutboundRequestHandeling() {//TODO FIFO
	while (true) {

		wait(add_tlp_to_outbound_request_buffer_event);

		while (outbound_requests[0] != 0) { //Element in spot number zero is allways the oldest one
			//cout << "Waiting for outbound request."<<endl;
//			PrintOutboundBuffer();
			wait(RENEW_FIRST_REQUEST_IN_OUTBOUNDQUEUE, SC_NS); //Wait until first element expires
			cout << "Outbound request timeout @ "<< RENEW_FIRST_REQUEST_IN_OUTBOUNDQUEUE <<" ns, renewing request." << endl;
			if (outbound_requests[0] == 0) { //check again after wait
				break;
			}
			cout << "PCI-Socket ID " << configuration_register.busNumber
					<< configuration_register.deviceNumber << configuration_register.functionNumber << ": "
					<< "-Renewing request " << outbound_requests[0] << endl; //Sticky
			RenewRequest();
		}
	}
}

bool PcieSocket::IsInOutboundRequestBuffer(int compareTag) { //BOKMERKE noe rart her
	if (outbound_requests[compareTag] != 0) { //The TLP is not a request, and is thus not in the buffer
		MemoryRequestHeader
				* tempHeader =
						dynamic_cast<MemoryRequestHeader*> (outbound_requests[compareTag]->get_header());
		if (tempHeader->tag == compareTag) { //Check if tag in the outbound buffer location is correct
			return true;
		}
	}
	return false;
}

void PcieSocket::RemoveOutboundRequest(int index) {
	//delete outbound_requests[index]->header;	//TODO
	delete outbound_requests[index]; //Can several processes write to the same array? i think not
	outbound_requests[index] = 0; //fix it with Events instead
	/*TLP * temp;
	 for (int i = index + 1; i < 32; i++) { //Collapsing the fifo
	 temp = outbound_requests[i];//TODO SEMAPHORE
	 outbound_requests[i] = 0;
	 outbound_requests[i - 1] = temp;
	 }*/
}

bool PcieSocket::CheckECRC(TLP * incTLP) {
	//TODO 	//Implement the ECRC part, this function is not doing anything
	//	cout<<"-PCIdevice ID "<<configuration_register.busNumber<<configuration_register.deviceNumber<<configuration_register.functionNumber<<": "<<"Checking ECRC(Not Implemented)"<<endl;
	//Check
	//Strip from inc if okey

	return true;
}

bool PcieSocket::DecodeTLP(TLP* TLPToBeDecoded) {//BOKMERKE crasher her inne
	//TODO	/Fiks for alle pakketyper, hvertfall Cpl og CplD	//sjekk outbound hvis Cpl eller hvilkensomhelst completion
	TLPHeader* TLPToBeDecodedHeader = TLPToBeDecoded->get_header();
	bool ECRCPassed = false;

	if (TLPToBeDecodedHeader->TD == 1) { //If it has an ECRC part
		ECRCPassed = CheckECRC(TLPToBeDecoded); //Perform ECRC check and strip field if everything is okey.//TD specifics on page 177, simplified for now
	} else {

		ECRCPassed = true;
	}
	//cout << configuration_register.busNumber << " " << TLPToBeDecodedHeader->type << endl;
	if (TLPToBeDecodedHeader->type == CplD || TLPToBeDecodedHeader->type == Cpl) {

		if (ECRCPassed && (IsInOutboundRequestBuffer(
				(dynamic_cast<CompletionHeader*> (TLPToBeDecodedHeader))->tag))) { //TODO create an isInOutboundRequestBuffer function
			if (print_full_simulation_information) {

				cout << "PCI-Socket: Removing outbound request @"
						<< sc_time_stamp() << endl;
			}
			RemoveOutboundRequest(
					(dynamic_cast<CompletionHeader*> (TLPToBeDecodedHeader))->tag);

		}
		//PrintOutboundBuffer();

	} else if (TLPToBeDecodedHeader->type == MRd) {
		if (!configuration_register.IsAddressInBaseAddressRegisters(
				(dynamic_cast<MemoryRequestHeader*> (TLPToBeDecodedHeader))->address1)) {

			cout << "The packet at address"<<(dynamic_cast<MemoryRequestHeader*> (TLPToBeDecodedHeader))->address1 << ", is not in the subsystem memory: "
					<< configuration_register.base_address_registers_segment_startpoints[0]
					<< " -> "
					<< configuration_register.base_address_registers_segment_limit[0] << endl;
			return false; //reject packet
		}
	}
	if (!ECRCPassed) {//Something went wrong, send a new request, keep the spot
		cout << "PCI-Socket: ECRCError" << endl; //, request is still stored in the outboundrequestbuffer
		return false;
	} else {
		return true;
	}
}

void PcieSocket::HandlereceivedTLP(TLP * tlpFromBuffer) {

	bool sendToSW = DecodeTLP(tlpFromBuffer); //Send pointer to incominTLPcopy decode,return true if request is removed from outbound	//and run ECRC, strip CRC, update the , etterhvert//
	if (sendToSW) {
		//cout <<"PACKET FROM VIRTUAL receiveBUFFER BUFFER @ "<< sc_time_stamp()<< "DEV ID: "<<configuration_register.busNumber<<configuration_register.deviceNumber<<configuration_register.functionNumber<<endl<<*tlpFromBuffer<<endl;
		//Return relevant data to the requester software-layer
		to_device_core = *tlpFromBuffer; //Just a TLP now with ECRC field length equal to 0
	}
	//If not then a new req is sent
	delete tlpFromBuffer;
	tlpFromBuffer = 0;//Delete the TLP after handeling //BOKMERKE// Set pointer to 0 afterwords to check elsewhere
}

//Help empty the vcb whenever there are packets in the queue and no notify from AddToSendVCB is sent
void PcieSocket::receiveVCBufferHandelingNotifyer() {
	while (true) {
		wait(CHECK_receive_VCB_DELAY, SC_NS);
		packet_sent_to_receive_vcb_event.notify();
	}
}

void PcieSocket::receiveBufferHandeling() {
	while (true) {
		wait(packet_sent_to_receive_vcb_event);
		for (int i = 7; i >= 0; i--) { //7 has higher prio than lower numbers
			receive_buffer_mutex.lock();
			if (virtual_channel_receive_buffers[i][0] != 0) {
				HandlereceivedTLP(virtual_channel_receive_buffers[i][0]); ////BOKMERKE denne kjoeres for ofte, den får det triggete signalet tlp til software til å være blank før den leses ut.
				virtual_channel_receive_buffers[i][0] = 0;
				TLP * tempBuffer = 0;

				for (int j = 1; j < (receive_TLP_BUFFER_SIZE); j++) { //collapse fifo
					tempBuffer = virtual_channel_receive_buffers[i][j];
					virtual_channel_receive_buffers[i][j] = 0;
					virtual_channel_receive_buffers[i][j - 1] = tempBuffer;
				}
			}
			receive_buffer_mutex.unlock();
		}
		//wait(CHECK_receive_VCB_DELAY, SC_NS);	Moved to separate time based notifyer
	}
}

void PcieSocket::AddIncomingTLPToVCbuffers(TLP * receivedTLP, int TrafficClass) { //TODO bool? hvis full
	receive_buffer_mutex.lock();
	for (int i = 0; i < receive_TLP_BUFFER_SIZE; i++) {
		if (virtual_channel_receive_buffers[TrafficClass][i] == 0) {
			//cout <<"-PCIdevice ID "<<configuration_register.busNumber<<configuration_register.deviceNumber<<configuration_register.functionNumber<<": "<<"-Adding TLP to VCRB "<< i <<" @ "<<sc_time_stamp()<<endl;
			virtual_channel_receive_buffers[TrafficClass][i] = receivedTLP;
			break;
		}
	}
	//PrintreceiveVcb();
	receive_buffer_mutex.unlock();
	packet_sent_to_receive_vcb_event.notify();
}

void PcieSocket::receiveAction() { //Adds the incoming TLP to its abuffer if the packet is either a completer, or if the address is in its BAR
	while (true) {
		wait();

		TLP * receivedTLP = new TLP; //TODO i think that this fucks up the dynamic casting, because we are adding a new tlp
		*receivedTLP = receive_tlp_port;

		*(this->packets_received_counter) = *(this->packets_received_counter)
				+ 1;

		if (print_full_simulation_information) {
			cout << endl
					<< "_______________________________RECIEVING_TLP______________________________"
					<< endl;
			if (configuration_register.busNumber == 0 && configuration_register.deviceNumber == 0
					&& configuration_register.functionNumber == 0) {//API owner is root complex
				cout << "|  ROOT COMPLEX is recieving a TLP @ "
						<< sc_time_stamp() << "\t\t\t" << "         |";
			} else {
				cout << "|  PCIdevice ID " << configuration_register.busNumber
						<< configuration_register.deviceNumber << configuration_register.functionNumber
						<< " is recieving a TLP @ " << sc_time_stamp()
						<< "\t\t\t\t" << " ";
			}
			cout << endl << *receivedTLP << endl << endl << endl;
		}
		TLPHeader* tempHeader = receivedTLP->get_header();
		//cout <<"Device ID: "<<configuration_register.busNumber<<configuration_register.deviceNumber<<configuration_register.functionNumber<< "IS ADDING TO VCRB: "<< tempHeader->TC<<"@ " << sc_time_stamp()<<endl;
		AddIncomingTLPToVCbuffers(receivedTLP, tempHeader->TC);
	}
}
