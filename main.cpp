#include <iterator>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <cassert>

#include "for_each_combination.h"
#include "pairings.h"
#include "lap.h"

using namespace std;

template <typename T>
void printVec(vector<T> values) {
	cout << "["; copy(values.begin(), values.end(), ostream_iterator<T>(cout," ")); cout << "]" << endl;
}

class Rooming {
public:
	Rooming(int n_girls, int n_boys, int n_singles, vector<string> room_names) :
		n_girls(n_girls), n_boys(n_boys), n_singles(n_singles),
		n_doubles(room_names.size() - n_singles), room_names(room_names),
		graphs_tried(0), best_utility(0), max_utility_from_rooms(0.0)
		{person_names.reserve(n_girls+n_boys); people_in_room.resize(n_singles+n_doubles);}

	void add_person(bool is_male, string name, vector<double>&& room_pref,  vector<double>&& mate_pref) {
		max_utility_from_rooms += *std::max_element(std::begin(room_pref), std::end(room_pref)); 
		room_utility.push_back(room_pref);
		vector<double> pad(is_male ? n_girls : n_boys, -1e333);
		mate_pref.insert(is_male ? mate_pref.begin() : mate_pref.end(), pad.begin(), pad.end());
		roommate_utility.push_back(move(mate_pref));
		person_names.push_back(name);
	};

	void run() {
		assert(roommate_utility.size() == n_girls + n_boys);
		for ( const auto& v : roommate_utility ) printVec(v);
		for ( const auto& v : room_utility ) printVec(v);
		for ( const auto& v : roommate_utility ) assert(v.size() == n_girls + n_boys);
		for ( const auto& v : room_utility ) assert(v.size() == room_names.size());
		assert(2*n_doubles + n_singles == n_girls + n_boys);

		assigncost = new double*[n_doubles + n_singles];
		for (int i = 0; i<n_doubles + n_singles; i++) {
			assigncost[i] = new double[n_doubles + n_singles];
		}
		rowsol = new int[n_doubles + n_singles];
		colsol = new int[n_doubles + n_singles];
		out_u = new double[n_doubles + n_singles];
		out_v = new double[n_doubles + n_singles];

		for (doubles_for_girls = 0; doubles_for_girls<=n_doubles; ++doubles_for_girls) {
			doubles_for_boys = n_doubles - doubles_for_girls;
			singles_for_girls = n_girls - 2*doubles_for_girls;
			singles_for_boys = n_boys - 2*doubles_for_boys;
			if (singles_for_boys < 0 || singles_for_girls < 0) continue;

			cout << endl << doubles_for_girls << " " << singles_for_girls << " " << doubles_for_boys << " " << singles_for_boys << endl;
			vector<int> girls(n_girls, 0); for(int i=0; i<n_girls; ++i) girls[i] = i;
			for_each_combination(girls.begin(), girls.begin()+2*doubles_for_girls, girls.end(),
							[&](vector<int>::iterator gitbeg, vector<int>::iterator gitend) {
				girls_in_doubles_begin = gitbeg;
				girls_in_doubles_end = gitend;
				girl_in_double = 0;
				for (auto it = girls_in_doubles_begin; it != girls_in_doubles_end; ++it) girl_in_double |= 1<<(*it);
				vector<int> boys(n_boys, 0); for(int i=0; i<n_boys; ++i) boys[i] = n_girls+i;
				for_each_combination(boys.begin(), boys.begin()+2*doubles_for_boys, boys.end(),
					[this](vector<int>::iterator a, vector<int>::iterator b) {return this->step(a,b);} );
				return false;
			});
		}
	}

