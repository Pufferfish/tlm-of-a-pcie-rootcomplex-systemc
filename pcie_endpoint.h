//
//  pcie_endpoint.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even_pcie_endpoint_h__
#define __Even_pcie_endpoint_h__

#include "systemc.h"
#include "tlp.h"
#include "pcie_socket.h"
#include "compute_pi_bellards_formula.h"
#include <queue>
#include <fstream> //used in the destructor to write MRd-cpl delays

using namespace std;

#define ENDPOINT_MEMORY_SIZE 500	// a subset of these 500 memory-locations are also mappen in the PCIe system memory
extern bool print_full_simulation_information;
extern bool print_data_sent_only_simulation_information;
extern bool print_statistics_only_simulation_information;
extern bool print_statistics_summary_information;
extern bool print_full_and_system_memory;
extern int endpoint_request_read_length;
extern int endpoint_request_write_length;
extern sc_mutex simulation_tlp_number_counter_mutex;
extern int simulation_tlp_number_counter;
extern bool rc_is_reader__init_mem_var;
/**	_________________
 *	|     PCIE-EP	 |
 *	|	  			 |
 *	|____v___^_______|
 *	|				 |
 *	|	Socket		 |
 *	|      			-|->	Upstream
 *	|_______________-|-<	Downstream
 */SC_MODULE(PcieEndPoint) {

	PcieSocket *socket;

	//Internal, input and output signals
	sc_in<TLP> apireceiveTLPChannel; //in
	sc_out<TLP> apiSendTLPChannel; //Out
	sc_signal<SoftwareData> apiFromDevCore; //internal
	sc_signal<TLP> apiToDevCore; //internal

	//Keep track of packets sent and received, decleared on heap memory to be available in destructor.
	int *requests_sent_counter = new int(0);
	int *completions_received_counter = new int(0);
	int *total_completions_received_counter = new int(0);

	sc_mutex requests_sent_counter_mutex;
	sc_mutex completions_received_counter_mutex;

	int *requests_received_counter = new int(0);
	int *completions_sent_counter = new int(0);

	sc_mutex requests_received_counter_mutex;
	sc_mutex completions_sent_counter_mutex;

	//Endpoint specific functions
	void InitializePacketDataToSocket(SoftwareData&, PacketType);
	void SendRequestAction();
	void receivePacketAction();
	void PrintEndpointSubSystemMemory();
	void SendQueueHandeling();
	bool TimeStampsAreSorted();

	//Send amount and period, set in constructor.
	int send_memory_read_request_period; //Period for sending packets
	int total_amount_of_requests; //Send a total of total_amount_of_requests packets

	PacketType endpoint_spammer_type; //set in the constructor, either MRd or MWr

	//Help variables for sending
	int traffic_class_to_use = 0;
	//ConfigurationRegister configuration_register; //isnt this also located within the socket ? ////BOKMERKE

	std::vector<sc_time> total_received_completions_timestamp;	//remove

	//timestamps sent and completed tlps
	std::vector<sc_time> received_completions_timestamp;
	std::vector<sc_time> sent_requests_timestamp;
	//Map timestamps to correct tlp
	std::vector<int> received_completion_tlp_number_vector;
	std::vector<int> sent_request_tlp_number_vector;
	std::vector<sc_time> received_requests_timestamp;
	std::vector<sc_time> sent_completions_timestamp;

	//Data recieved tempstorage
	std::vector<int> endpoint_received_data_vector;

	int endpoint_memory[ENDPOINT_MEMORY_SIZE];

	//SendHandeler
	std::queue<SoftwareData> send_data_to_software_layer_queue;
	sc_mutex send_data_to_software_layer_queue_mutex;
	sc_event send_data_to_software_layer_queue_event;

	bool ep_is_requester;

	SC_HAS_PROCESS( PcieEndPoint);
	PcieEndPoint(sc_module_name name_, int bnr_ = 0, int dnr_ = 0,
			int fnr_ = 0, bool use_ecrc_ = false,
			int send_request_period_ = 1000,
			int amount_of_packets_to_be_sent = 0,
			int traffic_class_to_use_ = 0,
			PacketType endpoint_spammer_type_ = MRd,
			int system_memory_address_from_ = 1001,
			int system_memory_address_to_ = 1100) :
		sc_module(name_) {
		//
		if (amount_of_packets_to_be_sent != 0) {
			ep_is_requester = true;
		} else {
			ep_is_requester = false;
		}
		for (int i = 0; i < ENDPOINT_MEMORY_SIZE; i++) {

			if(rc_is_reader__init_mem_var){
				endpoint_memory[i] = i; //Initialize memory to numbers ranging from 1 2 3 4 5 .. ENDPOINT_MEMORY_SIZE
			}else{
				endpoint_memory[i] = 0; //Initialize memory to numbers ranging from 1 2 3 4 5 .. ENDPOINT_MEMORY_SIZE
			}
		}

		endpoint_spammer_type = endpoint_spammer_type_;
		send_memory_read_request_period = send_request_period_;
		total_amount_of_requests = amount_of_packets_to_be_sent;
		traffic_class_to_use = traffic_class_to_use_;

		//Internal Modules
		socket = new PcieSocket("EP_1_API", bnr_, dnr_, fnr_, use_ecrc_,
				system_memory_address_from_, system_memory_address_to_);
		socket->configuration_register.busNumber = bnr_;
		socket->configuration_register.deviceNumber = dnr_;
		socket->configuration_register.functionNumber = fnr_;

		//Processes
		SC_THREAD(SendQueueHandeling);
		sensitive << send_data_to_software_layer_queue_event;
		SC_THREAD(SendRequestAction);

		SC_METHOD(receivePacketAction);
		sensitive << apiToDevCore;
		dont_initialize();

		//Route signals to api
		socket->receive_tlp_port(apireceiveTLPChannel);
		socket->send_tlp_port(apiSendTLPChannel);
		socket->from_device_core(apiFromDevCore);
		socket->to_device_core(apiToDevCore);
	}

	~PcieEndPoint() {





		if((sent_requests_timestamp.size() != 0
								|| received_completions_timestamp.size() != 0
								|| received_requests_timestamp.size() != 0
								|| sent_completions_timestamp.size() != 0)){
					if(print_statistics_summary_information){
							if ((sent_requests_timestamp.size() != 0)
									&& (sent_requests_timestamp.size()
											== received_completions_timestamp.size())) {
								remove("./generated_logs/ep_mrd_cpl_deltas.txt");
								std::ofstream write_deltas_to_file("./generated_logs/ep_mrd_cpl_deltas.txt");

								double delta;

								for (unsigned i = 0; i
										< sent_requests_timestamp.size(); i++) {
									delta=(received_completions_timestamp[i].to_double()
											- sent_requests_timestamp[i].to_double());
									write_deltas_to_file << (delta/1000) << endl;
								}
								write_deltas_to_file.close();
							}
					}


		if ((print_full_simulation_information
				|| print_statistics_only_simulation_information)
				&& (sent_requests_timestamp.size() != 0
						|| total_received_completions_timestamp.size() != 0
						|| received_requests_timestamp.size() != 0
						|| sent_completions_timestamp.size() != 0)) {

			cout << "--EndPoint: " << socket->configuration_register.busNumber
					<< socket->configuration_register.deviceNumber
					<< socket->configuration_register.functionNumber << " Destructor" << "--"
					<< endl;
			//			cout << "\tPackets received vs packets sent are: "<<*completions_received_counter<<"/"<<*requests_sent_counter<<endl;
			cout << "\t Packet Statistics: <" << *requests_sent_counter << "/"
					<< *total_completions_received_counter << "> <"
					<< *requests_received_counter << "/"
					<< *completions_sent_counter << ">"
			//<< "\t Format: <sent_req/rec_cpl> <rec_req/sent_cpl>"
					<< endl;

			if (*completions_received_counter != 0 || *requests_sent_counter!= 0 || *requests_received_counter != 0|| *completions_sent_counter != 0) {
				//SORT sent request timestamps to correspond to recieved completion timestamps.
				if (received_completions_timestamp.size() == sent_requests_timestamp.size()&&received_completion_tlp_number_vector.size()==sent_request_tlp_number_vector.size()) {
					unsigned number_of_time_stamps=sent_request_tlp_number_vector.size();
					//unsigned temp_iterator_storage=0;
					while(!TimeStampsAreSorted()){
						for(unsigned i=0;i<number_of_time_stamps;i++){	//myvector.erase (myvector.begin()+5);
							//cout<<sent_request_tlp_number_vector[i]<<" "<<received_completion_tlp_number_vector[i]<<endl;
							if(sent_request_tlp_number_vector[i]!=received_completion_tlp_number_vector[i]){//move timestamp and index to the back of the list
								for(unsigned j=(i+1);j<number_of_time_stamps;j++){
									//cout <<sent_request_tlp_number_vector[j]<<received_completion_tlp_number_vector[j]<<endl;
									if(sent_request_tlp_number_vector[i]==received_completion_tlp_number_vector[j]){
										//swap recieved timestamp and nrid with the ones in location j.
										std::swap(received_completion_tlp_number_vector[i],received_completion_tlp_number_vector[j]);
										std::swap(received_completions_timestamp[i],received_completions_timestamp[j]);
										break;
									}
								}
							}

						}
					}
				}
				if (!print_statistics_summary_information) {
					if (sent_requests_timestamp.size() != 0) {
						cout << "\t\tRequests sent with traffic class "<<traffic_class_to_use<< "  time stamps:" << endl;
						cout << "\t\t\t";
						for (unsigned i = 0; i
								< sent_requests_timestamp.size(); i++) {
							cout << sent_requests_timestamp[i] <<"\t";
							if((i+1)%4==0 &&i!=0){
								cout <<endl;
								cout << "\t\t\t";
							}
						}
						cout<<endl;
					}

					if (received_completions_timestamp.size() != 0) {
						cout << "\t\tCompletions received time stamps:" << endl;
						//Print out packet-received timestamp when destroying ep
						cout << "\t\t\t";
						for (unsigned i = 0; i
								< total_received_completions_timestamp.size(); i++) {
							cout << total_received_completions_timestamp[i] <<"\t";
							if((i+1)%4==0 &&i!=0){
								cout <<endl;
								cout << "\t\t\t";
							}
						}
						cout<<endl;
					}
					if (received_completions_timestamp.size() == sent_requests_timestamp.size()) {
						cout << endl;

						double delay_delta_sum = 0;

						if ((sent_requests_timestamp.size() != 0)
								&& (sent_requests_timestamp.size()
										== received_completions_timestamp.size())) {
							std::ofstream write_deltas_to_file("./generated_logs/ep_mrd_cpl_deltas.txt");

							double delta;

							for (unsigned i = 0; i
									< sent_requests_timestamp.size(); i++) {
								delta=(received_completions_timestamp[i].to_double()
										- sent_requests_timestamp[i].to_double());
								write_deltas_to_file << (delta/1000) << endl;

								//Assumption strict ordering.
								delay_delta_sum
										= delay_delta_sum
												+ delta;

							}
							write_deltas_to_file.close();
							cout << "\t\t\t" << "Average MRd-CplD time is:"
									<< endl << "\t\t\t\t"
									<< (delay_delta_sum)
											/ ((double) received_completions_timestamp.size())
									<< " ps" << endl;
						} else if ((sent_requests_timestamp.size() != 0)
								&& (sent_requests_timestamp.size()
										!= received_completions_timestamp.size())) {
							cout
									<< "Could not print Average MRd-CplD time, number of requests does not equal the number of completions"
									<< endl;
						}
					}

					//Print out data when destroying ep
					if (!(endpoint_received_data_vector.empty())) {
						if (!print_statistics_summary_information) {
							cout << "\t\t\tData received is: " << endl;
							cout << "\t\t\t\t";
							for (unsigned i = 0; i
									< endpoint_received_data_vector.size(); i++) {
								cout << endpoint_received_data_vector[i] << " ";
								if ((i + 1) % 5 == 0 && i != 0) {
									//cout <<endl<<i<<endl;
									cout << endl << "\t\t\t\t";
								}
							}
							cout << endl;
						}
					}

					if (received_requests_timestamp.size() != 0) {
						cout << "\t\tRequests received time stamps:" << endl;
						//Print out packet-received timestamp when destroying ep
						cout << "\t\t\t";
						for (unsigned i = 0; i
								< received_requests_timestamp.size(); i++) {
							cout << received_requests_timestamp[i] <<"\t";
							if((i+1)%4==0 &&i!=0){
								cout <<endl;
								cout << "\t\t\t";
							}
						}
						cout<<endl;
					}
					if (sent_completions_timestamp.size() != 0) {
						cout << "\t\tCompletions sent time stamps:" << endl;
					cout << "\t\t\t";
					for (unsigned i = 0; i
							< sent_completions_timestamp.size(); i++) {
						cout << sent_completions_timestamp[i] <<"\t";
						if((i+1)%4==0 &&i!=0){
							cout <<endl;
							cout << "\t\t\t";
						}
					}
					cout<<endl;
					}

				}

			}
			if (print_full_and_system_memory) {
				PrintEndpointSubSystemMemory();
			}
			cout << endl << endl;
		}
		}

		delete requests_received_counter;
		delete completions_sent_counter;
		delete requests_sent_counter;
		delete completions_received_counter;
		delete socket;
	}
};

#endif
