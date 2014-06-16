//
//  root_complex.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even_root_complex_h__
#define __Even_root_complex_h__

//Includes for the delay-moddeling
#include <iostream>
#include <ctime> // Needed for the true randomization
#include <cstdlib>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <algorithm>
#include <queue>
#include "systemc.h"
#include "pcie_socket.h"
#include "tlp.h"
#include "pcie_socket_software_layer_data.h"
#include "pcie_switch.h"

#define MEMORY_ACCESS_DELAY 0
#define MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS 10

extern bool print_full_simulation_information;
extern bool print_data_sent_only_simulation_information;
extern bool print_statistics_only_simulation_information;
extern bool print_statistics_summary_information;
extern int endpoint_request_read_length;
extern int endpoint_request_write_length;
extern int read_completion_boundary;
extern sc_mutex simulation_tlp_number_counter_mutex;
extern int simulation_tlp_number_counter;
extern bool rc_multitasking_is_enabled;

using namespace std;

/**
 *
 * 							________________________________  	   	 ______
 *							|	RC							|=======|MEMORY|
 *							| 								|		|______|
 *							|								|
 *							|								|
 *							|_______________________________|
 *							|	    						|
 *							|								|
 *							|			SW-Layer			|
 *							|								|
 *							|_______________________________|
 *							|			_________			|
 *							|			|PCIe 	 |			|
 *							|			|Protocol|			|
 *							|			|Socket	 |			|
 *							|			|________|			|
 *							|	 	 _______|_______		|
 *							|		|	SWICH		|		|
 *							|		|				|		|
 *							|		|				|		|
 *							|		|_______________|		|
 *							|_________|_____|_______|_______|
 *							/ 	|					 \		sc_vector< sc_in<TLP> > downstream_in_port_vector;
 *						   /	|					   \	sc_vector< sc_out<TLP> > downstream_out_port_vector;
 *					      /		|						\
 *				         /		|						 \
 *				        / 		|						  \
 *				       /		|		       			   \
 *				      / 		|							\
 *				     /  		|							 \
 *				    /   		|							  \
 *	 ______________/_	 _______|________    				   \________________
 *	|     PCIE-EP	 |	|     PCIE-EP	 |  				   	|   PCIE-EP		|		Up to 256 EPs
 *	|	  Nr:0		 |	|	  Nr:1		 | *   *   *   *   *   	|	Nr:n		|
 *	|________________|	|________________|   					|_______________|
 *
 *
 *
 *
 *
 */

