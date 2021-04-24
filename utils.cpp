#include "main.h"

/* В качестве аргумента должен быть передан уже открытый на чтение в двоичном формате файловый поток.
Возвращает размер файла в битах */
size_t getFileLength(fstream& file) {

	// Сдвигаем указатель на конец файла
	file.seekg(0, ios_base::end);
	// Узнаем, на каком месте стоит указатель
	size_t length_in_bytes = static_cast<size_t>(file.tellg());
	// Сдвигаем указатель назад в начало файла
	file.seekg(0, ios_base::beg);

	return length_in_bytes * BITS_IN_BYTE;
}