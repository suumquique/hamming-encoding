#include "main.h"

int main(void) {
	SetConsoleCP(RUS_ENCODING);
	SetConsoleOutputCP(RUS_ENCODING);

	string fileToEncodePath; // Путь к файлу, информация из которого будет закодирована методом Хемминга
	string fileForDecodedInformationPath; // Путь к файлу, куда будем записывать декодированную информацию
	fstream fileToEncode; // Файл, информацию из которого будем кодировать
	fstream fileToDecode; // Файл, где уже лежит закодированная и, возможно, поврежденная информация
	fstream fileForDecodedInformation; // Файл для записи декодированной информации с исправленными методом Хемминга ошибками
	unsigned blockSize; // Размер одного блока для кодирования / декодирования
	DWORD dwRetCode; // Код возврата

	cout << "Введите количество разрядов в кодируемом блоке: ";
	cin >> blockSize;
	cin.ignore(); // Очищаем поток ввода


	cout << "Введите путь к файлу, который надо закодировать методом Хемминга: ";
	cin >> fileToEncodePath;
	cin.ignore();
	fileToEncode.open(fileToEncodePath, ios::in || ios::binary);
	if (!fileToEncode.is_open()) {
		cout << "Файл, который требуется закодировать, не существует или недоступен.";
		exit(ERROR_FILE_INVALID);
	}

	cout << "\nРазмер файла в битах до кодирования равен " << getFileLength(fileToEncode) << endl << endl;

	fileToDecode = encode(fileToEncode, blockSize);

	cout << "\nРазмер файла в битах после кодирования равен " << getFileLength(fileToDecode) << endl << endl;

	cout << "Введите путь к файлу, в который будет записана декодированная информация: ";
	cin >> fileForDecodedInformationPath;
	dwRetCode = decode(fileToDecode, blockSize, fileForDecodedInformation);
	if (dwRetCode != ERROR_SUCCESS) {
		cout << "Ошибка при декодировании.";
		exit(ERROR_DECRYPTION_FAILED);
	}

	fileForDecodedInformation.open(fileForDecodedInformationPath, ios::in || ios::binary);
	if (!fileToEncode.is_open()) {
		cout << "Файл, в который была помещена декодированная информация, недоступен.";
		exit(ERROR_FILE_INVALID);
	}
	cout << "\nРазмер файла в битах после декодирования и восстановления равен " << getFileLength(fileForDecodedInformation) << endl << endl;
}