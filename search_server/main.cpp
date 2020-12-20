#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

#include "search_server.h"
#include "test_runner.h"
#include "tests.h"

int main() {
	TestSearchServer();
	cout << "Search server testing finished"s << endl;

	return 0;
}