#include "main.h"

/*Функция считывает переданный ей файл в двоичном формате, кодирует его методом Хемминга поблочно (размер
блока передает вторым аргументом), записывает получившуюся битовую последовательность в файл и возвращает
поток этого файла, открытый на чтение (для последующего декодирования).*/
fstream encode(fstream& binaryFileToEncode, unsigned blockSize) {
	size_t bitsInInputFile = getFileLength(binaryFileToEncode); // Количество бит в кодируемом файле
	// Количество значащих бит в последнем блоке (если не поделилось нацело)
	unsigned char residualBitsNumber = getEncodedBlockLength(bitsInInputFile % blockSize);
	bitset<MAX_BLOCK_LENGTH> currentBitset; // Текущий битсет для записи блока
	vector<byte> currentEncodedBlock; // Текущий массив байт для записи в файл

	string outputFilePath; // Путь к файлу, куда будет записано закодированное сообщение
	ofstream outputFile; // Файл, куда будет записано закодированное сообщение и служебная информацияz

	cout << "Введите путь к файлу, куда будет записана закодированная информация: ";
	cin >> outputFilePath;
	// Открываем файл, куда будет записано закодированное сообщение, на запись в двоичном виде
	outputFile.open(outputFilePath, ios_base::binary);
	if (!outputFile.is_open()) {
		cout << "Невозможно создать временный файл для записи закодированного сообщения, у программы нет разрешения для создания\
			файлов в данной директории";
		exit(ERROR_FILE_INVALID);
	}
	/* В первом байте файла сохраняется число, показывающее количество значащих бит в последнем блоке,
	чтобы при декодировании его можно было корректно интерпретировать */
	outputFile << (byte) residualBitsNumber;


	while (!binaryFileToEncode.eof()) {
		currentBitset = readBlock(binaryFileToEncode, blockSize);
		currentEncodedBlock = encodeBlock(currentBitset, blockSize);
		// Записываем вектор (массив байт) в выходной файл
		outputFile.write((const char*) currentEncodedBlock.data(), currentEncodedBlock.size());
	}
	outputFile.close();

	// Возвращаемый открытый на чтение в двоичном формате файл, куда записано закодированное сообщение
	fstream encodedFile(outputFilePath, ios_base::in || ios_base::binary);
	return encodedFile;
}

/* Аргумент "block" - массив бит, содержащий в себе считанный из файла битовый блок
Аргумент "blockSizeInBytes" - количество значащих бит в первом аргументе (размер блока).

Функция кодирует текущий блок алгоритмом Хемминга, добавляя контрольные биты и вычисляя их значение, а в дальнейшем
преобразует битсет в массив байт, который можно напрямую записать в файл */
vector<byte> encodeBlock(bitset<MAX_BLOCK_LENGTH> block, unsigned blockSizeInBites) {
	size_t encodedBlockLength = getEncodedBlockLength(blockSizeInBites); // Размер блока с контрольными и информационными битами
	unsigned currentBitIndex = 0; // Индекс текущего бита  в битсете (начинается с нуля)
	bitset<MAX_BLOCK_LENGTH> encodedBlock; // Блок с добавленными контрольными битами
	size_t encodedBytesNumber = ceil((double)encodedBlockLength / BITS_IN_BYTE); // Количество целых байт в закодированном блоке
	vector<byte> encodedBytes; // Массив байт, которые будут записаны в файл

	for (size_t i = 0; i < blockSizeInBites; i++) {
		/* Если текущий номер является степенью двойки, то зануляем его, потому что это контрольный бит.
		* Определяем, является ли число степенью двойки, через логарифм - если логарифм является целым числом, то это степень.
		* Прибавлять единицу надо, поскольку отсчет элементов идёт с нуля, а номера элементов - с единицы.
		while вместо if используется по той причине, что могут быть два таких бита подряд (под номером 1 и номером 2)*/
		while (log2(currentBitIndex + 1) == (double)(int)log2(currentBitIndex + 1)) {
			encodedBlock[currentBitIndex++] = 0;
		}

		// Переносим информационный бит из обычного блока в блок, куда добавлены контрольные биты
		encodedBlock[currentBitIndex++] = block[i];
	}

	if (currentBitIndex != encodedBlockLength) {
		cout << "Произошла непредвиденная ошибка в алгоритме, обратитесь к разработчику для выяснения деталей" << endl;
		exit(ERROR_ENCRYPTION_FAILED);
	}

	/* Идем по всем степеням двойки в битсете, однако не стоит забывать, что при обращении по индексу 
	требуется вычитать единицу, поскольку мы начали с числа 1, а индексы идут с нуля */
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockLength + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		/*Поскольку контрольный бит с номером N контролирует все последующие N бит через каждые N бит, начиная с позиции N,
		мы для каждого контрольного бита двигаемся на 2N за итерацию, а во внутреннем цикле уже пробегаемся по каждому N и
		считаем четность*/
		for (size_t i = currentBitIndex; i < encodedBlockLength; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber; k++) {
				// Считаем четность с помощью операции xor, поскольку нулевые биты не должны влиять на четность
				encodedBlock[currentBitIndex] = encodedBlock[currentBitIndex] ^ encodedBlock[k];
			}
		}
	}

	// Заполняем массив байтов, преобразуя в них текущий битсет (блок) с закодированной информаией
	for (size_t currentByteIndex = 0; currentByteIndex < encodedBytesNumber; currentByteIndex++) {
		size_t startBitIndex = currentByteIndex * BITS_IN_BYTE; // Стартовый индекс бита в блоке для каждого байта в блоке
		bitset<BITS_IN_BYTE> tempByteBitset; // Битсет на 8 бит для преобразования в байт

		// Копируем из блока по восемь бит во временный битсет 
		for (currentBitIndex = startBitIndex; currentBitIndex < startBitIndex + BITS_IN_BYTE; currentBitIndex ++) {
			tempByteBitset[currentBitIndex % 8] = encodedBlock[currentBitIndex];
		}
		
		// Преобразуем битсет из восьми бит в байт и добавляем его в массив
		encodedBytes.push_back(static_cast<byte>(tempByteBitset.to_ulong()));
	}

	return encodedBytes;
}