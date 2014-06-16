//
//  sc_main_help_functions.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "sc_main_help_functions.h"

//Configuration global variables
//Set print options for the output
bool print_full_simulation_information = false;
bool print_data_sent_only_simulation_information = false;
bool print_statistics_only_simulation_information = false;
bool print_statistics_summary_information = false;
bool print_full_and_system_memory = false;

bool rc_is_reader__init_mem_var=true;
bool ep_is_reader__init_mem_var=true;

bool plot_mrd_cpl_delay_dist=false;

sc_mutex simulation_tlp_number_counter_mutex;
int simulation_tlp_number_counter=0;


int max_payload_size = 256;
int read_completion_boundary=64;

//Set when executing Initialize, these are set with inputs next to the execution command ./Pciesystem.out ep0 ep1 ep2, in this manner greping is easy.
int number_of_packets_from_pcie_ep_0 = 0;
int number_of_packets_from_pcie_ep_1 = 0;
int number_of_packets_from_pcie_ep_2 = 0;
int number_of_packets_from_pcie_rc = 0;

PacketType request_type_endpoint_0 = MRd;
PacketType request_type_endpoint_1 = MRd;
PacketType request_type_endpoint_2 = MRd;
PacketType request_type_from_pcie_rc = MRd;

int endpoint_request_read_length = 1;
int endpoint_request_write_length = 1;

//Set pre runtime
int packet_period_from_pcie_ep_0 = 10; //TODO fails when 1 2 3 ns //quues fills up somewhere...
int packet_period_from_pcie_ep_1 = 50;
int packet_period_from_pcie_ep_2 = 10000;
int packet_period_from_pcie_rc = 400;

int traffic_class_pcie_ep_0 = 5;		//002
int traffic_class_pcie_ep_1 = 6;		//003
int traffic_class_pcie_ep_2 = 4;		//004
int traffic_class_pcie_rc = 1;

int simulation_time = 10000;




bool only_ep_sends=false;
//bool only_ep_sends=false;
bool rc_multitasking_is_enabled=true;


void InitializeProgramFromInputArguments(int argc, char* argv[]) {
	//Initialize the program with the parametres that came with the execution command
	if (argc >= 2) {
		number_of_packets_from_pcie_ep_0 = atoi(argv[1]);
	}
	if (argc >= 3) {
		number_of_packets_from_pcie_ep_1 = atoi(argv[2]);
	}
	if (argc >= 4) {
		number_of_packets_from_pcie_ep_2 = atoi(argv[3]);
	}
	if (argc >= 5) {
		if (std::string(argv[4]) == "MWr") {
			request_type_endpoint_0 = MWr;
			request_type_endpoint_1 = MWr;
			request_type_endpoint_2 = MWr;

			ep_is_reader__init_mem_var=false;
		} else if (std::string(argv[4]) == "MRd") {
			request_type_endpoint_0 = MRd;
			request_type_endpoint_1 = MRd;
			request_type_endpoint_2 = MRd;
			ep_is_reader__init_mem_var=true;
		}
	}
	if (argc >= 6) {
		number_of_packets_from_pcie_rc = atoi(argv[5]);
		if(atoi(argv[5])>0){
			only_ep_sends=false;
		}
	}
	if (argc >= 7) {
		if (std::string(argv[6]) == "MRd") {
			request_type_from_pcie_rc = MRd;
			rc_is_reader__init_mem_var=true;
		} else if (std::string(argv[6]) == "MWr") {
			request_type_from_pcie_rc = MWr;
			rc_is_reader__init_mem_var=false;
		}
	}
	if (argc >= 8) {
		simulation_time = atoi(argv[7]);
	}
	if (argc >= 9) {
		if (std::string(argv[8]) == "-show_full") {
			print_full_simulation_information = true;
			//cout << "YES" << endl;
		} else if (std::string(argv[8]) == "-show_traffic") {
			print_data_sent_only_simulation_information = true;
		} else if (std::string(argv[8]) == "-show_statistics") {
			print_statistics_only_simulation_information = true;
		} else if (std::string(argv[8]) == "-show_statistics_summary") {
			print_statistics_summary_information = true;
			print_statistics_only_simulation_information = true;
		} else if (std::string(argv[8]) == "-show_full_and_memory") {
			print_full_and_system_memory = true;
			print_full_simulation_information = true;
		}
	}
	if(argc >=10){
		endpoint_request_write_length=atoi(argv[9]);
		endpoint_request_read_length=atoi(argv[9]);
	}

	if(argc >=11){
		if(std::string(argv[10])== "-out.txt"){
			freopen("./generated_logs/terminal_log.txt","w",stdout);
		}
	}
	if(argc >=12){
		if(std::string(argv[11])== "-plot"){
			plot_mrd_cpl_delay_dist=true;
		}
	}
}

void RunSystemCSimulation(int simulation_time, int argc, char* argv[]) {

	cout << endl << endl << endl << endl << endl << endl << endl << endl
			<< endl;

	cout
			<< "_________________________ INITIATING SIMULATION _________________________"
			<< endl << endl;
	//Run simulation for
	sc_start(simulation_time, SC_NS);
	if (!print_full_simulation_information
			&& !print_data_sent_only_simulation_information
			&& !print_statistics_only_simulation_information && !print_statistics_summary_information && !print_full_and_system_memory ) {


		cout << endl << "\t\tNo flags were set; try using: "
				<< "-show_full, -show_full_and_memory, -show_traffic, -show_statistics or -show_statistics_summary"
				<< endl << endl;

		cout << endl << "\t\tExecute format is:\t"
				<< "$./PciSystem [(Pcie request numbers:) EP1(int) EP2(int) EP3(int)] [flag]"
				<< endl << endl;

		cout << endl << "\t\tAn example is:\t\t"
				<< "$./PciSystem 1 1 0 -show_traffic" << endl << endl;

	}

	cout << endl
			<< "_________________________ SIMULATION COMPLETE __________________________"
			<< endl;
	cout << endl
			<< "_________________________         @"
			<< sc_time_stamp()
			<< "       _________________________ "
			<< endl;
	cout << endl
			<<"________________________________________________________________________"
			<< endl;
	//	cout << ;

	cout << endl << endl << endl;
//	if (print_full_simulation_information || print_statistics_summary_information) {
		cout << "Program was executed with the following parameters:" << endl
				<< "\t";
		for (int i = 0; i < argc; i++) {

			cout << argv[i] << " ";

		}
//	}
	cout << endl << endl << endl;

	//sc_stop();

}
