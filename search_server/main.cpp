#include <iostream>

#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"

using namespace std;

int main() {
	SearchServer search_server("� � ��"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "������� ��� ������ ������� "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });


	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("������ ������"s);
	}

	request_queue.AddFindRequest("�������� ��"s);
	request_queue.AddFindRequest("������� �������"s);
	request_queue.AddFindRequest("�������"s);
	cout << "��������, �� ������� ������ �� ������� "s << request_queue.GetNoResultRequests();

	return 0;
}