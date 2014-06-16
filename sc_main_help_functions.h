//
//  sc_main_help_functions.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__sc_main_help_functions_h__
#define __Even__sc_main_help_functions_h__

#include <iostream>
#include <ctime> // Needed for the true randomization
#include "systemc.h"
#include "tlp_header.h"

//Configuration global variables
//Set print options for the output
extern bool print_full_simulation_information;
extern bool print_data_sent_only_simulation_information;
extern bool print_statistics_only_simulation_information;
extern bool print_statistics_summary_information;
extern bool print_full_and_system_memory;

//Set when executing Initialize, these are set with inputs next to the execution command ./Pciesystem.out ep0 ep1 ep2, in this manner greping is easy.
extern int number_of_packets_from_pcie_ep_0;
extern int number_of_packets_from_pcie_ep_1;
extern int number_of_packets_from_pcie_ep_2;
extern int number_of_packets_from_pcie_rc;

extern sc_mutex simulation_tlp_number_counter_mutex;
extern int simulation_tlp_number_counter;

extern PacketType request_type_endpoint_0;
extern PacketType request_type_endpoint_1;
extern PacketType request_type_endpoint_2;
extern PacketType request_type_from_pcie_rc;

//Set pre runtime
extern int packet_period_from_pcie_ep_0; //TODO fails when 1 2 3 ns //quues fills up somewhere...
extern int packet_period_from_pcie_ep_1;
extern int packet_period_from_pcie_ep_2;
extern int packet_period_from_pcie_rc;

extern int traffic_class_pcie_ep_0;
extern int traffic_class_pcie_ep_1;
extern int traffic_class_pcie_ep_2;
extern int traffic_class_pcie_rc;

extern bool rc_multitasking_is_enabled;
extern int simulation_time;

void InitializeProgramFromInputArguments(int argc, char* argv[]);

void RunSystemCSimulation(int simulation_time, int argc, char* argv[]);

#endif

