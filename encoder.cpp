#include "main.hpp"

/*������� ��������� ���������� �� ���� � �������� �������, �������� ��� ������� �������� �������� (������
����� �������� ������ ����������), ���������� ������������ ������� ������������������ � ���� � ���������� ������ ����������.*/
DWORD encode(fstream& binaryFileToEncode, unsigned blockSize, string fileForEncodedInformationPath) {
	size_t bitsInInputFile = getFileLength(binaryFileToEncode); // ���������� ��� � ���������� �����
	size_t fullBlocksInInputFile = bitsInInputFile / blockSize; // ���������� ����� ������ � ���������� �����
	// ���������� �������� ��� � ��������� ����� (���� �� ���������� ������)
	unsigned char residualBitsNumber = bitsInInputFile % blockSize;
	bitset<MAX_BLOCK_LENGTH> currentBitset; // ������� ������ ��� ������ �����
	vector<byte> currentEncodedBlock; // ������� ������ ���� ��� ������ � ����
	ofstream outputFile; // ����, ���� ����� �������� �������������� ��������� � ��������� ����������

	// ��������� ����, ���� ����� �������� �������������� ���������, �� ������ � �������� ����
	outputFile.open(fileForEncodedInformationPath, ios_base::binary);
	if (!outputFile.is_open()) {
		cout << "���������� ������� ��������� ���� ��� ������ ��������������� ���������, � ��������� ��� ���������� ��� ��������\
			������ � ������ ����������" << endl;
		return ERROR_FILE_INVALID;
	}
	/* � ������ ����� ����� ����������� �����, ������������ ���������� �������� ��� � ��������� �����,
	����� ��� ������������� ��� ����� ���� ��������� ���������������� */
	outputFile << (byte) residualBitsNumber;

	// ���������� �������������� ���������� �������� � ����
	for (size_t i = 0; i < fullBlocksInInputFile; i++) {
		currentBitset = readBlock(binaryFileToEncode, blockSize);
		currentEncodedBlock = encodeBlock(currentBitset, blockSize);
		// ���������� ������ (������ ����) � �������� ����
		outputFile.write((const char*) currentEncodedBlock.data(), currentEncodedBlock.size());
	}
	// ���� ������� � ����� ���� �������� ����, �������� ������� � ��� �� ���������� ��� � ����
	if (residualBitsNumber != 0) {
		currentBitset = readBlock(binaryFileToEncode, residualBitsNumber);
		currentEncodedBlock = encodeBlock(currentBitset, residualBitsNumber);
		outputFile.write((const char*)currentEncodedBlock.data(), currentEncodedBlock.size());
	}
	outputFile.close();
	
	return ERROR_SUCCESS;
}

/* �������� "block" - ������ ���, ���������� � ���� ��������� �� ����� ������� ����
�������� "blockSizeInBytes" - ���������� �������� ��� � ������ ��������� (������ �����).

������� �������� ������� ���� ���������� ��������, �������� ����������� ���� � �������� �� ��������, � � ����������
����������� ������ � ������ ����, ������� ����� �������� �������� � ���� */
vector<byte> encodeBlock(bitset<MAX_BLOCK_LENGTH> block, unsigned blockSizeInBites) {
	size_t encodedBlockLength = getEncodedBlockLength(blockSizeInBites); // ������ ����� � ������������ � ��������������� ������
	unsigned currentBitIndex = 0; // ������ �������� ����  � ������� (���������� � ����)
	bitset<MAX_BLOCK_LENGTH> encodedBlock; // ���� � ������������ ������������ ������
	size_t encodedBytesNumber = getIntegerBlockSizeInBytes(encodedBlockLength); // ���������� ����� ���� � �������������� �����
	vector<byte> encodedBytes; // ������ ����, ������� ����� �������� � ����

	for (size_t i = 0; i < blockSizeInBites; i++) {
		// ���� ������� ����� �������� �������� ������, �� �������� ���, ������ ��� ��� ����������� ���.
		while (isIndexPowerOfTwo(currentBitIndex)) {
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
				if (k != currentBitIndex) encodedBlock[currentBitIndex] = encodedBlock[currentBitIndex] ^ encodedBlock[k];
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