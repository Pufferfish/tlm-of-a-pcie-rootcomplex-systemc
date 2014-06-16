//
//  complete_system.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__complete_system__
#define __Even__complete_system__	

//This file implements the RC, the EP and the memory modules.

#include "systemc.h"
#include "tlp.h"
#include "pcie_socket.h"
#include "root_complex.h"
#include "pcie_endpoint.h"
#include "memory.h"
#include "pcie_switch.h"

extern int number_of_packets_from_pcie_ep_0;
extern int number_of_packets_from_pcie_ep_1;
extern int number_of_packets_from_pcie_ep_2;

extern PacketType request_type_endpoint_0;
extern PacketType request_type_endpoint_1;
extern PacketType request_type_endpoint_2;

extern int packet_period_from_pcie_ep_0;
extern int packet_period_from_pcie_ep_1;
extern int packet_period_from_pcie_ep_2;

extern int traffic_class_pcie_ep_0;
extern int traffic_class_pcie_ep_1;
extern int traffic_class_pcie_ep_2;

extern int packet_period_from_pcie_rc;
extern int number_of_packets_from_pcie_rc;
extern int traffic_class_pcie_rc;
extern PacketType request_type_from_pcie_rc;

using namespace std;

/**
 *			TODO
 * 				-Pointers or global variables to symbolize credits.
 * 				-Clean up code, get all defines in one place.
 * 				-Remove delays on critical path.
 *				-
 *
 *
 * 							________________________________  	   	 ______
 *							|	RC							|=======|MEMORY|
 *							| 								|		|______|
 *							|								|
 *							|								|
 *							|_______________________________|
 *							|	    						|
 *							|								|
 *							|								|
 *							|	Arbiter						|
 *							|								|
 *							|				Bnr 0			|
 *				  			|		_______v___				|
 *				  			|		|  SWITCH	|			|	TODO BOKMERKE, implement switch module inside RC.
 *				  			|		| p1 	pn	|			|		BOKMERKE, For nå ta kun å ha en switch under RC, dropp pcie-ep n og switchen inni
 *							|		| delay asb4|			|
 *				  			|		|_v_*_*_v___|			|
 *							|  _____/		|_________		|
 *							| /			ID:000		  \		|
 *							|/_________________________\____|
 *							/ 							|
 *						   /Bnr 1						|
 *					      /								|Bnr 4
 *				  _______v______						|
 *				  |  SWITCH		|						|
 *				  | p1 	  p4	|					    |
 *				  |	ID:100		|						|
 *				  |	Delay:		|						|
 *				  |	 1ns for	|						|
 *				  |	 every vcb 	|						|
 *				  |  and 1ns for| 						|
 *				  |  every RR 	|						|
 *				  |	 wait.		|						|
 *				  |_v_v_v_v_____|						|
 *				    |  	  |								|
 *			Bnr 2   |     |	Bnr 3						|
 *	 _______________|_	 _|_______________  		 ___|____________
 *	|     PCIE-EP	 |	|     PCIE-EP	 | 		   	|   PCIE-EP		|		Up to 256 EPs
 *	|	  Nr:0		 |	|	  Nr:1		 | *   *   	|	Nr:2		|
 *	| 	ID: 200 	 |	| 	ID: 300		 |			|	ID:400		|
 *	|	Delay:0ns	 |  |				 | 			|   			|
 *	|________________|	|________________| 			|_______________|
 *
 *
 *
 *  • Primary Bus Number Register = 11.
 *	• Secondary Bus Number Register = 12.
 *	• Subordinate Bus Number Register = 12.
 *
 *
 *
 *
 */

