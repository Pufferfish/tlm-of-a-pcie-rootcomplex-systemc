//
//  pcie_switch.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even_pcie_switch_h__
#define __Even_pcie_switch_h__

#include "systemc.h"
#include "tlp.h"
#include "pcie_socket.h"
#include "pcie_configuration_register_type_1.h"

#define PORT_ARBITRATION_BUFFER_SIZE 5		//Portarbitration before VCB arbitration
#define SWITCH_VCB_SIZE 5					//UPSTREAM packets are stored here, downstream are not, as the bottleneck is upstream
#define NUMBER_OF_DOWNSTREAM_PORTS 10
#define MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS 10

//DELAYS
#define ROUND_ROBIN_DELAY 1
#define SEND_UPSTREAM_VCB_DELAY 1
#define CHECK_SEND_UPSTREAM_VCB_DELAY 50		//TODO get all defines into one file,
extern bool print_full_simulation_information;

using namespace std;

SC_MODULE(PcieSwitch) {

	sc_in<TLP> upstream_in; //writes
	sc_out<TLP> upstream_out; //reads

	//vector<sc_in<TLP>> downstream_in_port_vector;
	sc_vector<sc_in<TLP> > downstream_in_port_vector;

	//vector<sc_out<TLP>> downstream_out_port_vector;
	sc_vector<sc_out<TLP> > downstream_out_port_vector;

	vector<int> downstream_port_map_to_bus_number;

	//2d array buffer, [32][TLP] for port arbitration, just dont use the rest of the array, 32 is maximum number of
	TLP
			* port_buffers_for_arbitration[NUMBER_OF_DOWNSTREAM_PORTS][PORT_ARBITRATION_BUFFER_SIZE];//2d array, FIFO, always 32, but not all are used


	//VCBBuffers
	TLP* virtual_channel_upstream_send_buffers[8][SWITCH_VCB_SIZE];
	//TLP* virtual_channel_receive_buffers[8][SWITCH_VCB_SIZE];

	//ConfRegister with devicenr etc
	ConfigurationRegisterType1 conf_reg;

	int bus_numbers_connected_local_copy[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS];
	//BUS-registers, with depth first it knows how to root packets if the current device isnt the receiver.

	//Keep track of packets sent and received, decleared on heap memory to be available in destructor.
	int *packets_sent_upstream_counter = new int(0);
	int *packets_received_from_upstream_counter = new int(0);
	int *packets_sent_downstream_counter = new int(0);
	int *packets_received_from_downstream_counter = new int(0);

	sc_mutex packets_sent_upstream_counter_mutex;
	sc_mutex packets_received_from_upstream_counter_mutex;
	sc_mutex packets_sent_downstream_counter_mutex;
	sc_mutex packets_received_from_downstream_counter_mutex;

	//	sc_mutex send_buffer_mutex;
	sc_mutex upstream_virtual_channel_buffer_mutex;
	sc_mutex receive_downstream_port_buffer_mutex;

	sc_event packet_sent_to_send_vcb_event;

	//	sc_event start_buffer_handler;

	//Functions		//TODO	RYDD

	int SendToThisPort(int);
	void PrintSendVCB();
	void SendVCBufferHandelingNotifyer();
	bool VirtualChannelBufferIsEmpty();
	void VirtualChannelBufferHandeling();
	void InsertInUpstreamVirtualChannelBuffer(TLP*);
	void PortBufferHandler();
	void receiveFromDownStreamAction();
	void receiveFromUpStreamAction();
	void SendFirstTLPAndUpdateVCBuffers(int);
	int ConvertPortToBusNr(int);

	//	void connectedElementsWatcher(); //TODO Implement ping to check if connected, and implement broadcast on endpoints to allow hot-plugin


	//Constructor
	SC_HAS_PROCESS( PcieSwitch);
	PcieSwitch(
			sc_module_name name_,
			int bnr_ = 0,
			int dnr_ = 0,
			int fnr_ = 0,
			int number_of_downstream_ports_connected_ = 0,
			int bus_numbers_connected_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 },
			int system_memory_address_from_ = -1,
			int system_memory_address_to_ = -1,
			int port_base_address_registers_base_array_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 },
			int port_base_address_registers_limit_array_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 }) :
		sc_module(name_) {



		if (!(bnr_ || dnr_ || fnr_)) { //Construct differently if switch is submodule of root complex
			for (int i = 0; i < number_of_downstream_ports_connected_; i++) {
				downstream_port_map_to_bus_number.push_back(
						bus_numbers_connected_[i]);
			}
		} else {

			for (int i = 1; i < number_of_downstream_ports_connected_ + 1; i++) {
				downstream_port_map_to_bus_number.push_back(
						bus_numbers_connected_[i]);
			}
		}
		//Dynamic number of io downstream ports
		downstream_in_port_vector.init(number_of_downstream_ports_connected_); //Instanciate a dynamic number of ports
		downstream_out_port_vector.init(number_of_downstream_ports_connected_);
		//Thread
		SC_METHOD(receiveFromUpStreamAction);
		sensitive << upstream_in;
		dont_initialize();
		//Switch Processes

		SC_METHOD(receiveFromDownStreamAction); //One listner for each sc_in.
		for (int i = 0; i < number_of_downstream_ports_connected_; i++) {
			sensitive << downstream_in_port_vector[i];
		}
		dont_initialize();

		SC_THREAD(PortBufferHandler);

		SC_THREAD(SendVCBufferHandelingNotifyer);
		for (int i = 0; i < number_of_downstream_ports_connected_; i++) {
			sensitive << downstream_in_port_vector[i];
		}
		dont_initialize();

		SC_THREAD(VirtualChannelBufferHandeling);







		//Construct the confREG for the switch
		conf_reg.base_address_registers_segment_startpoints[0]
				= system_memory_address_to_;
		conf_reg.base_address_registers_segment_limit[0]
				= system_memory_address_from_;
		for (int i = 0; i < number_of_downstream_ports_connected_; i++) {

			conf_reg.port_base_address_registers_base_array[i]
					= port_base_address_registers_base_array_[i];
			conf_reg.port_base_address_registers_limit_array[i]
					= port_base_address_registers_limit_array_[i];
		}
		conf_reg.busNumber = bnr_; //bus number 0 is RC
		conf_reg.deviceNumber = dnr_; //All endpoints have devnr 0
		conf_reg.functionNumber = fnr_; //0 is default

		//Init VCBuffers to zero//
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < SWITCH_VCB_SIZE; j++) {
				virtual_channel_upstream_send_buffers[i][j] = 0;
			}
		}
		//Init Port buffers to zero
		for (int i = 0; i < NUMBER_OF_DOWNSTREAM_PORTS; i++) {
			for (int j = 0; j < PORT_ARBITRATION_BUFFER_SIZE; j++) {
				port_buffers_for_arbitration[i][j] = 0;
			}
		}
	}


	//Destructor
	~PcieSwitch() {
		if (print_full_simulation_information) {

			if (conf_reg.busNumber == 0 && conf_reg.deviceNumber == 0
					&& conf_reg.functionNumber == 0) {
				cout << "\t";
			}

			cout << "--Switch: " << conf_reg.busNumber << conf_reg.deviceNumber
					<< conf_reg.functionNumber << " Destructor" << "--" << endl;
			if (conf_reg.busNumber == 0 && conf_reg.deviceNumber == 0
					&& conf_reg.functionNumber == 0) {
				cout << "\t";
			}

			cout << "\t Packet Statistics: " << "<"
					<< *packets_sent_upstream_counter << "/"
					<< *packets_received_from_downstream_counter << "> <"
					<< *packets_sent_downstream_counter << "/"
					<< *packets_received_from_upstream_counter << ">" << endl;
		}

		//Empty VCB
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < SWITCH_VCB_SIZE; j++) {
				delete virtual_channel_upstream_send_buffers[i][j];
			}
		}
		//PrintSendVCB();


		for (int i = 0; i < NUMBER_OF_DOWNSTREAM_PORTS; i++) {
			for (int j = 0; j < PORT_ARBITRATION_BUFFER_SIZE; j++) {
				delete port_buffers_for_arbitration[i][j];
			}
		}
		delete packets_sent_upstream_counter;
		delete packets_received_from_upstream_counter;
		delete packets_sent_downstream_counter;
		delete packets_received_from_downstream_counter;
	}

};
#endif

