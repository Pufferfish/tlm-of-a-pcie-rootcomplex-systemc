//
//  pcie_socket.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__pcie_socket__
#define __Even__pcie_socket__

#include "systemc.h"
#include "pcie_socket_software_layer_data.h"
#include "tlp.h"
#include "pcie_configuration_register_type_0.h"

#define SEND_TLP_BUFFER_SIZE 30
#define receive_TLP_BUFFER_SIZE 30

extern bool print_full_simulation_information;
extern int max_payload_size;

using namespace std;

SC_MODULE(PcieSocket) {
	//IO//
	//Talk_with_PCI_Network
	sc_in<TLP> receive_tlp_port;
	sc_out<TLP> send_tlp_port;
	//Talk_with_SW_Layer
	sc_in<SoftwareData> from_device_core;
	sc_out<TLP> to_device_core; //TODO TLP::strippedFromECRC?

	//Internal-variables//
	ConfigurationRegisterType0 configuration_register;
	TLP* outbound_requests[32];

	//Helpvars
	int *packets_sent_counter = new int(0);
	int *packets_received_counter = new int(0);
	TLP* virtual_channel_send_buffers[8][SEND_TLP_BUFFER_SIZE];
	TLP* virtual_channel_receive_buffers[8][receive_TLP_BUFFER_SIZE];
	sc_mutex send_buffer_mutex;
	sc_mutex receive_buffer_mutex;
	sc_event add_tlp_to_outbound_request_buffer_event;
	sc_event packet_sent_to_send_vcb_event;
	sc_event packet_sent_to_receive_vcb_event;

	/////FUNCTIONS/////
	//GENERAL////TODO createECRC
	bool ContainsData(SoftwareData);
	bool IsRequestPacket(int);
	void PrintSendVCB();
	void PrintreceiveVcb();
	void PrintOutboundBuffer();
	void SendTLP(TLP *);
	void SendFirstTLPAndUpdateVCBuffers(int);
	void SendVCBufferHandeling();
	void SendVCBufferHandelingNotifyer();
	bool AddToSendVCB(TLP *);
	int InsertOutboundRequests(TLP *);
	TLP* BuildTLP(SoftwareData);
	void SendAction();
	void RenewRequest();
	void OutboundRequestHandeling();
	bool IsInOutboundRequestBuffer(int);
	void RemoveOutboundRequest(int);
	bool CheckECRC(TLP*);
	bool DecodeTLP(TLP*);
	void receiveVCBufferHandelingNotifyer();
	void HandlereceivedTLP(TLP*);
	void receiveBufferHandeling();
	void AddIncomingTLPToVCbuffers(TLP*, int);
	void receiveAction();

	//Constructor//
	SC_HAS_PROCESS(PcieSocket);
	PcieSocket(sc_module_name name_, int bus_number_ = 0,
			int device_number_ = 0, int function_number_ = 0,
			bool use_ecrc_ = 0, int system_memory_fraction_base_ = -1,
			int system_memory_fraction_limit_ = -1) :
		sc_module(name_) {

		SC_THREAD(OutboundRequestHandeling);

		SC_THREAD(receiveVCBufferHandelingNotifyer);
		sensitive << receive_tlp_port;
		dont_initialize();

		SC_THREAD(receiveBufferHandeling);

		SC_THREAD(receiveAction);
		sensitive << receive_tlp_port;

		SC_THREAD(SendVCBufferHandelingNotifyer);
		sensitive << from_device_core;//packet_sent_to_send_vcb_event;		//sensitive to its own notification signal, start whenever SendAction sets it high.
		dont_initialize();

		SC_THREAD(SendVCBufferHandeling);

		SC_THREAD(SendAction);
		sensitive << from_device_core;

		//Construct variables

		//Initialize Virtual channel buffers//
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < SEND_TLP_BUFFER_SIZE; j++) {
				virtual_channel_send_buffers[i][j] = 0;
			}
			for (int j = 0; j < receive_TLP_BUFFER_SIZE; j++) {
				virtual_channel_receive_buffers[i][j] = 0;
			}
		}

		//Clear the outboundRequestbuffers
		for (int i = 0; i < 32; i++) {
			outbound_requests[i] = 0; //initialize with 0 pointers in the replay-buffer for the endpoint
		}

		//Construct the confREG
		configuration_register.base_address_registers_segment_startpoints[0]
				= system_memory_fraction_base_;
		configuration_register.base_address_registers_segment_limit[0]
				= system_memory_fraction_limit_;
		configuration_register.busNumber = bus_number_; //bus number 0 is RC
		configuration_register.deviceNumber = device_number_; //All endpoints have devnr 0
		configuration_register.functionNumber = function_number_; //0 is default
		configuration_register.useECRCWhenSending = use_ecrc_; //

	}

	~PcieSocket() {
		if (print_full_simulation_information) {
			if (*packets_sent_counter != 0 || *packets_received_counter != 0) {

				cout << "\t--Socket Destructor--" << endl;
				if (configuration_register.busNumber == 0
						&& configuration_register.deviceNumber == 0
						&& configuration_register.functionNumber == 0) {
					cout << "\t";
				}
				cout << "\t Packet Statistics: <" << *packets_sent_counter
						<< "/" << *packets_received_counter << ">"
						<< endl << endl;
			}
		}
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < SEND_TLP_BUFFER_SIZE; j++) {
				if (virtual_channel_send_buffers[i][j] != 0) {
					delete virtual_channel_send_buffers[i][j];

				}
				if (virtual_channel_receive_buffers[i][j] != 0) {
					delete virtual_channel_receive_buffers[i][j];
				}
			}
		}
		for (int i = 0; i < 32; i++) {
			if (outbound_requests[i] != 0) {
				delete outbound_requests[i];

			}
		}
		delete packets_sent_counter;
		delete packets_received_counter;
	}

};

#endif
