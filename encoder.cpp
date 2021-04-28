#include "main.h"

/*������� ��������� ���������� �� ���� � �������� �������, �������� ��� ������� �������� �������� (������
����� �������� ������ ����������), ���������� ������������ ������� ������������������ � ���� � ����������
����� ����� �����, �������� �� ������ (��� ������������ �������������).*/
fstream encode(fstream& binaryFileToEncode, unsigned blockSize) {
	size_t bitsInInputFile = getFileLength(binaryFileToEncode); // ���������� ��� � ���������� �����
	// ���������� �������� ��� � ��������� ����� (���� �� ���������� ������)
	unsigned char residualBitsNumber = getEncodedBlockLength(bitsInInputFile % blockSize);
	bitset<MAX_BLOCK_LENGTH> currentBitset; // ������� ������ ��� ������ �����
	vector<byte> currentEncodedBlock; // ������� ������ ���� ��� ������ � ����

	string outputFilePath; // ���� � �����, ���� ����� �������� �������������� ���������
	ofstream outputFile; // ����, ���� ����� �������� �������������� ��������� � ��������� ����������z

	cout << "������� ���� � �����, ���� ����� �������� �������������� ����������: ";
	cin >> outputFilePath;
	// ��������� ����, ���� ����� �������� �������������� ���������, �� ������ � �������� ����
	outputFile.open(outputFilePath, ios_base::binary);
	if (!outputFile.is_open()) {
		cout << "���������� ������� ��������� ���� ��� ������ ��������������� ���������, � ��������� ��� ���������� ��� ��������\
			������ � ������ ����������";
		exit(ERROR_FILE_INVALID);
	}
	/* � ������ ����� ����� ����������� �����, ������������ ���������� �������� ��� � ��������� �����,
	����� ��� ������������� ��� ����� ���� ��������� ���������������� */
	outputFile << (byte) residualBitsNumber;


	while (!binaryFileToEncode.eof()) {
		currentBitset = readBlock(binaryFileToEncode, blockSize);
		currentEncodedBlock = encodeBlock(currentBitset, blockSize);
		// ���������� ������ (������ ����) � �������� ����
		outputFile.write((const char*) currentEncodedBlock.data(), currentEncodedBlock.size());
	}
	outputFile.close();

	// ������������ �������� �� ������ � �������� ������� ����, ���� �������� �������������� ���������
	fstream encodedFile(outputFilePath, ios_base::in || ios_base::binary);
	return encodedFile;
}

/* �������� "block" - ������ ���, ���������� � ���� ��������� �� ����� ������� ����
�������� "blockSizeInBytes" - ���������� �������� ��� � ������ ��������� (������ �����).

������� �������� ������� ���� ���������� ��������, �������� ����������� ���� � �������� �� ��������, � � ����������
����������� ������ � ������ ����, ������� ����� �������� �������� � ���� */
vector<byte> encodeBlock(bitset<MAX_BLOCK_LENGTH> block, unsigned blockSizeInBites) {
	size_t encodedBlockLength = getEncodedBlockLength(blockSizeInBites); // ������ ����� � ������������ � ��������������� ������
	unsigned currentBitIndex = 0; // ������ �������� ����  � ������� (���������� � ����)
	bitset<MAX_BLOCK_LENGTH> encodedBlock; // ���� � ������������ ������������ ������
	size_t encodedBytesNumber = ceil((double)encodedBlockLength / BITS_IN_BYTE); // ���������� ����� ���� � �������������� �����
	vector<byte> encodedBytes; // ������ ����, ������� ����� �������� � ����

	for (size_t i = 0; i < blockSizeInBites; i++) {
		/* ���� ������� ����� �������� �������� ������, �� �������� ���, ������ ��� ��� ����������� ���.
		* ����������, �������� �� ����� �������� ������, ����� �������� - ���� �������� �������� ����� ������, �� ��� �������.
		* ���������� ������� ����, ��������� ������ ��������� ��� � ����, � ������ ��������� - � �������.
		while ������ if ������������ �� ��� �������, ��� ����� ���� ��� ����� ���� ������ (��� ������� 1 � ������� 2)*/
		while (log2(currentBitIndex + 1) == (double)(int)log2(currentBitIndex + 1)) {
			encodedBlock[currentBitIndex++] = 0;
		}

		// ��������� �������������� ��� �� �������� ����� � ����, ���� ��������� ����������� ����
		encodedBlock[currentBitIndex++] = block[i];
	}

	if (currentBitIndex != encodedBlockLength) {
		cout << "��������� �������������� ������ � ���������, ���������� � ������������ ��� ��������� �������" << endl;
		exit(ERROR_ENCRYPTION_FAILED);
	}

	/* ���� �� ���� �������� ������ � �������, ������ �� ����� ��������, ��� ��� ��������� �� ������� 
	��������� �������� �������, ��������� �� ������ � ����� 1, � ������� ���� � ���� */
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockLength + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		/*��������� ����������� ��� � ������� N ������������ ��� ����������� N ��� ����� ������ N ���, ������� � ������� N,
		�� ��� ������� ������������ ���� ��������� �� 2N �� ��������, � �� ���������� ����� ��� ����������� �� ������� N �
		������� ��������*/
		for (size_t i = currentBitIndex; i < encodedBlockLength; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber; k++) {
				// ������� �������� � ������� �������� xor, ��������� ������� ���� �� ������ ������ �� ��������
				encodedBlock[currentBitIndex] = encodedBlock[currentBitIndex] ^ encodedBlock[k];
			}
		}
	}

	// ��������� ������ ������, ���������� � ��� ������� ������ (����) � �������������� ����������
	for (size_t currentByteIndex = 0; currentByteIndex < encodedBytesNumber; currentByteIndex++) {
		size_t startBitIndex = currentByteIndex * BITS_IN_BYTE; // ��������� ������ ���� � ����� ��� ������� ����� � �����
		bitset<BITS_IN_BYTE> tempByteBitset; // ������ �� 8 ��� ��� �������������� � ����

		// �������� �� ����� �� ������ ��� �� ��������� ������ 
		for (currentBitIndex = startBitIndex; currentBitIndex < startBitIndex + BITS_IN_BYTE; currentBitIndex ++) {
			tempByteBitset[currentBitIndex % 8] = encodedBlock[currentBitIndex];
		}
		
		// ����������� ������ �� ������ ��� � ���� � ��������� ��� � ������
		encodedBytes.push_back(static_cast<byte>(tempByteBitset.to_ulong()));
	}

	return encodedBytes;
}