	bool step(vector<int>::iterator boys_in_doubles_begin, vector<int>::iterator boys_in_doubles_end) {
		uint64_t boy_in_double = 0;
		for (auto it = boys_in_doubles_begin; it != boys_in_doubles_end; ++it) boy_in_double |= 1<<(*it);
		for (pairings girl_pairs(doubles_for_girls*2); girl_pairs; ++girl_pairs) {
			for (pairings boy_pairs(doubles_for_boys*2); boy_pairs; ++boy_pairs) {
				vector<pair<int,int> > roomeds; roomeds.reserve(n_singles+n_doubles);
				for (const auto& gp : *girl_pairs) {
					auto l = *(girls_in_doubles_begin + gp.first);
					auto r = *(girls_in_doubles_begin + gp.second);
					assert(girl_in_double&(1<<l));
					assert(girl_in_double&(1<<r));
					roomeds.push_back(make_pair(l, r));
				}
				for (int g=0; g<n_girls; g++) if (!(girl_in_double&(1<<g))) {
					roomeds.push_back(make_pair(g,g));
				}
				for (const auto& bp : *boy_pairs) {
					auto l = *(boys_in_doubles_begin + bp.first);
					auto r = *(boys_in_doubles_begin + bp.second);
					assert(boy_in_double&(1<<l));
					assert(boy_in_double&(1<<r));
					roomeds.push_back(make_pair(l, r));
				}
				for (int b=n_girls; b<n_girls+n_boys; b++) if (!(boy_in_double&(1<<b))) {
					roomeds.push_back(make_pair(b,b));
				}
				assert(n_girls+n_boys-n_doubles == n_singles+n_doubles);
				assert(roomeds.size() == n_singles+n_doubles);

				for (int i=0; i<n_singles+n_doubles; ++i) {
					for (int j=0; j<n_singles+n_doubles; ++j) assigncost[i][j] = 1e444;
				}
				double current_utility = 0;
				for (int i=0; i<roomeds.size(); ++i) {
					int l = roomeds[i].first, r = roomeds[i].second;
					current_utility += roommate_utility[l][r];
					// cout << '(' << l << ',' << r << ") ";
					if (l==r) for (int room = 0; room<n_singles; ++room) {
						assigncost[i][room] = - (room_utility[l][room]);
					} else for (int room = n_singles; room<n_singles+n_doubles; ++room) {
						assigncost[i][room] = - (room_utility[l][room] + room_utility[r][room]);
					}
				}
				if (current_utility + max_utility_from_rooms < best_utility) continue;
				current_utility -= lap(n_singles+n_doubles, assigncost, rowsol, colsol, out_u, out_v);

				if (current_utility > best_utility) {
					best_utility = current_utility;
					for (int room = 0; room < n_singles+n_doubles; ++room) {
						people_in_room[room] = roomeds[colsol[room]];
					}
					cout << best_utility << " ";
					for ( const auto& room : this->get() ) {
						cout << room.first<<":";
						cout << room.second[0];
						if (room.second.size() == 2) cout << ',' << room.second[1];
						cout << " ";
					}
					cout << endl;
				}

				graphs_tried++;
				if (graphs_tried % 10000 == 0) {
					cout << graphs_tried << endl;
				}
			}
		}
		return false;
	}

	vector< pair<string, vector<string> > > get() {
		vector< pair< string, vector<string> > > ret;
		for (int room = 0; room < n_singles+n_doubles; ++room) {
			auto people = people_in_room[room];
			int l = people.first, r = people.second;
			vector<string> rr;
			rr.push_back(person_names[l]);
			if (r != l) rr.push_back(person_names[r]);
			ret.push_back(make_pair( room_names[room], rr ));
		}
		return ret;
	}

private:
	// inputs:
	int n_girls, n_boys, n_singles, n_doubles;
	vector<string> room_names;
	vector<string> person_names;
	vector< vector<double> > room_utility; // person -> room -> utility
	vector< vector<double> > roommate_utility; // person -> person -> utility. Singles are modelled as roommate = self

	// intermediates:
	double max_utility_from_rooms;
	int singles_for_girls, singles_for_boys, doubles_for_girls, doubles_for_boys;
	vector<int>::iterator girls_in_doubles_begin, girls_in_doubles_end;
	uint64_t girl_in_double;
	double **assigncost;
	double *out_u, *out_v;
	int *rowsol, *colsol;

	// results:
	double best_utility;
	vector<pair<int,int> > people_in_room;
public:
	long long int graphs_tried;
};

template <typename T>
vector<T> line(ifstream& ifs, T) {
	string wordline;
	getline(ifs, wordline);
	istringstream words_iss(wordline);
	return vector<T>{istream_iterator<T>{words_iss}, istream_iterator<T>{}};
}

int main () {
	ifstream f_rooms("rooms.txt");
	ifstream f_girls("girls.txt");
	ifstream f_boys("boys.txt");
	int n_singles = 0;
	vector<string> room_names(line(f_rooms, string()));
	vector<string> girls_names(line(f_girls, string()));
	vector<string> boys_names(line(f_boys, string()));

	int c_prev = 0;
	for ( auto c : line(f_rooms, int())) {
		cout << c << ">=" << c_prev << endl;
		assert(c == 1 || c == 2);
		assert(c >= c_prev /* all singles must come before all doubles*/);
		if (c == 1) n_singles++;
		c_prev = c;
	}
	assert(c_prev != 0);

	Rooming rooming(girls_names.size(), boys_names.size(), n_singles, room_names);

	for ( const auto& name : girls_names ) {
		string name_, name__;
		f_girls >> name_;
		f_rooms >> name__;
		assert(name == name_);
		assert(name == name__);
		vector <double> mate_pref = line(f_girls, double());
		vector <double> room_pref = line(f_rooms, double());
		rooming.add_person(false, name, move(room_pref), move(mate_pref));
	}

	for ( const auto& name : boys_names ) {
		string name_, name__;
		f_boys >> name_;
		f_rooms >> name__;
		assert(name == name_);
		assert(name == name__);
		vector <double> mate_pref = line(f_boys, double());
		vector <double> room_pref = line(f_rooms, double());
		rooming.add_person(true, name, move(room_pref), move(mate_pref));
	}
	
	rooming.run();

	cout << endl;
	for ( const auto& room : rooming.get() ) {
		cout << room.first<<":";
		cout << room.second[0];
		if (room.second.size() == 2) cout << ',' << room.second[1];
		cout << " ";
	}
	cout << endl;
	cout << rooming.graphs_tried << endl;
}