SC_MODULE(CompleteSystem) {
	// Sub-module pointer declarations
	RootComplex *root_complex_module_pointer;
	PcieEndPoint *end_point_0_sub_module_pointer;
	PcieEndPoint *end_point_1_sub_module_pointer;
	PcieEndPoint *end_point_2_sub_module_pointer;
	Memory *memory_module_pointer;
	PcieSwitch *pcie_switch_module_pointer;

	//Signal vectors for up and downstream bus, correlates to real bus numbers, exept 0 equals bnr 1 and 1 equals bnr 2 etc
	sc_vector<sc_signal<TLP> > buses_upstream_signal_vector;
	sc_vector<sc_signal<TLP> > buses_downstream_signal_vector;

	int total_bus_number = 4; //exept from 0
	//map <int,int> busmap_level1;

	//Memory interconnect signals
	sc_signal<int> data_to_memory; //Inout cannot have several drivers?
	sc_signal<int> data_from_memory;
	sc_signal<int> address_memory;
	sc_signal<bool> read_write_bit_memory;
	sc_signal<bool> enable_memory;
	sc_signal<bool> write_confirmation;

	// Constructor
	SC_CTOR(CompleteSystem) { //TODO No threads?
		//Initialize the vector-length of the bus signal containers
		buses_upstream_signal_vector.init(total_bus_number);
		buses_downstream_signal_vector.init(total_bus_number);

		//MEMORY map
		int rc_memory_base = 0;
		int rc_memory_limit = 1000;

		int ep_0_memory_base = 1001; //ep
		int ep_0_memory_limit = 1100;

		int ep_1_memory_base = 1101;
		int ep_1_memory_limit = 1200;

		int ep_2_memory_base = 1201;
		int ep_2_memory_limit = 1300;

		//Array of bus numbers connected to root complex
		int	bus_numbers_connected_to_root_complex[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS];
		int number_of_root_complex_connections = 2;
		bus_numbers_connected_to_root_complex[0] = 1; //switch std::vector<int> port_base_address_registers_base_vector_,std::vector<int> port_base_address_registers_limit_vector_
		bus_numbers_connected_to_root_complex[1] = 4; //ep 2 directly connected

		//Array of bus numbers connected to the switch
		int	bus_numbers_connected_to_switch[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS];
		int number_of_switch_connections = 3;
		bus_numbers_connected_to_switch[0] = 1; //rc upstream
		bus_numbers_connected_to_switch[1] = 2; //ep 0
		bus_numbers_connected_to_switch[2] = 3; //ep 1

		int switch_port_base_address_registers_base_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS]; //Contains ep 0 and ep 1's memory map.
		int	switch_port_base_address_registers_limit_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS];

		int	rc_port_base_address_registers_base_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS]; //Contains ep 0 and ep 1's memory map.
		int	rc_port_base_address_registers_limit_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS];

		switch_port_base_address_registers_base_array[0] = ep_0_memory_base;
		switch_port_base_address_registers_base_array[1] = ep_1_memory_base;
		switch_port_base_address_registers_limit_array[0] = ep_0_memory_limit;
		switch_port_base_address_registers_limit_array[1] = ep_1_memory_limit;

		rc_port_base_address_registers_base_array[0] = 1001;
		rc_port_base_address_registers_base_array[1] = 1201;
		rc_port_base_address_registers_limit_array[0] = 1200;
		rc_port_base_address_registers_limit_array[1] = 1300;

		//Module allocation and instantiation.
		//RootComplex
		root_complex_module_pointer = new RootComplex("ROOTCOMPLEX",
				number_of_root_complex_connections,
				bus_numbers_connected_to_root_complex,
				packet_period_from_pcie_rc, number_of_packets_from_pcie_rc,
				traffic_class_pcie_rc, request_type_from_pcie_rc,
				rc_memory_base, rc_memory_limit,
				rc_port_base_address_registers_base_array,
				rc_port_base_address_registers_limit_array); //2 devices connected directly to RC
		//EndPoints
		end_point_0_sub_module_pointer = new PcieEndPoint("ENDPOINT0",
				bus_numbers_connected_to_switch[1], 0, 0, false,
				packet_period_from_pcie_ep_0, number_of_packets_from_pcie_ep_0,
				traffic_class_pcie_ep_0, request_type_endpoint_0,
				ep_0_memory_base, ep_0_memory_limit); //BUS , DEV, FUNC, ECRC, DELAYBETWEENPACKETS, NR OF PACKETS
		end_point_1_sub_module_pointer = new PcieEndPoint("ENDPOINT1",
				bus_numbers_connected_to_switch[2], 0, 0, false,
				packet_period_from_pcie_ep_1, number_of_packets_from_pcie_ep_1,
				traffic_class_pcie_ep_1, request_type_endpoint_1,
				ep_1_memory_base, ep_1_memory_limit); //TODO check for 0 and high numbers
		end_point_2_sub_module_pointer = new PcieEndPoint("ENDPOINT2",
				bus_numbers_connected_to_root_complex[1], 0, 0, false,
				packet_period_from_pcie_ep_2, number_of_packets_from_pcie_ep_2,
				traffic_class_pcie_ep_2, request_type_endpoint_2,
				ep_2_memory_base, ep_2_memory_limit);
		//Memory
		memory_module_pointer = new Memory("MEMORY", 0, 1000);
		//Switch
		pcie_switch_module_pointer = new PcieSwitch("PCISWITCHEN",
				bus_numbers_connected_to_root_complex[0], 0, 0,
				number_of_switch_connections - 1,
				bus_numbers_connected_to_switch, -1, -1,
				switch_port_base_address_registers_base_array,
				switch_port_base_address_registers_limit_array); //TODO BOKMERKE fiks lignende på de andre modulene

		//RC- SignalBinding
		for (int i = 0; i < number_of_root_complex_connections; i++) {
			root_complex_module_pointer->downstream_in_port_vector[i](
					buses_upstream_signal_vector[bus_numbers_connected_to_root_complex[i]
							- 1]); //-1 to get 0 index in the sc_vector of signals
			root_complex_module_pointer->downstream_out_port_vector[i](
					buses_downstream_signal_vector[bus_numbers_connected_to_root_complex[i]
							- 1]);
		}

		//SWITCH- SignalBinding
		for (int i = 0; i < number_of_switch_connections; i++) {
			if (i == 0) {
				pcie_switch_module_pointer->upstream_in(
						buses_downstream_signal_vector[bus_numbers_connected_to_root_complex[0]
								- 1]);
				pcie_switch_module_pointer->upstream_out(
						buses_upstream_signal_vector[bus_numbers_connected_to_root_complex[0]
								- 1]);
			} else {
				pcie_switch_module_pointer->downstream_in_port_vector[i - 1](
						buses_upstream_signal_vector[bus_numbers_connected_to_switch[i]
								- 1]);
				pcie_switch_module_pointer->downstream_out_port_vector[i - 1](
						buses_downstream_signal_vector[bus_numbers_connected_to_switch[i]
								- 1]);
			}
		}

		//EP- SignalBinding
		end_point_0_sub_module_pointer->apireceiveTLPChannel(
				buses_downstream_signal_vector[bus_numbers_connected_to_switch[1]
						- 1]);
		end_point_0_sub_module_pointer->apiSendTLPChannel(
				buses_upstream_signal_vector[bus_numbers_connected_to_switch[1]
						- 1]);

		end_point_1_sub_module_pointer->apireceiveTLPChannel(
				buses_downstream_signal_vector[bus_numbers_connected_to_switch[2]
						- 1]);
		end_point_1_sub_module_pointer->apiSendTLPChannel(
				buses_upstream_signal_vector[bus_numbers_connected_to_switch[2]
						- 1]);

		end_point_2_sub_module_pointer->apireceiveTLPChannel(
				buses_downstream_signal_vector[bus_numbers_connected_to_root_complex[1]
						- 1]);
		end_point_2_sub_module_pointer->apiSendTLPChannel(
				buses_upstream_signal_vector[bus_numbers_connected_to_root_complex[1]
						- 1]);

		//MEMORY- SignalBinding
		root_complex_module_pointer->data_to_memory(data_to_memory);
		root_complex_module_pointer->data_from_memory(data_from_memory);
		root_complex_module_pointer->address_memory(address_memory);
		root_complex_module_pointer->write_memory(read_write_bit_memory);
		root_complex_module_pointer->enable_memory(enable_memory);
		root_complex_module_pointer->write_confirmation_memory(
				write_confirmation);

		memory_module_pointer->dataToMemory(data_to_memory);
		memory_module_pointer->dataFromMemory(data_from_memory);
		memory_module_pointer->address_memory(address_memory);
		memory_module_pointer->write_memory(read_write_bit_memory);
		memory_module_pointer->enable(enable_memory);
		memory_module_pointer->write_confirmation(write_confirmation);
	}

	~CompleteSystem() { //added for memory-leakage reasons
		delete root_complex_module_pointer;
		cout << endl;
		delete end_point_0_sub_module_pointer;
		delete end_point_1_sub_module_pointer;
		delete end_point_2_sub_module_pointer;
		cout << endl;
		delete memory_module_pointer;
		delete pcie_switch_module_pointer;
		if (print_full_simulation_information) {
			cout << endl << "- - - The PCIe System has been destroyed - - -"
					<< endl;
		}
		cout << endl;
	}
};

#endif
