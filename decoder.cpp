#include "main.hpp"

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
	// Размер закодированного остатка в битах
	unsigned char residualEncodedBlockSize = getEncodedBlockLength(residualBlockSize);
	// Размер закодированного остаточного блока в файле целых байтах, переведенный в биты
	unsigned char residualEncodedBlockInFileSize = getIntegerBlockSizeInBytes(residualEncodedBlockSize) * BITS_IN_BYTE;
	size_t encodedBlockLength = getEncodedBlockLength(blockSize); // Размер закодированного блока в битах
	// Размер одного закодированного блока в файле в битах (поскольку один блок лежит в целом количестве байт)
	size_t fullBlockSizeInBites = getIntegerBlockSizeInBytes(encodedBlockLength) * BITS_IN_BYTE;
	// Количество целых блоков в закодированном файле
	size_t fullBlockInEncodedFileNumber = significantBitsInEncodedFileNumber / fullBlockSizeInBites;
	// Текущий битсет для записи блока из файла и текущий декодированный (восстановленный) блок
	bitset<MAX_BLOCK_LENGTH> currentBitset, currentDecodedBlock;
	vector<byte> decodedInformation; // Декодированная информация, записанная побайтово
	
	for (size_t i = 0; i < fullBlockInEncodedFileNumber; i++) {
		currentBitset = readBlock(binaryFileToDecode, fullBlockSizeInBites);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, encodedBlockLength);
		decodedInformation = convertDecodedBlockAndAddToByteArray(currentDecodedBlock, blockSize, decodedInformation);
	}
	// Если есть остаток, отдельно работаем с ним
	if (residualBlockSize) {
		currentBitset = readBlock(binaryFileToDecode, residualEncodedBlockInFileSize);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, residualEncodedBlockSize);
		decodedInformation = convertDecodedBlockAndAddToByteArray(currentDecodedBlock, residualBlockSize, decodedInformation, TRUE);
	}

	fileForDecodedInformation.write((const char*)decodedInformation.data(), decodedInformation.size());
	
	return ERROR_SUCCESS;
}

/* Преобразует текущий блок (битсет) в массив байтов и добавляет его к текущему вектору, содержащему биты.
* Если размер блока не кратен восьми и невозможно полностью поместить его в байты, от него остается остаток, 
* который добавляется к следующему блоку */
vector<byte> convertDecodedBlockAndAddToByteArray(bitset<MAX_BLOCK_LENGTH> decodedBlock, unsigned blockSizeInBites, vector<byte> currentVector, BOOL end) {
	// Статическая переменная для оставшихся бит, если размер блока не кратен восьми
	static bitset<BITS_IN_BYTE> residualBits;
	// Количество оставшихся с прошлого вызова функции бит
	static size_t residualBitsCount = 0;

	size_t currentBitIndex = 0; // Номер текущего бита из блока
	size_t currentTempBitIndex = 0; // Номер текущего бита для заполнения байта (от 0 до 7)
	bitset<BITS_IN_BYTE> tempBitsetByte; // Временное хранилище для байта в битовом представлении

	

	// Пока есть остаточные биты с прошлого блока, заполняем текущий временный битсет на 8 бит (байт) ими
	for (currentTempBitIndex = 0; currentTempBitIndex < residualBitsCount; currentTempBitIndex++) {
		tempBitsetByte[currentTempBitIndex] = residualBits[currentTempBitIndex];
		residualBits[currentTempBitIndex] = 0;
	}
	residualBitsCount = 0;

	// Читаем текущий блок побитово
	for (currentBitIndex = 0; currentBitIndex < blockSizeInBites; currentBitIndex++) {
		// Побитово заполняем текущий байт
		tempBitsetByte[currentTempBitIndex++] = decodedBlock[currentBitIndex];

		if (currentTempBitIndex == BITS_IN_BYTE) {
			// Преобразуем текущий заполненный битсет в байт и добавляем в конец массива
			currentVector.push_back(static_cast<byte>(tempBitsetByte.to_ulong()));
			// Зануляем текущий временный индекс, чтобы начать заполнение нового битсета на восемь бит
			currentTempBitIndex = 0;
		}
	}

	// Если есть остаток, то добавляем вносим его в статическую переменную до следующего запуска программы
	if (currentTempBitIndex > 1) {
		residualBitsCount = currentTempBitIndex;
		residualBits = tempBitsetByte;
	}

	// Если работа с текущим сообщением окончена, очищаем остатки
	if (end) {
		residualBits.reset();
		residualBitsCount = 0;
	}

	return currentVector;
}

// Декодирует блок, исправляет одиночную ошибку, если таковая имеется. Возвращает декодированный блок без контрольных бит
bitset<MAX_BLOCK_LENGTH> decodeAndRestoreBlock(bitset<MAX_BLOCK_LENGTH> encodedBlock, unsigned encodedBlockSize) {
	bitset<MAX_BLOCK_LENGTH> decodedBlock; // Декодированный блок без контрольных бит
	string stringBlock; // Битсет (блок) в строковом представлении
	size_t currentBitIndex; // Индекс текущего бита в битсете
	size_t errorBitPosition = NO_ERROR_BITS; // Номер ошибочного бита, который требуется инвертировать. По умолчанию такого бита нет
	bool currentControlBit = 0; // Текущий контрольный бит, который мы проверяем

	// Проходимся по всем контрольным битам (степеням двойки)
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockSize + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		for (size_t i = currentBitIndex; i < encodedBlockSize; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber && k < encodedBlockSize; k++) {
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
		stringBlock = encodedBlock.to_string();
		cout << "Исправлен " << errorBitPosition << " бит в текущем блоке (вид до исправления | вид после исправления): "
			<< stringBlock.substr(stringBlock.size() - encodedBlockSize) << " | ";
		// Вычитаем единицу, потому что мы должны будем брать бит по индексу, а позиция у нас выступает в качестве номера
		errorBitPosition--;
		encodedBlock.flip(errorBitPosition); // Инвертируем ошибочный бит
		stringBlock = encodedBlock.to_string();
		cout << stringBlock.substr(stringBlock.size() - encodedBlockSize) << endl;
	}

	// Переносим в декодированный блок все информационные биты из закодированного
	for (size_t i = 0, currentBitIndex = 0; i < encodedBlockSize; i++) {
		// Пропускаем контрольные биты
		if (isIndexPowerOfTwo(i)) continue;
		decodedBlock[currentBitIndex++] = encodedBlock[i];
	}

	return decodedBlock;
}