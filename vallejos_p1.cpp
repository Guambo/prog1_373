#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

bool parse_file(std::string filename, std::vector<std::vector<std::string>> &transitions, std::string &start_state, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states);
bool parse_state(std::string state_line, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states, std::string &start_state);
std::string parse_transitions(std::string transition_line);
bool run_machine(std::vector<std::vector<std::string>> &transitions, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states, std::string &sequence_of_states, std::string &input, std::string &start_state, int &max_transitions);

int main(int argc, char **argv) {
	std::string start_state;
	std::vector<std::vector<std::string>> transitions(1000, std::vector<std::string>(0, ""));
	std::vector<std::string> accept_states;
	std::vector<std::string> reject_states;
	std::string sequence_of_states;
	std::string input;
	int max_transitions;

	/* Initialize fields */
	if(argc == 4) {
		sequence_of_states = "";
		input = argv[2];
		max_transitions = atoi(argv[3]);
	}
	else {
		std::cout << "ERROR: not enough cmd args\n";
		return 1;
	}

	if(parse_file(argv[1], transitions, start_state, accept_states, reject_states)) {
		std::string s = "";
		s += start_state;
		if(run_machine(transitions, accept_states, reject_states, sequence_of_states, input, s, max_transitions)) {
			std::cout << sequence_of_states << std::endl;
		}
	}
}

bool parse_file(std::string filename, std::vector<std::vector<std::string>> &transitions, std::string &start_state, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states) {
	std::ifstream file (filename);
	std::string line;
	std::string token;

	if(file.is_open()) {
		while(std::getline(file, line)) {
			token = line.substr(0, line.find("\t"));
			if(token.compare("state") == 0) {
				if(parse_state(line.substr(line.find("\t") + 1), accept_states, reject_states, start_state)) {
					//printf("successfully parsed state line!\n");
					//std::cout << "accept_states=" << accept_states << " | reject_states=" << reject_states << " | start_state=" << start_state  << std::endl;
				}
			}
			else if(token.compare("transition") == 0) {
				token = parse_transitions(line.substr(line.find("\t") + 1));
				int index = std::stoi(token.substr(0,token.find("\t")));
				transitions[index].push_back(token);
			}
		}
	}
	else {
		return false;
	}
	file.close();
	return true;
}

bool parse_state(std::string state_line, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states, std::string &start_state) {
	bool retval = false;
	std::string flag = state_line.substr(state_line.find("\t") + 1);
	if(flag.compare("accept") == 0) {
		accept_states.push_back(state_line.substr(0, state_line.find("\t")));
		retval = true;
	}
	else if(flag.compare("reject") == 0) {
		reject_states.push_back(state_line.substr(0, state_line.find("\t")));
		retval = true;
	}
	else {
		start_state = state_line.substr(0, state_line.find("\t"));
		retval = true;
	}
	return retval;
}

std::string parse_transitions(std::string transition_line) {
	std::string retval = "";
	while(1) {
		retval += transition_line.substr(0, transition_line.find("\t"));
		if(transition_line.find("\t") > transition_line.length() ) {
			break;
		}
		else {
			transition_line = transition_line.substr(transition_line.find("\t") + 1);
		}
		retval += "\t"; // Here we separate each segment of a transition line with a tab
	}

	return retval;
}

bool run_machine(std::vector<std::vector<std::string> > &transitions, std::vector<std::string> &accept_states, std::vector<std::string> &reject_states, std::string &sequence_of_states, std::string &input,  std::string &start_state, int &max_transitions) {
	int current_state = std::stoi(start_state);
	int current_symbol_index = 0;
	int transition_count = 0;
	sequence_of_states += std::to_string(current_state);
	char current_symbol;
	bool retval = false;
	
	while(1) {
		/* Check to see if we are done going through our machine */
		if(transition_count > max_transitions) {
			sequence_of_states += " quit";
			retval = true;
			break;
		}
		else {
			std::string c_state = std::to_string(current_state);
			for(unsigned int i = 0; i < accept_states.size(); i++) {
				if(c_state.compare(accept_states[i]) == 0) {
					sequence_of_states += " accept";
					retval = true;
					return retval;
				}
			}
			
			for(unsigned int i = 0; i < reject_states.size(); i++) {
				if(c_state.compare(reject_states[i]) == 0) {
					sequence_of_states += " reject";
					retval = true;
					return retval;
				}
			}
		}
		
		/* We have to check to make sure that current_symbol_index is in the bounds of input first */
		if(current_symbol_index >= 0 && current_symbol_index < input.length()) {
			current_symbol = input.at(current_symbol_index);
		}
		else {
			current_symbol = '_';
		}
		
		for(unsigned int i = 0; i < transitions[current_state].size(); i++) {
			std::string t = transitions[current_state][i];
			
			if(t.compare("") == 0) {
				continue;
			}
			
			/* Break up the transition into 4 chars: it_symbol nt_state nt_symbol LR */
			std::string temp = t.substr(t.find("\t") + 1); // temp now at initial symbol
			char it_symbol = temp.at(0);
			
			if(it_symbol == current_symbol) {
				/* We have reached the correct transition */
				std::string nt_state = "";

				temp = temp.substr(temp.find("\t") + 1); // temp now at next state
				nt_state += temp.substr(0, temp.find("\t"));

				temp = temp.substr(temp.find("\t") + 1); //temp now at next symbol
				std::string nt_symbol = "";
				nt_symbol += temp.at(0);

				temp = temp.substr(temp.find("\t") + 1); // temp at L/R
				char LR = temp.at(0);
				
				current_state = std::stoi(nt_state);
				// Replace input[current_symbol_index] with nt_symbol IF we are in the bounds of input
				if(current_symbol_index >= 0 && current_symbol_index < input.length()) {
					input.replace(current_symbol_index, 1, nt_symbol);
				}
				
				current_symbol_index += (LR == 'R') ? 1 : -1;
				
				++transition_count;
				if(transition_count > max_transitions) {
					break;
				}
				
				std::string str = "->" + std::to_string(current_state);
				sequence_of_states += str;
				break;
			}
		}
	}
	return retval;
}