SC_MODULE(RootComplex) {
	// Sub-modules
	PcieSocket* socket;
	PcieSwitch* pcie_switch_pointer;

	//Outputs and inputs from RC connects downstream ports
	sc_vector<sc_in<TLP> > downstream_in_port_vector;
	sc_vector<sc_out<TLP> > downstream_out_port_vector;

	//Socket Signals, connects to switch upstream port
	sc_signal<TLP> apireceiveTLPChannel; //in
	sc_signal<TLP> apiSendTLPChannel; //Out

	//Output and input of socket upstream
	sc_signal<SoftwareData> apiFromDevCore; //internal
	sc_signal<TLP> apiToDevCore; //internal

	sc_signal<TLP> switch_socket_wire_downstream;
	sc_signal<TLP> switch_socket_wire_upstream;

	//memory signals
	sc_in<int> data_from_memory;
	sc_out<int> data_to_memory;
	sc_out<int> address_memory;
	sc_out<bool> write_memory;
	sc_out<bool> enable_memory;
	sc_in<bool> write_confirmation_memory;

	sc_event data_read_from_memory_available_event; //TODO Occurs when data changes?!?! on the memorybus. Hva hvis dataen er lik
	sc_event data_was_successfully_written_event;
	sc_event handle_tlp_in_incoming_tlp_queue;
	sc_event tlp_is_handeled;
	sc_event new_tlp_in_incoming_tlp_queue;
	sc_event send_data_to_software_layer_queue_event;
	sc_event elements_in_multitasking_queue;

	sc_mutex send_data_to_software_layer_queue_mutex;
	sc_mutex send_cpl_data_to_software_layer_multitasking_vector_mutex;

	std::vector<int> delay_samples_vector;
	std::vector<sc_time> requests_received_timestamps_vector;
	std::vector<sc_time> completions_sent_timestamps_vector;
	std::vector<sc_time> requests_sent_timestamps_vector;
	std::vector<sc_time> completions_received_timestamps_vector;
	std::vector<int> wait_sample_vector_for_printouts;
	std::vector<sc_time> send_time_vector;
	std::vector<int> received_data_vector;
	std::vector<SoftwareData> send_cpl_data_to_software_layer_multitasking_vector;

	std::queue<SoftwareData> send_data_to_software_layer_queue;
	//Queue to prevent congestion, simply add every incoming TLP to this queue and handle them.
	queue<TLP*> incoming_tlp_queue;


	//HelpVariables
	int* requests_received_counter = new int(0);
	int* completions_sent_counter = new int(0);
	int* requests_sent_counter = new int(0);
	int* completions_received_counter = new int(0);
	PacketType rootcomplex_spammer_type;
	int total_amount_of_requests;
	int send_request_period;
	int requests_traffic_class;
	//ROOTCOMPLEX specific
	int busNumber = 0;
	int deviceNumber = 0;
	int functionNumber = 0;
	bool useECRC = false;

	//Functions
	void RootComplexInitiateSendAction(); //TODO If RC wants to initiate anything on behalf of the cpu
	void SendQueueHandeling(); //Handles the sendqueue, sendrequests from completer and from sender are in order sent down to transaction layer workaround for multiple process writers to one port
	void InitializePacketDataToSocket(SoftwareData&, PacketType);// Function to configure packet senddata, like a UI
	int NumberOfCompletions(MemoryRequestHeader request);
	void SendCplDRecieveMultitaskingQueueHandling();
	bool IsInSendTimeVector(sc_time check_time_stamp);
	int ReturnIndexOfData(sc_time check_time_stamp);
	void HandleTlpQueue();
	void InsertTlpInQueue();
	void RcAction(); //TODO Root complex stuff
	void RootComplexreceiveAction(); //Activates whenever an endpoint sends a TLP to the RC and it is delvered to the SW layer
	bool ReadMemory(int address, int length, SoftwareData * completionData); //
	int WaitModeledRandomSample();
	void WriteMemory(MemoryRequestHeader, std::vector<int>);
	void WriteMemoryConfirmationWatcher();
	void MemoryDataEventSetter(); //setting data_read_from_memory_available_event when data is available from the memory

	SC_HAS_PROCESS( RootComplex);
	RootComplex(
			sc_module_name name_,
			int number_of_downstream_ports_connected_ = 0,
			int bus_numbers_connected_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 },
			int send_request_period_ = 1000,
			int amount_of_packets_to_be_sent_ = 0,
			int traffic_class_to_use_ = 0,
			PacketType rc_spammer_type_ = MRd,
			int system_memory_address_from_ = 0,
			int system_memory_address_to_ = 1000,
			int port_base_address_registers_base_array_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 },
			int port_base_address_registers_limit_array_[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS] =
					{ 0 }) :
		sc_module(name_) {

		rootcomplex_spammer_type = rc_spammer_type_;
		total_amount_of_requests = amount_of_packets_to_be_sent_;
		send_request_period = send_request_period_;
		requests_traffic_class = traffic_class_to_use_;

		downstream_in_port_vector.init(number_of_downstream_ports_connected_); //Instanciate a dynamic number of ports, set vector length equal to number of downstream ports
		downstream_out_port_vector.init(number_of_downstream_ports_connected_);

		socket = new PcieSocket("EP_1_API", busNumber, deviceNumber,
				functionNumber, useECRC, system_memory_address_from_,
				system_memory_address_to_);
		pcie_switch_pointer = new PcieSwitch("RC_MULTIINPUT_SWITCH", 0, 0, 0,
				number_of_downstream_ports_connected_, bus_numbers_connected_,
				-1, -1, port_base_address_registers_base_array_,
				port_base_address_registers_limit_array_);

		//ROOT-COMPLEX PROCESSES

		SC_THREAD(SendCplDRecieveMultitaskingQueueHandling);
		SC_THREAD(RootComplexInitiateSendAction); //Thread for requestsending
		SC_THREAD(SendQueueHandeling); //Thread for handeling the sendingqueue
		sensitive << send_data_to_software_layer_queue_event;
		SC_THREAD(HandleTlpQueue); //incoming	//TODO change name to receive TLPPQUEHANDELING ish
		sensitive << new_tlp_in_incoming_tlp_queue; //Run the handle thread and empty the buffer, every time a new packet enters the queue
		SC_METHOD(InsertTlpInQueue); //insert incoming TLP in handelingqueue, and notify HandleTlpQueue with new_tlp_in_incoming_tlp_queue
		sensitive << apiToDevCore;
		dont_initialize();
		SC_THREAD(RootComplexreceiveAction); //receives TLP and answer
		//sensitive << apiToDevCore;			//
		SC_THREAD(MemoryDataEventSetter); //Sets event high whenever memory returns data	//changeto method?
		sensitive << data_from_memory;
		SC_THREAD(WriteMemoryConfirmationWatcher); //Notifies Rootcomplexreceive action when data has been writen to the memory and it is ready for new data
		sensitive << write_confirmation_memory;

		for (int i = 0; i < number_of_downstream_ports_connected_; i++) {
			pcie_switch_pointer->downstream_in_port_vector[i](
					downstream_in_port_vector[i]);
			pcie_switch_pointer->downstream_out_port_vector[i](
					downstream_out_port_vector[i]);
		}

		pcie_switch_pointer->upstream_in(switch_socket_wire_downstream); //needs a signal to connect inbetween?
		pcie_switch_pointer->upstream_out(switch_socket_wire_upstream);
		socket->send_tlp_port(switch_socket_wire_downstream);
		socket->receive_tlp_port(switch_socket_wire_upstream);

		//socket->receive_tlp_port(pcie_switch_module_pointer->upstream_out);		//Note to self, input signals has to be casted to output signals, not opositely

		socket->from_device_core(apiFromDevCore);
		socket->to_device_core(apiToDevCore);

		//INIT delay_vector, read from txt file
		char input_file_name[255] = "./trace_data/delay_traces.txt";
		std::ifstream read_from_pex_file(input_file_name);
		std::string line_string;
		int delaysum=0;
		int counter=0;
		while (std::getline(read_from_pex_file, line_string)) {
			//Read through text file and store all numbers in the delay_samples_vector
			delay_samples_vector.push_back(atoi(line_string.c_str()));
			delaysum=delaysum+atoi(line_string.c_str());
			counter++;
		}
		cout <<"Average delay is equal to: " <<delaysum/counter<<endl;
		read_from_pex_file.close();
		std::random_shuffle ( delay_samples_vector.begin(), delay_samples_vector.end() );

	}

	~RootComplex() {
		if (print_full_simulation_information
				|| print_statistics_only_simulation_information) {
			cout
					<< "___________________________________________________________________________________________________________________________"
					<< endl;
			cout << "Simulation statistics: " << endl
					<< "\tFormat: EPs and RC <req_sent/cpl_rec><req_rec/cpl_sent>"
					<< endl;
			//if (print_full_simulation_information) {
				cout
						<< "\tSockets: <sent/received>, Switches, <rec_ds/fwd_us><rec_us/fwd_ds> "
						<< endl;
			//}
			cout
					<< "___________________________________________________________________________________________________________________________"
					<< endl << endl;

			cout << "--RootComplex Destructor--" << endl;
			cout << "\t Packet Statistics: <" << *requests_sent_counter << "/"
					<< *completions_received_counter << "> <"
					<< *requests_received_counter << "/"
					<< *completions_sent_counter << ">"
			//<< "\t Format: <sent_req/rec_cpl> <rec_req/sent_cpl>"
					<< endl;

			if (*completions_received_counter != 0 || *requests_sent_counter
					!= 0 || *requests_received_counter != 0
					|| *completions_sent_counter != 0) {

				if (!print_statistics_summary_information) {
					if (requests_sent_timestamps_vector.size() != 0) {
						cout << "\t\tRequests sent with traffic class "<<requests_traffic_class<< " time stamps:" << endl;
						for (unsigned i = 0; i
								< requests_sent_timestamps_vector.size(); i++) {
							cout << "\t\t\t"
									<< requests_sent_timestamps_vector[i]
									<< endl;
						}
					}
					if (completions_received_timestamps_vector.size() != 0) {
						cout << "\t\tCompletions received time stamps:" << endl;
						//Print out packet-received timestamp when destroying ep
						for (unsigned i = 0; i
								< completions_received_timestamps_vector.size(); i++) {
							cout << "\t\t\t"
									<< completions_received_timestamps_vector[i]
									<< endl;
						}
					}

					if (completions_received_timestamps_vector.size()
							== requests_sent_timestamps_vector.size()) {
						//cout << endl;
						double delay_delta_sum = 0;
						if (requests_sent_timestamps_vector.size() > 0) {

							for (unsigned i = 0; i // TODO assumes in order
									< requests_sent_timestamps_vector.size(); i++) {
								//Assumption strict ordering.
								delay_delta_sum
										= delay_delta_sum
												+ (completions_received_timestamps_vector[i].to_double()
														- requests_sent_timestamps_vector[i].to_double());
							}

							cout << "\t\t\t" << "Average MRd-CplD time is:"
									<< endl << "\t\t\t\t"
									<< (delay_delta_sum)
											/ ((double) completions_received_timestamps_vector.size())
									<< " ps" << endl;
						}
					} else {
						cout
								<< "\tCould not print avg MRd-CplD response delay, number of packets are different"
								<< endl;
					}

					//Print out data when destroying rc
					if (!(received_data_vector.empty())) {
						if (!print_statistics_summary_information) {
							cout << "\t\t\tData received is: " << endl;
							cout << "\t\t\t\t";
							for (unsigned i = 0; i
									< received_data_vector.size(); i++) {
								cout << received_data_vector[i] << " ";
								if ((i + 1) % 5 == 0 && i != 0) {
									//cout <<endl<<i<<endl;
									cout << endl << "\t\t\t\t";
								}
							}
							cout << endl;
						}
					}

					if (requests_received_timestamps_vector.size() != 0) {
						cout << "\t\tRequests received time stamps:" << endl;
						//Print out packet-received timestamp when destroying ep
						cout << "\t\t\t";
						for (unsigned i = 0; i
								< requests_received_timestamps_vector.size(); i++) {
							cout << requests_received_timestamps_vector[i] <<"\t";
							if((i+1)%4==0 &&i!=0){
								cout <<endl;
								cout << "\t\t\t";
							}
						}
						cout<<endl;
					}
					if (completions_sent_timestamps_vector.size() != 0) {
						cout << "\t\tCompletions sent time stamps:" << endl;
						cout << "\t\t\t";
						for (unsigned i = 0; i
								< completions_sent_timestamps_vector.size(); i++) {
							cout << completions_sent_timestamps_vector[i] <<"\t";
							if((i+1)%4==0 &&i!=0){
								cout <<endl;
								cout << "\t\t\t";
							}
						}
						cout<<endl;
					}


				}

			}

		}
		if (!incoming_tlp_queue.empty()) {
			cout << "incoming_tlp_queue not empty" << endl;
		}


		/////////////////////////////////////////////////////////////////////////

		remove("./generated_logs/rc_rand_drawn_deltas.txt");
		std::ofstream write_deltas_to_file("./generated_logs/rc_rand_drawn_deltas.txt");
		for (unsigned i = 0; i
				< wait_sample_vector_for_printouts.size(); i++) {
			//cout<<wait_sample_vector_for_printouts[i]<<" ";
			//if((i+1)%18==0){
			//	cout << endl;
			//}
			write_deltas_to_file << wait_sample_vector_for_printouts[i]<< endl;
		}


		write_deltas_to_file.close();
		delete requests_received_counter;
		delete completions_sent_counter;
		delete requests_sent_counter;
		delete completions_received_counter;
		delete pcie_switch_pointer;
		delete socket;

	}
};

#endif
