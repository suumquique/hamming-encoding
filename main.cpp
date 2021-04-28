#include "main.h"

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

	string fileToEncodePath; // ���� � �����, ���������� �� �������� ����� ������������ ������� ��������
	string fileForDecodedInformationPath; // ���� � �����, ���� ����� ���������� �������������� ����������
	fstream fileToEncode; // ����, ���������� �� �������� ����� ����������
	fstream fileToDecode; // ����, ��� ��� ����� �������������� �, ��������, ������������ ����������
	fstream fileForDecodedInformation; // ���� ��� ������ �������������� ���������� � ������������� ������� �������� ��������
	unsigned blockSize; // ������ ������ ����� ��� ����������� / �������������
	DWORD dwRetCode; // ��� ��������

	cout << "������� ���������� �������� � ���������� �����: ";
	cin >> blockSize;
	cin.ignore(); // ������� ����� �����


	cout << "������� ���� � �����, ������� ���� ������������ ������� ��������: ";
	cin >> fileToEncodePath;
	cin.ignore();
	fileToEncode.open(fileToEncodePath, ios::in || ios::binary);
	if (!fileToEncode.is_open()) {
		cout << "����, ������� ��������� ������������, �� ���������� ��� ����������.";
		exit(ERROR_FILE_INVALID);
	}

	cout << "\n������ ����� � ����� �� ����������� ����� " << getFileLength(fileToEncode) << endl << endl;

	fileToDecode = encode(fileToEncode, blockSize);

	cout << "\n������ ����� � ����� ����� ����������� ����� " << getFileLength(fileToDecode) << endl << endl;

	cout << "������� ���� � �����, � ������� ����� �������� �������������� ����������: ";
	cin >> fileForDecodedInformationPath;
	dwRetCode = decode(fileToDecode, blockSize, fileForDecodedInformation);
	if (dwRetCode != ERROR_SUCCESS) {
		cout << "������ ��� �������������.";
		exit(ERROR_DECRYPTION_FAILED);
	}

	fileForDecodedInformation.open(fileForDecodedInformationPath, ios::in || ios::binary);
	if (!fileToEncode.is_open()) {
		cout << "����, � ������� ���� �������� �������������� ����������, ����������.";
		exit(ERROR_FILE_INVALID);
	}
	cout << "\n������ ����� � ����� ����� ������������� � �������������� ����� " << getFileLength(fileForDecodedInformation) << endl << endl;
}