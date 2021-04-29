#include "main.h"

/* Функция восстанавливает файл, закодированный методом Хэмминга, исправляя возможные одиночные ошибки.

Аргументы:
fstream& binaryFileToDecode - файл, который требуется декдировать, открыт на чтение в двоичном виде.
unsigned blockSize - размер информационного блока кодирования (без учета контрольных бит).
string fileWithDecodedInformationPath - путь к файлу, куда должно быть записано декодированное сообщение.

Возвращает код выполнения, равный ERROR_SUCCESS при успехе. */
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath) {
	
	return ERROR_SUCCESS;
}