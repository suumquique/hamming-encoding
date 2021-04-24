#include "main.h"

/* � �������� ��������� ������ ���� ������� ��� �������� �� ������ � �������� ������� �������� �����.
���������� ������ ����� � ����� */
size_t getFileLength(fstream& file) {

	// �������� ��������� �� ����� �����
	file.seekg(0, ios_base::end);
	// ������, �� ����� ����� ����� ���������
	size_t length_in_bytes = static_cast<size_t>(file.tellg());
	// �������� ��������� ����� � ������ �����
	file.seekg(0, ios_base::beg);

	return length_in_bytes * BITS_IN_BYTE;
}