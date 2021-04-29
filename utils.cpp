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

// Возвращает размер закодированного блока, то есть количество информационных и контрольных бит, вместе взятых
size_t getEncodedBlockLength(unsigned blockSize) {
	// Количество контрольных бит - двоичный логарифм от количества бит в блоке, округленный в большую сторону
	unsigned controlBitsNumber = floor(log2(blockSize) + 1);

	/* Иногда при добавлении контрольных бит получается ситуация, что в сумме с информационными их становится столько,
	что требуется добавлять еще один контрольный бит (например, 12 битов информационных + 4 контрольных => становится 16 бит, 
	а 16 должен быть контрольным, следовательно, требуется добавить еще один контрольный бит на 16 позицию и перенести последний 
	информационный на 17-ю позицию) */
	while (floor(log2(blockSize + controlBitsNumber) + 1) > controlBitsNumber) {
		controlBitsNumber++;
	}

	return blockSize + controlBitsNumber;
}

/*Функция считывает из файла требуемое количество бит (по размеру блока), делая это побайтово.
Если количество требуемых бит для считывания не кратно количеству бит в байте (восьми), то незаписанный в текущий блок
остаток сохраняется в статической переменной до следующего вызова функции. При окончании работы с файлом остатки обнуляются.

В качестве аргументов принимает файловый поток с информацией, открытый на чтение в двоичном виде (binaryInputFile) и
размер одного блока БЕЗ контрольных бит (только количество информационных - blockSizeInBites).

Возвращает массив битов, в конце которого записано значение текущего блока; то есть, если блок - 64 бита, то 
последние 64 бита bitset`а будут содержать считанные значения*/
bitset<MAX_BLOCK_LENGTH> readBlock(fstream& binaryInputFile, unsigned blockSizeInBites) {
	// Статическая переменная для оставшихся бит, если размер блока не кратен восьми
	static bitset<MAX_BLOCK_LENGTH> residualBits;
	// Количество оставшихся с прошлого вызова функции бит
	static size_t residualBitsCount = 0;

	char temp; // Временная переменная для считывания одного байта из потока
	bitset<8> tempByte; // Временная переменная из восьми бит для работы с одним байтом из потока
	unsigned currentBitNumber = 0; // Номер текущего бита
	bitset<MAX_BLOCK_LENGTH> currentBlock; // Текущий блок кодирования

	// Если у нас есть остаток с прошлого вызова функции, сначала записываем в текущий битсет остаток
	while (residualBitsCount != 0 && currentBitNumber < blockSizeInBites) {
		currentBlock[currentBitNumber] = residualBits[currentBitNumber];
		currentBitNumber++;
		residualBitsCount--;
	}

	// Читаем из файла, пока номер текущего бита меньше, чем размер блока (мы еще не полностью заполнили блок)
	while (true) {
		binaryInputFile.read((char*)&temp, sizeof(temp));
		/* Поскольку флаг eof устанавливается исключительно в том случае, когда уже произошла попытка чтения за границей файла,
		* мы проверяем границу фойла после считывания, а цикл идет while(true) */
		if (binaryInputFile.eof()) return currentBlock;
		tempByte = bitset<8>(temp);

		// Читаем текущий байт побитово
		for (size_t i = 0; i < BITS_IN_BYTE; i++) {
			// Пока номер текущего бита меньше, чем размер блока, переносим биты из текущего байта в блок
			if (currentBitNumber < blockSizeInBites) currentBlock[currentBitNumber++] = tempByte[i];
			// Если при считывании битов из текущего байта мы полностью заполнили блок и остались еще биты
			else {
				// Записываем оставшиеся биты в "перенос" (остаток) до следующего вызова функции
				for (size_t k = i; k < BITS_IN_BYTE; k++) {
					residualBits[k - i] = tempByte[k];
					residualBitsCount++;
				}

				// Поскольку блок уже заполнен, возвращаем его
				return currentBlock;
			}
		}

		

		if (currentBitNumber == blockSizeInBites) return currentBlock;
	}

	// Поскольку, если программа дошла до сюда, файл дочитан до конца, то мы скидываем имеющийся остаток и количество бит в остатке
	residualBits.reset();
	residualBitsCount = 0;
}
