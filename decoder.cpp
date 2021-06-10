#include "main.hpp"

/* ������� ��������������� ����, �������������� ������� ��������, ��������� ��������� ��������� ������.

���������:
fstream& binaryFileToDecode - ����, ������� ��������� �����������, ������ �� ������ � �������� ����.
unsigned blockSize - ������ ��������������� ����� ����������� (��� ����� ����������� ���).
string fileWithDecodedInformationPath - ���� � �����, ���� ������ ���� �������� �������������� ���������.

���������� ��� ����������, ������ ERROR_SUCCESS ��� ������. */
DWORD decode(fstream& binaryFileToDecode, unsigned blockSize, string fileForDecodedInformationPath) {
	// ���������� �������� ��� � �������������� ����� (�������� ���� ����, ��� ��� � ������ ����� ����� ���������� � ����� �������)
	size_t significantBitsInEncodedFileNumber = getFileLength(binaryFileToDecode) - 1 * BITS_IN_BYTE;
	// �������� ����� ��� ������ ��������������� ���������
	ofstream fileForDecodedInformation(fileForDecodedInformationPath, ios_base::binary);
	unsigned char residualBlockSize; // ������ ����������������� ����������� ����� � ����� (���� �� ����)
	// ��������� ������ ���� �����, � ������� ����� ������ ����������� ����� � �����
	binaryFileToDecode.read((char*)&residualBlockSize, sizeof(residualBlockSize)); 
	// ������ ��������������� ������� � �����
	unsigned char residualEncodedBlockSize = getEncodedBlockLength(residualBlockSize);
	// ������ ��������������� ����������� ����� � ����� ����� ������, ������������ � ����
	unsigned char residualEncodedBlockInFileSize = getIntegerBlockSizeInBytes(residualEncodedBlockSize) * BITS_IN_BYTE;
	size_t encodedBlockLength = getEncodedBlockLength(blockSize); // ������ ��������������� ����� � �����
	// ������ ������ ��������������� ����� � ����� � ����� (��������� ���� ���� ����� � ����� ���������� ����)
	size_t fullBlockSizeInBites = getIntegerBlockSizeInBytes(encodedBlockLength) * BITS_IN_BYTE;
	// ���������� ����� ������ � �������������� �����
	size_t fullBlockInEncodedFileNumber = significantBitsInEncodedFileNumber / fullBlockSizeInBites;
	// ������� ������ ��� ������ ����� �� ����� � ������� �������������� (���������������) ����
	bitset<MAX_BLOCK_LENGTH> currentBitset, currentDecodedBlock;
	vector<byte> decodedInformation; // �������������� ����������, ���������� ���������
	
	for (size_t i = 0; i < fullBlockInEncodedFileNumber; i++) {
		currentBitset = readBlock(binaryFileToDecode, fullBlockSizeInBites);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, encodedBlockLength);
		decodedInformation = convertDecodedBlockAndAddToByteArray(currentDecodedBlock, blockSize, decodedInformation);
	}
	// ���� ���� �������, �������� �������� � ���
	if (residualBlockSize) {
		currentBitset = readBlock(binaryFileToDecode, residualEncodedBlockInFileSize);
		currentDecodedBlock = decodeAndRestoreBlock(currentBitset, residualEncodedBlockSize);
		decodedInformation = convertDecodedBlockAndAddToByteArray(currentDecodedBlock, residualBlockSize, decodedInformation, TRUE);
	}

	fileForDecodedInformation.write((const char*)decodedInformation.data(), decodedInformation.size());
	
	return ERROR_SUCCESS;
}

