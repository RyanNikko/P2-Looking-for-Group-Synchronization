//Ryan Nicholas A. Taino - S11

#include<iostream>

using namespace std;

int main() {
	int n; //maximum number of concurrent instances
	int t, h, d; //number of tank/healer/dps players in the queue
	int t1, t2; //minimum/maximum time before an instance is finished

	cout << "Max number of parties within the dungeon: ";
	cin >> n;

	cout << endl; 

	cout << "How many tank players are in the queue?: ";
	cin >> t;

	cout << endl; 

	cout << "How many healer players are in the queue?: ";
	cin >> h; 

	cout << endl; 
	
	cout << "How many dps players are in the queue?: ";
	cin >> d; 

	cout << endl; 

	cout << "Minimum time to finish a raid?: ";
	cin >> t1; 

	cout << endl; 

	cout << "Maximum time to finish a raid?: ";
	cin >> t2;

	cout << endl; 

	cout << "Parameters: " << endl;
	cout << "Max # of parties in dungeon: " << n << endl;  
	cout << "Tanks: " << t << endl;
	cout << "Healers: " << h << endl;
	cout << "DPSs: " << d << endl;  
	cout << "It should take a minimum of " << t1; 
	cout << " and a maximum of " << t2 << " to finish a raid." << endl;

	return 0;
}