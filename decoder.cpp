#include "main.h"

/* Функция восстанавливает файл, закодированный методом Хэмминга, исправляя возможные одиночные ошибки.

Аргументы:
fstream& binaryFileToDecode - файл, который требуется декдировать, открыт на чтение в двоичном виде.
unsigned blockSize - размер информационного блока кодирования (без учета контрольных бит).
string fileWithDecodedInformationPath - путь к файлу, куда должно быть записано декодированное сообщение.

Возвращает код выполнения, равный ERROR_SUCCESS при успехе. */
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath) {
	// Количество значащих бит в закодированном файле (вычитаем один байт, так как в первом байте лежит информация о длине остатка)
	size_t significantBitsInEncodedFileNumber = getFileLength(binaryFileToDecode) - 1 * BITS_IN_BYTE;
	// Файловый поток для записи декодированного сообщения
	ofstream fileForDecodedInformation(fileForDecodedInformationPath, ios_base::binary);
	unsigned char residualBlockSize; // Размер незакодированного остаточного блока в битах (если он есть)
	// Считываем первый байт файла, в котором лежит размер остаточного блока в битах
	binaryFileToDecode.read((char*)&residualBlockSize, sizeof(residualBlockSize)); 
	// Размер закодированного остаточного блока в битах (если он есть)
	unsigned char residualEncodedBlockSize = getEncodedBlockLength(residualBlockSize);
	size_t encodedBlockLength = getEncodedBlockLength(blockSize); // Размер закодированного блока в битах
	// Размер одного закодированного блока в файле в битах (поскольку один блок лежит в целом количестве байт)
	size_t fullBlockSizeInBites = ceil((double)encodedBlockLength / BITS_IN_BYTE) * BITS_IN_BYTE;
	// Количество целых блоков в закодированном файле
	size_t fullBlockInEncodedFileNumber = significantBitsInEncodedFileNumber / fullBlockSizeInBites;
	// Текущий битсет для записи блока из файла и текущий декодированный (восстановленный) блок
	bitset<MAX_BLOCK_LENGTH> currentBitset, currentDecodedBlock;
	
	for (size_t i = 0; i < fullBlockInEncodedFileNumber; i++) {
		currentBitset = readBlock(binaryFileToDecode, fullBlockSizeInBites);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, encodedBlockLength);
	}
	
	return ERROR_SUCCESS;
}

bitset<MAX_BLOCK_LENGTH> decodeAndRestoreBlock(bitset<MAX_BLOCK_LENGTH> encodedBlock, unsigned encodedBlockSize) {
	bitset<MAX_BLOCK_LENGTH> decodedBlock; // Декодированный блок без контрольных бит
	size_t currentBitIndex; // Индекс текущего бита в битсете
	size_t errorBitPosition = NO_ERROR_BITS; // Номер ошибочного бита, который требуется инвертировать. По умолчанию такого бита нет
	bool currentControlBit = 0; // Текущий контрольный бит, который мы проверяем

	// Проходимся по всем контрольным битам (степеням двойки)
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockSize + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		for (size_t i = currentBitIndex; i < encodedBlockSize; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber; k++) {
				// Считаем четность с помощью операции xor, поскольку нулевые биты не должны влиять на четность
				if (k != currentBitIndex) currentControlBit ^= encodedBlock[k];
			}
		}

		/* Если значение текущего контрольного бита, вычисленное из зависимых информационных битов, не совпадает
		* с ожидаемым значением (четностью) контрольного бита, то добавляем его позицию в номер позиции ошибочного бита */
		if (currentControlBit != encodedBlock[currentBitIndex]) {
			if (errorBitPosition == NO_ERROR_BITS) errorBitPosition = 0;
			errorBitPosition += currentBitNumber;
		}
		// Зануляем контрольный бит, чтобы на следующей итерации начать с чистого листа
		currentControlBit = 0;
	}

	// Если нашелся ошибочный бит
	if (errorBitPosition != NO_ERROR_BITS) {
		// Вычитаем единицу, потому что мы должны будем брать бит по индексу, а позиция у нас выступает в качестве номера
		errorBitPosition--;
		encodedBlock.flip(errorBitPosition); // Инвертируем ошибочный бит
	}

	// Переносим в декодированный блок все информационные биты из закодированного
	for (size_t i = 0, currentBitIndex = 0; i < encodedBlockSize; i++) {
		// Пропускаем контрольные биты
		if (isIndexPowerOfTwo(i)) continue;
		decodedBlock[currentBitIndex++] = encodedBlock[i];
	}

	return decodedBlock;
}