/* ����������� ������� ���� (������) � ������ ������ � ��������� ��� � �������� �������, ����������� ����.
* ���� ������ ����� �� ������ ������ � ���������� ��������� ��������� ��� � �����, �� ���� �������� �������, 
* ������� ����������� � ���������� ����� */
vector<byte> convertDecodedBlockAndAddToByteArray(bitset<MAX_BLOCK_LENGTH> decodedBlock, unsigned blockSizeInBites, vector<byte> currentVector, BOOL end) {
	// ����������� ���������� ��� ���������� ���, ���� ������ ����� �� ������ ������
	static bitset<BITS_IN_BYTE> residualBits;
	// ���������� ���������� � �������� ������ ������� ���
	static size_t residualBitsCount = 0;

	size_t currentBitIndex = 0; // ����� �������� ���� �� �����
	size_t currentTempBitIndex = 0; // ����� �������� ���� ��� ���������� ����� (�� 0 �� 7)
	bitset<BITS_IN_BYTE> tempBitsetByte; // ��������� ��������� ��� ����� � ������� �������������

	

	// ���� ���� ���������� ���� � �������� �����, ��������� ������� ��������� ������ �� 8 ��� (����) ���
	for (currentTempBitIndex = 0; currentTempBitIndex < residualBitsCount; currentTempBitIndex++) {
		tempBitsetByte[currentTempBitIndex] = residualBits[currentTempBitIndex];
		residualBits[currentTempBitIndex] = 0;
	}
	residualBitsCount = 0;

	// ������ ������� ���� ��������
	for (currentBitIndex = 0; currentBitIndex < blockSizeInBites; currentBitIndex++) {
		// �������� ��������� ������� ����
		tempBitsetByte[currentTempBitIndex++] = decodedBlock[currentBitIndex];

		if (currentTempBitIndex == BITS_IN_BYTE) {
			// ����������� ������� ����������� ������ � ���� � ��������� � ����� �������
			currentVector.push_back(static_cast<byte>(tempBitsetByte.to_ulong()));
			// �������� ������� ��������� ������, ����� ������ ���������� ������ ������� �� ������ ���
			currentTempBitIndex = 0;
		}
	}

	// ���� ���� �������, �� ��������� ������ ��� � ����������� ���������� �� ���������� ������� ���������
	if (currentTempBitIndex > 1) {
		residualBitsCount = currentTempBitIndex;
		residualBits = tempBitsetByte;
	}

	// ���� ������ � ������� ���������� ��������, ������� �������
	if (end) {
		residualBits.reset();
		residualBitsCount = 0;
	}

	return currentVector;
}

// ���������� ����, ���������� ��������� ������, ���� ������� �������. ���������� �������������� ���� ��� ����������� ���
bitset<MAX_BLOCK_LENGTH> decodeAndRestoreBlock(bitset<MAX_BLOCK_LENGTH> encodedBlock, unsigned encodedBlockSize) {
	bitset<MAX_BLOCK_LENGTH> decodedBlock; // �������������� ���� ��� ����������� ���
	string stringBlock; // ������ (����) � ��������� �������������
	size_t currentBitIndex; // ������ �������� ���� � �������
	size_t errorBitPosition = NO_ERROR_BITS; // ����� ���������� ����, ������� ��������� �������������. �� ��������� ������ ���� ���
	bool currentControlBit = 0; // ������� ����������� ���, ������� �� ���������

	// ���������� �� ���� ����������� ����� (�������� ������)
	for (unsigned currentBitNumber = 1; currentBitNumber < encodedBlockSize + 1; currentBitNumber *= 2) {
		currentBitIndex = currentBitNumber - 1;
		for (size_t i = currentBitIndex; i < encodedBlockSize; i += ((currentBitNumber) * 2)) {
			for (size_t k = i; k < i + currentBitNumber && k < encodedBlockSize; k++) {
				// ������� �������� � ������� �������� xor, ��������� ������� ���� �� ������ ������ �� ��������
				if (k != currentBitIndex) currentControlBit ^= encodedBlock[k];
			}
		}

		/* ���� �������� �������� ������������ ����, ����������� �� ��������� �������������� �����, �� ���������
		* � ��������� ��������� (���������) ������������ ����, �� ��������� ��� ������� � ����� ������� ���������� ���� */
		if (currentControlBit != encodedBlock[currentBitIndex]) {
			if (errorBitPosition == NO_ERROR_BITS) errorBitPosition = 0;
			errorBitPosition += currentBitNumber;
		}
		// �������� ����������� ���, ����� �� ��������� �������� ������ � ������� �����
		currentControlBit = 0;
	}

	// ���� ������� ��������� ���
	if (errorBitPosition != NO_ERROR_BITS) {
		stringBlock = encodedBlock.to_string();
		cout << "��������� " << errorBitPosition << " ��� � ������� ����� (��� �� ����������� | ��� ����� �����������): "
			<< stringBlock.substr(stringBlock.size() - encodedBlockSize) << " | ";
		// �������� �������, ������ ��� �� ������ ����� ����� ��� �� �������, � ������� � ��� ��������� � �������� ������
		errorBitPosition--;
		encodedBlock.flip(errorBitPosition); // ����������� ��������� ���
		stringBlock = encodedBlock.to_string();
		cout << stringBlock.substr(stringBlock.size() - encodedBlockSize) << endl;
	}

	// ��������� � �������������� ���� ��� �������������� ���� �� ���������������
	for (size_t i = 0, currentBitIndex = 0; i < encodedBlockSize; i++) {
		// ���������� ����������� ����
		if (isIndexPowerOfTwo(i)) continue;
		decodedBlock[currentBitIndex++] = encodedBlock[i];
	}

	return decodedBlock;
}