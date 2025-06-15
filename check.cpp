#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <bitset>
#include <array>
#include <utility>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

const double M_PI = 3.14159265358979323846;

// Матрицы квантования (стандартные JPEG)
const int QYold[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

const int QY[8][8] = {
    {99, 101, 92, 77, 62, 56, 55, 61},
    {103, 120, 113, 103, 80, 69, 60, 51},
    {100, 121, 104, 109, 87, 57, 58, 40},
    {112, 103, 81, 68, 51, 40, 26, 24},
    {98, 87, 64, 56, 29, 24, 19, 16},
    {95, 78, 55, 37, 22, 16, 14, 10},
    {92, 64, 35, 22, 17, 13, 12, 11},
    {72, 49, 24, 18, 14, 14, 12, 16}
};


const int QC[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};

// -------------------------------------------------------

const unordered_map<int, string> HA_Y_DC_TABLE = {
    {0, "00"},
    {1, "010"},
    {2, "011"},
    {3, "100"},
    {4, "101"},
    {5, "110"},
    {6, "1110"},
    {7, "11110"},
    {8, "111110"},
    {9, "1111110"},
    {10, "11111110"},
    {11, "111111110"}
};

const unordered_map<int, string> HA_C_DC_TABLE = {
    {0, "00"},
    {1, "01"},
    {2, "10"},
    {3, "110"},
    {4, "1110"},
    {5, "11110"},
    {6, "111110"},
    {7, "1111110"},
    {8, "11111110"},
    {9, "111111110"},
    {10, "1111111110"},
    {11, "11111111110"}
};

const map<pair<int, int>, string> HA_Y_AC_TABLE = {
    {{0, 0}, "1010"},
    {{0, 1}, "00"},
    {{0, 2}, "01"},
    {{0, 3}, "100"},
    {{0, 4}, "1011"},
    {{0, 5}, "11010"},
    {{0, 6}, "1111000"},
    {{0, 7}, "11111000"},
    {{0, 8}, "1111110110"},
    {{0, 9}, "1111111110000010"},
    {{0, 10}, "1111111110000011"},
    {{1, 1}, "1100"},
    {{1, 2}, "11011"},
    {{1, 3}, "1111001"},
    {{1, 4}, "111110110"},
    {{1, 5}, "11111110110"},
    {{1, 6}, "1111111110000100"},
    {{1, 7}, "1111111110000101"},
    {{1, 8}, "1111111110000110"},
    {{1, 9}, "1111111110000111"},
    {{1, 10}, "1111111110001000"},
    {{2, 1}, "11100"},
    {{2, 2}, "11111001"},
    {{ 2, 3}, "1111110111"},
    {{2, 4}, "111111110100"},
    {{2, 5}, "1111111110001001"},
    {{2, 6}, "1111111110001010"},
    {{2, 7}, "1111111110001011"},
    {{2, 8}, "1111111110001100"},
    {{2, 9}, "1111111110001101"},
    {{2, 10}, "1111111110001110"},
    {{3, 1}, "111010"},
    {{3, 2}, "111110111"},
    {{3, 3}, "111111110101"},
    {{3, 4}, "1111111110001111"},
    {{3, 5}, "1111111110010000"},
    {{3, 6}, "1111111110010001"},
    {{3, 7}, "1111111110010010"},
    {{3, 8}, "1111111110010011"},
    {{3, 9}, "1111111110010100"},
    {{3, 10}, "1111111110010101"},
    {{4, 1}, "111011"},
    {{4, 2}, "1111111000"},
    {{4, 3}, "1111111110010110"},
    {{4, 4}, "1111111110010111"},
    {{4, 5}, "1111111110011000"},
    {{4, 6}, "1111111110011001"},
    {{4, 7}, "1111111110011010"},
    {{4, 8}, "1111111110011011"},
    {{4, 9}, "1111111110011100"},
    {{4, 10}, "1111111110011101"},
    {{5, 1}, "1111010"},
    {{5, 2}, "11111110111"},
    {{5, 3}, "1111111110011110"},
    {{5, 4}, "1111111110011111"},
    {{5, 5}, "1111111110100000"},
    {{5, 6}, "1111111110100001"},
    {{5, 7}, "1111111110100010"},
    {{5, 8}, "1111111110100011"},
    {{5, 9}, "1111111110100100"},
    {{5, 10}, "1111111110100101"},
    {{6, 1}, "1111011"},
    {{6, 2}, "111111110110"},
    {{6, 3}, "1111111110100110"},
    {{6, 4}, "1111111110100111"},
    {{6, 5}, "1111111110101000"},
    {{6, 6}, "1111111110101001"},
    {{6, 7}, "1111111110101010"},
    {{6, 8}, "1111111110101011"},
    {{6, 9}, "1111111110101100"},
    {{6, 10}, "1111111110101101"},
    {{7, 1}, "11111010"},
    {{7, 2}, "111111110111"},
    {{7, 3}, "1111111110101110"},
    {{7, 4}, "1111111110101111"},
    {{7, 5}, "1111111110110000"},
    {{7, 6}, "1111111110110001"},
    {{7, 7}, "1111111110110010"},
    {{7, 8}, "1111111110110011"},
    {{7, 9}, "1111111110110100"},
    {{7, 10}, "1111111110110101"},
    {{8, 1}, "111111000"},
    {{8, 2}, "111111111000000"},
    {{8, 3}, "1111111110110110"},
    {{8, 4}, "1111111110110111"},
    {{8, 5}, "1111111110111000"},
    {{8, 6}, "1111111110111001"},
    {{8, 7}, "1111111110111010"},
    {{8, 8}, "1111111110111011"},
    {{8, 9}, "1111111110111100"},
    {{8, 10}, "1111111110111101"},
    {{9, 1}, "111111001"},
    {{9, 2}, "1111111110111110"},
    {{9, 3}, "1111111110111111"},
    {{ 9, 4}, "1111111111000000"},
    {{9, 5}, "1111111111000001"},
    {{9, 6}, "1111111111000010"},
    {{9, 7}, "1111111111000011"},
    {{9, 8}, "1111111111000100"},
    {{9, 9}, "1111111111000101"},
    {{9, 10}, "1111111111000110"},
    {{10, 1}, "111111010"},
    {{10, 2}, "1111111111000111"},
    {{10, 3}, "1111111111001000"},
    {{10, 4}, "1111111111001001"},
    {{10, 5}, "1111111111001010"},
    {{10, 6}, "1111111111001011"},
    {{10, 7}, "1111111111001100"},
    {{10, 8}, "1111111111001101"},
    {{10, 9}, "1111111111001110"},
    {{10, 10}, "1111111111001111"},
    {{11, 1}, "1111111001"},
    {{11, 2}, "1111111111010000"},
    {{11, 3}, "1111111111010001"},
    {{11, 4}, "1111111111010010"},
    {{11, 5}, "1111111111010011"},
    {{11, 6}, "1111111111010100"},
    {{11, 7}, "1111111111010101"},
    {{11, 8}, "1111111111010110"},
    {{11, 9}, "1111111111010111"},
    {{11, 10}, "1111111111011000"},
    {{12, 1}, "1111111010"},
    {{12, 2}, "1111111111011001"},
    {{12, 3}, "1111111111011010"},
    {{12, 4}, "1111111111011011"},
    {{12, 5}, "1111111111011100"},
    {{12, 6}, "1111111111011101"},
    {{12, 7}, "1111111111011110"},
    {{12, 8}, "1111111111011111"},
    {{12, 9}, "1111111111100000"},
    {{12, 10}, "1111111111100001"},
    {{13, 1}, "11111111000"},
    {{13, 2}, "1111111111100010"},
    {{13, 3}, "1111111111100011"},
    {{13, 4}, "1111111111100100"},
    {{13, 5}, "1111111111100101"},
    {{13, 6}, "1111111111100110"},
    {{13, 7}, "1111111111100111"},
    {{13, 8}, "1111111111101000"},
    {{13, 9}, "1111111111101001"},
    {{13, 10}, "1111111111101010"},
    {{14, 1}, "1111111111101011"},
    {{14, 2}, "1111111111101100"},
    {{14, 3}, "1111111111101101"},
    {{14, 4}, "1111111111101110"},
    {{14, 5}, "1111111111101111"},
    {{14, 6}, "1111111111110000"},
    {{14, 7}, "1111111111110001"},
    {{14, 8}, "1111111111110010"},
    {{14, 9}, "1111111111110011"},
    {{14, 10}, "1111111111110100"},
    {{15, 0}, "11111111001"},
    {{15, 1}, "1111111111110101"},
    {{15, 2}, "1111111111110110"},
    {{15, 3}, "1111111111110111"},
    {{15, 4}, "1111111111111000"},
    {{15, 5}, "1111111111111001"},
    {{15, 6}, "1111111111111010"},
    {{15, 7}, "1111111111111011"},
    {{15, 8}, "1111111111111100"},
    {{15, 9}, "1111111111111101"},
    {{15, 10}, "1111111111111110"}
};

const map<pair<int, int>, string> HA_C_AC_TABLE = {
    {{0, 0}, "00"},
    {{0, 1}, "01"},
    {{0, 2}, "100"},
    {{0, 3}, "1010"},
    {{0, 4}, "11000"},
    {{0, 5}, "11001"},
    {{0, 6}, "111000"},
    {{0, 7}, "1111000"},
    {{0, 8}, "111110100"},
    {{0, 9}, "1111110110"},
    {{0, 10}, "111111110100"},
    {{1, 1}, "1011"},
    {{1, 2}, "111001"},
    {{1, 3}, "11110110"},
    {{1, 4}, "111110101"},
    {{1, 5}, "11111110110"},
    {{1, 6}, "111111110101"},
    {{1, 7}, "1111111110001000"},
    {{1, 8}, "1111111110001001"},
    {{1, 9}, "1111111110001010"},
    {{1, 10}, "1111111110001011"},
    {{2, 1}, "11010"},
    {{2, 2}, "11110111"},
    {{2, 3}, "1111110111"},
    {{2, 4}, "111111110110"},
    {{2, 5}, "111111111000010"},
    {{2, 6}, "1111111110001100"},
    {{2, 7}, "1111111110001101"},
    {{2, 8}, "1111111110001110"},
    {{2, 9}, "1111111110001111"},
    {{2, 10}, "1111111110010000"},
    {{3, 1}, "11011"},
    {{3, 2}, "11111000"},
    {{3, 3}, "1111111000"},
    {{3, 4}, "1111111110010001"},
    {{3, 5}, "1111111110010010"},
    {{3, 6}, "1111111110010011"},
    {{3, 7}, "1111111110010100"},
    {{3, 8}, "1111111110010101"},
    {{3, 9}, "1111111110010110"},
    {{3, 10}, "1111111110010111"},
    {{4, 1}, "111010"},
    {{4, 2}, "111110110"},
    {{4, 3}, "1111111110011000"},
    {{4, 4}, "1111111110011001"},
    {{4, 5}, "1111111110011010"},
    {{4, 6}, "1111111110011011"},
    {{4, 7}, "1111111110011100"},
    {{4, 8}, "1111111110011101"},
    {{4, 9}, "1111111110011110"},
    {{4, 10}, "1111111110011111"},
    {{5, 1}, "111011"},
    {{5, 2}, "1111111001"},
    {{5, 3}, "1111111110100000"},
    {{5, 4}, "1111111110100001"},
    {{5, 5}, "1111111110100010"},
    {{5, 6}, "1111111110100011"},
    {{5, 7}, "1111111110100100"},
    {{5, 8}, "1111111110100101"},
    {{5, 9}, "1111111110100110"},
    {{5, 10}, "1111111110100111"},
    {{6, 1}, "1111001"},
    {{6, 2}, "11111110111"},
    {{6, 3}, "1111111110101000"},
    {{6, 4}, "1111111110101001"},
    {{6, 5}, "1111111110101010"},
    {{6, 6}, "1111111110101011"},
    {{6, 7}, "1111111110101100"},
    {{6, 8}, "1111111110101101"},
    {{6, 9}, "1111111110101110"},
    {{6, 10}, "1111111110101111"},
    {{7, 1}, "1111010"},
    {{7, 2}, "111111110111"},
    {{7, 3}, "1111111110110000"},
    {{7, 4}, "1111111110110001"},
    {{7, 5}, "1111111110110010"},
    {{7, 6}, "1111111110110011"},
    {{7, 7}, "1111111110110100"},
    {{7, 8}, "1111111110110101"},
    {{7, 9}, "1111111110110110"},
    {{7, 10}, "1111111110110111"},
    {{8, 1}, "11111001"},
    {{8, 2}, "1111111110111000"},
    {{8, 3}, "1111111110111001"},
    {{8, 4}, "1111111110111010"},
    {{8, 5}, "1111111110111011"},
    {{8, 6}, "1111111110111100"},
    {{8, 7}, "1111111110111101"},
    {{8, 8}, "1111111110111110"},
    {{8, 9}, "1111111110111111"},
    {{8, 10}, "1111111111000000"},
    {{9, 1}, "111110111"},
    {{9, 2}, "1111111111000001"},
    {{9, 3}, "1111111111000010"},
    {{9, 4}, "1111111111000011"},
    {{9, 5}, "1111111111000100"},
    {{9, 6}, "1111111111000101"},
    {{9, 7}, "1111111111000110"},
    {{9, 8}, "1111111111000111"},
    {{9, 9}, "1111111111001000"},
    {{9, 10}, "1111111111001001"},
    {{10, 1}, "111111000"},
    {{10, 2}, "1111111111001010"},
    {{10, 3}, "1111111111001011"},
    {{10, 4}, "1111111111001100"},
    {{10, 5}, "1111111111001101"},
    {{10, 6}, "1111111111001110"},
    {{10, 7}, "1111111111001111"},
    {{10, 8}, "1111111111010000"},
    {{10, 9}, "1111111111010001"},
    {{10, 10}, "1111111111010010"},
    {{11, 1}, "111111001"},
    {{11, 2}, "1111111111010011"},
    {{11, 3}, "1111111111010100"},
    {{11, 4}, "1111111111010101"},
    {{11, 5}, "1111111111010110"},
    {{11, 6}, "1111111111010111"},
    {{11, 7}, "1111111111011000"},
    {{11, 8}, "1111111111011001"},
    {{11, 9}, "1111111111011010"},
    {{11, 10}, "1111111111011011"},
    {{12, 1}, "111111010"},
    {{12, 2}, "1111111111011100"},
    {{12, 3}, "1111111111011101"},
    {{12, 4}, "1111111111011110"},
    {{12, 5}, "1111111111011111"},
    {{12, 6}, "1111111111100000"},
    {{12, 7}, "1111111111100001"},
    {{12, 8}, "1111111111100010"},
    {{12, 9}, "1111111111100011"},
    {{12, 10}, "1111111111100100"},
    {{13, 1}, "11111110010"},
    {{13, 2}, "1111111111100101"},
    {{13, 3}, "1111111111100110"},
    {{13, 4}, "1111111111100111"},
    {{13, 5}, "1111111111101000"},
    {{13, 6}, "1111111111101001"},
    {{13, 7}, "1111111111101010"},
    {{13, 8}, "1111111111101011"},
    {{13, 9}, "1111111111101100"},
    {{13, 10}, "1111111111101101"},
    {{14, 1}, "1111111010"},
    {{14, 2}, "1111111111101110"},
    {{14, 3}, "1111111111101111"},
    {{14, 4}, "1111111111110000"},
    {{14, 5}, "1111111111110001"},
    {{14, 6}, "1111111111110010"},
    {{14, 7}, "1111111111110011"},
    {{14, 8}, "1111111111110100"},
    {{14, 9}, "1111111111110101"},
    {{14, 10}, "1111111111110110"},
    {{15, 0}, "11111111000"},
    {{15, 1}, "1111111111110111"},
    {{15, 2}, "1111111111111000"},
    {{15, 3}, "1111111111111001"},
    {{15, 4}, "1111111111111010"},
    {{15, 5}, "1111111111111011"},
    {{15, 6}, "1111111111111100"},
    {{15, 7}, "1111111111111101"},
    {{15, 8}, "1111111111111110"},
    {{15, 9}, "1111111111111111"},
    {{15, 10}, "111111111000000"}
};

// -------------------------------------------------------

// Структура для записи битового потока
struct BitStream {
    vector<uint8_t> data; // Основное хранилище данных
    uint8_t buffer = 0; // Буфер для накопления битов перед записью в data
    int bitCount = 0;// Количество битов в буфере
};

// Запись одного бита в поток
void writeBit(BitStream* stream, bool bit) {
    stream->buffer |= (bit << (7 - stream->bitCount)); // Помещаем в буфер соответствущий бит начиная со старшего 
    stream->bitCount++;
    if (stream->bitCount == 8) { // Буфер заполнен? 8/8
        stream->data.push_back(stream->buffer); // Добавляем буфер в data
        stream->buffer = 0; // Сбрасываем буфер
        stream->bitCount = 0; // И счётчик
    }
}

// Запись строки битов в поток
void writeBits(BitStream* stream, const string& bits) {
    for (char bit : bits) {
        writeBit(stream, bit == '1');
    }
}

// Запись байта в поток
void writeByte(BitStream* stream, uint8_t byte) {
    if (stream->bitCount == 0) { // Если буфер пуст, можно записать байт напрямую
        stream->data.push_back(byte);
    }
    else { // Если нет, то по битам записываем
        for (int i = 7; i >= 0; i--) {
            writeBit(stream, (byte >> i) & 1);
        }
    }
}

// Получение данных из потока с завершением последнего байта
vector<uint8_t> getData(BitStream* stream) {
    if (stream->bitCount > 0) { // Если в буфере остались биты, добавляем их в data
        stream->data.push_back(stream->buffer);
    }
    return stream->data;
}

// Структура для чтения битового потока
struct BitStreamReader {
    const vector<uint8_t>& data; // Ссылка на данные для чтения
    size_t bytePos = 0; // Текущая позиция в data (индекс байта)
    int bitPos = 0; // Текущая позиция внутри байта (0-7)
    bool eof = false; // Флаг конца данных
};

// Инициализация ридера
BitStreamReader createBitStreamReader(const vector<uint8_t>& input) {
    return { input, 0, 0, false };
}

// Чтение одного бита
bool readBit(BitStreamReader* reader) {
    if (reader->eof || reader->bytePos >= reader->data.size()) { // Проверка на конец данных
        reader->eof = true;
        return false;
    }
    bool bit = (reader->data[reader->bytePos] >> (7 - reader->bitPos)) & 1; // Читаю бит из текущей позиции
    reader->bitPos++;
    if (reader->bitPos == 8) { // Перехожу к следующему байту.
        reader->bytePos++;
        reader->bitPos = 0;
    }
    return bit;
}

// Чтение байта
uint8_t readByte(BitStreamReader* reader) {
    if (reader->eof) return 0;

    if (reader->bitPos == 0) { // Если битовая позиция на границе байта, то можно читать напрямую
        if (reader->bytePos >= reader->data.size()) {
            reader->eof = true;
            return 0;
        }
        return reader->data[reader->bytePos++];
    }
    else { // Ну или читаем по битам
        uint8_t byte = 0;
        for (int i = 0; i < 8; i++) {
            byte |= readBit(reader) << (7 - i);
            if (reader->eof) break;
        }
        return byte;
    }
}

// Конец файла?
bool isEof(BitStreamReader* reader) {
    return reader->eof;
}

// Загрузка изображения из .raw формата (3 байта на пиксель в моем случае)
vector<vector<vector<float>>> loadRawImage(const char* filename, int width, int height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open RAW file " << filename << endl;
        exit(1);
    }

    // Вычисляем размер файла и проверяем соответствие
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    if (fileSize != width * height * 3) {
        cerr << "Error: RAW file size doesn't match dimensions" << endl;
        exit(1);
    }

    vector<vector<vector<float>>> image(height, vector<vector<float>>(width, vector<float>(3)));
    vector<uint8_t> buffer(width * height * 3);
    
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            image[y][x][0] = buffer[idx]; // R
            image[y][x][1] = buffer[idx+1]; // G
            image[y][x][2] = buffer[idx+2]; // B
        }
    }
    
    return image;
}

// Сохранение изображения в .raw формат
void saveRawImage(const char* filename, const vector<vector<vector<float>>>& image) {
    int height = image.size();
    if (height == 0) return;
    int width = image[0].size();

    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot create RAW file " << filename << endl;
        return;
    }

    vector<uint8_t> buffer(width * height * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            buffer[idx] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][0])));
            buffer[idx+1] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][1])));
            buffer[idx+2] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][2])));
        }
    }

    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

void writeCompressedToFile(const string& filename, const vector<uint8_t>& compressedData, int width, int height, int quality) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cerr << "Error: Cannot create file " << filename << endl;
        return;
    }

    // Записываем заголовок
    outFile.put(width >> 8); // Старший байт ширины [2]
    outFile.put(width & 0xFF); // Младший байт ширины
    outFile.put(height >> 8); // Старший байт высоты [2]
    outFile.put(height & 0xFF); // Младший байт высоты
    outFile.put(quality); // Качество (1 байт)

    // Записываем матрицы квантования
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            outFile.put(QY[y][x]);
        }
    }
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            outFile.put(QC[y][x]);
        }
    }

    // Записываем сжатые данные. Итого 133 байта: 5 + 64 + 64
    outFile.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());
    outFile.close();
}

vector<uint8_t> readCompressedFromFile(const string& filename, int& width, int& height, int& quality) {
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << "Error: Cannot open file " << filename << endl;
        return {};
    }

    // Читаем заголовок
    width = (inFile.get() << 8) | inFile.get();
    height = (inFile.get() << 8) | inFile.get();
    quality = inFile.get();

    // Пропускаем матрицы квантования
    inFile.seekg(128, ios::cur); // 64 + 64 байт

    // Читаем сжатые данные
    vector<uint8_t> compressedData((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    return compressedData;
}

// Кодирование DC коэффициента
string encodeHuffmanDC(int value, bool isLuminance) {
    // Определяем категорию DC коэффициента
    int category = 0;
    if (value != 0) {
        category = static_cast<int>(log2(abs(value))) + 1; // В какой степени двойки укладывается значение + 1
    }

    // Получаем код Хаффмана для категории
    const auto& table = isLuminance ? HA_Y_DC_TABLE : HA_C_DC_TABLE;
    auto it = table.find(category);
    if (it == table.end()) {
        cerr << "Error: DC category " << category << " not found in Huffman table" << endl;
        return "";
    }

    // Добавляем биты значения VLI
    string code = it->second;
    if (category > 0) {
        int vli = value > 0 ? value : (value + (1 << category) - 1);
        code += bitset<32>(vli).to_string().substr(32 - category);
    }

    return code;
}

// Функция для кодирования AC коэффициентов
string encodeAC(int run, int size, int value, bool isLuminance) {
    if (run == 0 && size == 0) { // EOB
        return isLuminance ? HA_Y_AC_TABLE.at({ 0, 0 }) : HA_C_AC_TABLE.at({ 0, 0 });
    } // Если все оставшиеся AC-коэффициенты в блоке нулевые, то возвращаем код Хаффмана для (0, 0), который означает конец блока

    if (run == 15 && size == 0) { // ZRL
        return isLuminance ? HA_Y_AC_TABLE.at({ 15, 0 }) : HA_C_AC_TABLE.at({ 15, 0 });
    }

    string code = isLuminance ? HA_Y_AC_TABLE.at({ run, size }) : HA_C_AC_TABLE.at({ run, size });
    if (size > 0) {
        if (value < 0) {
            value = (1 << size) - 1 + value;
        }
        bitset<16> bits(value);
        code += bits.to_string().substr(16 - size, size);
    }
    return code;
}

// RLE кодирование
vector<pair<int, int>> rleEncode(const vector<int>& block) {
    vector<pair<int, int>> rle;
    int zeroRun = 0;

    for (size_t i = 0; i < block.size(); i++) {
        if (block[i] == 0) {
            zeroRun++;
            if (zeroRun == 16) {
                rle.emplace_back(15, 0); // ZRL (Zero Run Length для хаффмана)
                zeroRun = 0;
            }
        }
        else {
            rle.emplace_back(zeroRun, block[i]);
            zeroRun = 0;
        }
    }

    // Добавляем EOB (End Of Block)
    rle.emplace_back(0, 0);
    return rle;
}

// RLE декодирование
vector<int> rleDecode(const vector<pair<int, int>>& rle) {
    vector<int> block;

    for (const auto& pair : rle) {
        int run = pair.first;
        int value = pair.second;

        if (run == 0 && value == 0) { // EOB
            // Заполняем оставшиеся нули
            while (block.size() < 64) {
                block.push_back(0);
            }
            break;
        }
        else if (run == 15 && value == 0) { // ZRL
            for (int i = 0; i < 16; i++) {
                block.push_back(0);
            }
        }
        else {
            for (int i = 0; i < run; i++) {
                block.push_back(0);
            }
            block.push_back(value);
        }
    }

    // Обрезаем до 64 коэффициентов на случай, если данных больше
    if (block.size() > 64) {
        block.resize(64);
    }

    return block;
}

// Разностное кодирование DC коэффициентов
vector<int> encodeDCDifferences(const vector<int>& dcValues) {
    if (dcValues.empty()) return {};

    vector<int> diffValues(dcValues.size());
    diffValues[0] = dcValues[0]; // Первый коэффициент остается без изменений

    for (size_t i = 1; i < dcValues.size(); i++) {
        diffValues[i] = dcValues[i] - dcValues[i - 1];
    }

    return diffValues;
}

// Декодирование разностей DC коэффициентов
vector<int> decodeDCDifferences(const vector<int>& diffValues) {
    if (diffValues.empty()) return {};

    vector<int> dcValues(diffValues.size());
    dcValues[0] = diffValues[0]; // Первый коэффициент остается без изменений

    for (size_t i = 1; i < diffValues.size(); i++) {
        dcValues[i] = dcValues[i - 1] + diffValues[i];
    }

    return dcValues;
}

// Кодирование AC коэффициента
string encodeHuffmanAC(int run, int size, int value, bool isLuminance) {
    // Специальные случаи: EOB и ZRL
    if (run == 0 && value == 0) { // EOB
        const auto& table = isLuminance ? HA_Y_AC_TABLE : HA_C_AC_TABLE;
        auto it = table.find({ 0, 0 });
        if (it == table.end()) {
            cout << "Error: EOB code not found in Huffman table" << endl;
            return "";
        }
        return it->second;
    }
    else if (run == 15 && value == 0) { // ZRL
        const auto& table = isLuminance ? HA_Y_AC_TABLE : HA_C_AC_TABLE;
        auto it = table.find({ 15, 0 });
        if (it == table.end()) {
            cout << "Error: ZRL code not found in Huffman table" << endl;
            return "";
        }
        return it->second;
    }

    // Определяем категорию значения
    int category = 0;
    if (value != 0) {
        category = static_cast<int>(log2(abs(value))) + 1;
    }

    // Получаем код Хаффмана для пары (run, category)
    const auto& table = isLuminance ? HA_Y_AC_TABLE : HA_C_AC_TABLE;
    auto it = table.find({ run, category });
    if (it == table.end()) {
        cerr << "Error: AC code for (run=" << run << ", size=" << category << ") not found in Huffman table" << endl;
        return "";
    }

    // Добавляем биты значения VLI
    string code = it->second;
    if (category > 0) {
        int vli = value > 0 ? value : (value + (1 << category) - 1);
        code += bitset<32>(vli).to_string().substr(32 - category);
    }

    return code;
}

// Функция для кодирования блока
void encodeBlock(const vector<int>& block, BitStream* bitStream, bool isLuminance) {
    // 1. Кодирование DC коэффициента
    int dcValue = block[0];
    int category = dcValue == 0 ? 0 : (int)log2(abs(dcValue)) + 1;

    const auto& dcTable = isLuminance ? HA_Y_DC_TABLE : HA_C_DC_TABLE;
    auto it = dcTable.find(category);
    if (it != dcTable.end()) {
        writeBits(bitStream, it->second);
        if (category > 0) {
            int vli = dcValue > 0 ? dcValue : dcValue + (1 << category) - 1;
            for (int i = category - 1; i >= 0; i--) {
                writeBit(bitStream, (vli >> i) & 1);
            }
        }
    }
    else {
        cerr << "Error: DC category not found\n";
    }

    // 2. Кодирование AC коэффициентов
    const auto& acTable = isLuminance ? HA_Y_AC_TABLE : HA_C_AC_TABLE;
    int zeroRun = 0;

    for (int i = 1; i < 64; i++) {
        if (block[i] == 0) {
            zeroRun++;
            if (zeroRun == 16) {
                // ZRL
                writeBits(bitStream, acTable.at({ 15, 0 }));
                zeroRun = 0;
            }
        }
        else {
            int value = block[i];
            int size = value == 0 ? 0 : (int)log2(abs(value)) + 1;
            writeBits(bitStream, acTable.at({ zeroRun, size }));

            if (size > 0) {
                int vli = value > 0 ? value : value + (1 << size) - 1;
                for (int j = size - 1; j >= 0; j--) {
                    writeBit(bitStream, (vli >> j) & 1);
                }
            }
            zeroRun = 0;
        }
    }

    // EOB
    if (zeroRun > 0) {
        writeBits(bitStream, acTable.at({ 0, 0 }));
    }
}

// Функция для декодирования блока Хаффмана
vector<int> decodeHuffmanBlock(BitStreamReader* reader, bool isLuminance) {
    vector<int> block(64, 0);
    const auto& dcTable = isLuminance ? HA_Y_DC_TABLE : HA_C_DC_TABLE;
    const auto& acTable = isLuminance ? HA_Y_AC_TABLE : HA_C_AC_TABLE;

    // 1. Декодирование DC коэффициента
    string code;
    while (code.size() < 16) { // Максимальная длина кода DC
        code += readBit(reader) ? '1' : '0';
        auto it = find_if(dcTable.begin(), dcTable.end(),
            [&code](const auto& p) { return p.second == code; });
        if (it != dcTable.end()) {
            int category = it->first;
            int value = 0;
            if (category > 0) {
                for (int i = 0; i < category; i++) {
                    value = (value << 1) | (readBit(reader) ? 1 : 0);
                }
                if ((value & (1 << (category - 1))) == 0) {
                    value -= (1 << category) - 1;
                }
            }
            block[0] = value;
            break;
        }
    }

    // 2. Декодирование AC коэффициентов
    int index = 1;
    while (index < 64) {
        code.clear();
        bool found = false;

        // Читаем код, пока не найдем совпадение
        while (code.size() < 32 && !found) {
            code += readBit(reader) ? '1' : '0';

            // Проверяем специальные коды EOB и ZRL
            if (code == acTable.at({ 0, 0 })) { // EOB
                while (index < 64) block[index++] = 0;
                return block;
            }
            if (code == acTable.at({ 15, 0 })) { // ZRL
                for (int i = 0; i < 16 && index < 64; i++) {
                    block[index++] = 0;
                }
                found = true;
                continue;
            }

            // Ищем обычные AC коды
            for (const auto& entry : acTable) {
                if (entry.second == code) {
                    int run = entry.first.first;
                    int size = entry.first.second;

                    // Заполняем нулями согласно run
                    for (int i = 0; i < run && index < 64; i++) {
                        block[index++] = 0;
                    }

                    // Читаем значение
                    if (size > 0 && index < 64) {
                        int value = 0;
                        for (int i = 0; i < size; i++) {
                            value = (value << 1) | (readBit(reader) ? 1 : 0);
                        }
                        if ((value & (1 << (size - 1))) == 0) {
                            value -= (1 << size) - 1;
                        }
                        block[index++] = value;
                    }
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            cerr << "Warning: AC code not found, filling with zeros\n";
            while (index < 64) block[index++] = 0;
            return block;
        }
    }
    return block;
}

// Функции для сохранения промежуточных изображений
void saveChannelAsImage(const char* filename, const vector<vector<float>>& channel, bool scaleToRGB = false) {
    int height = channel.size();
    if (height == 0) return;
    int width = channel[0].size();

    vector<uint8_t> data(height * width * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            float val = channel[y][x];

            val = max(0.0f, min(255.0f, val));
            data[idx] = static_cast<uint8_t>(val);
            data[idx + 1] = static_cast<uint8_t>(val);
            data[idx + 2] = static_cast<uint8_t>(val);
        }
    }

    stbi_write_jpg(filename, width, height, 3, data.data(), 90);
}

void saveYCbCrImage(const char* filename, const vector<vector<vector<float>>>& ycbcr) {
    int height = ycbcr.size();
    if (height == 0) return;
    int width = ycbcr[0].size();

    vector<uint8_t> data(height * width * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            data[idx] = static_cast<uint8_t>(max(0.0f, min(255.0f, ycbcr[y][x][0])));     // Y
            data[idx + 1] = static_cast<uint8_t>(max(0.0f, min(255.0f, ycbcr[y][x][1]))); // Cb
            data[idx + 2] = static_cast<uint8_t>(max(0.0f, min(255.0f, ycbcr[y][x][2]))); // Cr
        }
    }

    stbi_write_jpg(filename, width, height, 3, data.data(), 90);
}

vector<vector<float>> inverseZigzag(const vector<int>& scanned);

// Визуализация блоков DCT
void visualizeDCTBlocks(const vector<vector<vector<int>>>& blocks, const string& filename) {
    // Проверка на пустые блоки
    if (blocks.empty() || blocks[0].empty()) return;

    // Создаем изображение нужного размера
    int blockRows = blocks.size();
    int blockCols = blocks[0].size();
    vector<vector<float>> image(blockRows * 8, vector<float>(blockCols * 8));

    // Заполняем изображение данными из блоков
    for (int by = 0; by < blockRows; by++) {
        for (int bx = 0; bx < blockCols; bx++) {
            // Проверка на валидность блока
            if (by >= blocks.size() || bx >= blocks[by].size()) continue;

            auto block = inverseZigzag(blocks[by][bx]);
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (by * 8 + y < image.size() && bx * 8 + x < image[0].size()) {
                        image[by * 8 + y][bx * 8 + x] = block[y][x];
                    }
                }
            }
        }
    }

    // Сохраняем изображение
    saveChannelAsImage(filename.c_str(), image);
}

// Конвертация RGB в YCbCr
void RGBtoYCbCr(vector<vector<vector<float>>>& image) {
    int height = image.size();
    if (height == 0) return;
    int width = image[0].size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float R = image[y][x][0];
            float G = image[y][x][1];
            float B = image[y][x][2];

            float Y = 0.299f * R + 0.587f * G + 0.114f * B;
            float Cb = -0.1687f * R - 0.3313f * G + 0.5f * B + 128;
            float Cr = 0.5f * R - 0.4187f * G - 0.0813f * B + 128;

            image[y][x][0] = Y;
            image[y][x][1] = Cb;
            image[y][x][2] = Cr;
        }
    }

    // Сохраняем промежуточное изображение YCbCr
    saveYCbCrImage("1_ycbcr.jpg", image);

    // Сохраняем отдельные каналы
    vector<vector<float>> Y(height, vector<float>(width));
    vector<vector<float>> Cb(height, vector<float>(width));
    vector<vector<float>> Cr(height, vector<float>(width));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Y[y][x] = image[y][x][0];
            Cb[y][x] = image[y][x][1];
            Cr[y][x] = image[y][x][2];
        }
    }

    /*saveChannelAsImage("1_Y_channel.jpg", Y);
    saveChannelAsImage("1_Cb_channel.jpg", Cb);
    saveChannelAsImage("1_Cr_channel.jpg", Cr);*/
}

// Даунсэмплинг каналов Cb и Cr (2x2)
void downsample(vector<vector<float>>& channel, const string& channelName) {
    int height = channel.size();
    int width = height > 0 ? channel[0].size() : 0;
    int newHeight = (height + 1) / 2;
    int newWidth = (width + 1) / 2;

    vector<vector<float>> downsampled(newHeight, vector<float>(newWidth, 0));

    // Используем точное усреднение 4:2:0
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            float sum = 0;
            int count = 0;

            for (int dy = 0; dy < 2; dy++) { // Перебор блока 2 на 2 в исходнике
                for (int dx = 0; dx < 2; dx++) {
                    int ny = y * 2 + dy; // Исходный x
                    int nx = x * 2 + dx; // Исходный y
                    if (ny < height && nx < width) {
                        sum += channel[ny][nx]; 
                        count++;
                    }
                }
            }
            downsampled[y][x] = count > 0 ? sum / count : 0; // Усредняю суммой на количество
        }
    }
    channel = downsampled;

    // Сохраняем даунсэмплированный канал
    //saveChannelAsImage(("2_" + channelName + "_downsampled.jpg").c_str(), channel);
}

// Коэффициент для DCT
float alpha(int u) {
    return u == 0 ? sqrt(1.0 / 8.0) : sqrt(2.0 / 8.0);
} // Для нормализации преобразования

// Функция для вычисления DCT блока 8x8
void DCT(vector<vector<float>>& block) {
    vector<vector<float>> temp(8, vector<float>(8, 0));
    vector<vector<float>> result(8, vector<float>(8, 0));

    // Первое применение для строк
    for (int y = 0; y < 8; y++) {
        for (int u = 0; u < 8; u++) {
            float sum = 0.0f;
            for (int x = 0; x < 8; x++) {
                sum += block[y][x] * cos((2 * x + 1) * u * M_PI / 16.0);
            }
            temp[y][u] = alpha(u) * sum;
        }
    }

    // Повторное применение для столбцов
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            float sum = 0.0f;
            for (int y = 0; y < 8; y++) {
                sum += temp[y][u] * cos((2 * y + 1) * v * M_PI / 16.0);
            }
            result[v][u] = alpha(v) * sum;
        }
    }

    // Сохраняем результаты в блок
    block = result;
}

// Функция для вычисления обратного DCT блока 8x8
void IDCT(vector<vector<float>>& block) {
    vector<vector<float>> temp(8, vector<float>(8, 0));
    vector<vector<float>> result(8, vector<float>(8, 0));

    // Аналогично первое применение
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            float sum = 0.0f;
            for (int u = 0; u < 8; u++) {
                sum += alpha(u) * block[y][u] * cos((2 * x + 1) * u * M_PI / 16.0);
            }
            temp[y][x] = sum;
        }
    }

    // Повторное применение
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            float sum = 0.0f;
            for (int v = 0; v < 8; v++) {
                sum += alpha(v) * temp[v][x] * cos((2 * y + 1) * v * M_PI / 16.0);
            }
            result[y][x] = sum;
        }
    }

    // Сохраняем результат обратно в блок
    block = result;
}

// Тут квантование
void quantize(vector<vector<float>>& block, const int quantizationMatrix[8][8], int quality) {
    float qualityFactor = quality <= 0 ? 0.01f : quality > 100 ? 8.0f : quality < 50 ? 50.0f / quality : 2.0f - (2.0f * quality) / 100.0f;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            float q = quantizationMatrix[y][x] * qualityFactor;
            block[y][x] = round(block[y][x] / q);
        }
    }
}

// Тут деквантование
void dequantize(vector<vector<float>>& block, const int quantizationMatrix[8][8], int quality) {
    float qualityFactor = quality <= 0 ? 0.01f : quality > 100 ? 8.0f : quality < 50 ? 50.0f / quality : 2.0f - (2.0f * quality) / 100.0f;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            float q = quantizationMatrix[y][x] * qualityFactor;
            block[y][x] *= q;
        }
    }
}

// Функция для зигзаг-сканирования
vector<int> zigzagScan(const vector<vector<float>>& block) {
    vector<int> scanned(64);
    int index = 0;
    int row = 0, col = 0;
    bool goingUp = true; // Направление движения

    for (int i = 0; i < 64; i++) {
        scanned[index++] = static_cast<int>(block[row][col]);

        if (goingUp) {
            if (col == 7) { // Достигли правой границы
                row++;
                goingUp = false;
            }
            else if (row == 0) { // Достигли верхней границы
                col++;
                goingUp = false;
            }
            else { // Продолжаем движение вверх-вправо
                row--;
                col++;
            }
        }
        else { // Движение вниз-влево
            if (row == 7) { // Достигли нижней границы
                col++;
                goingUp = true;
            }
            else if (col == 0) { // Достигли левой границы
                row++;
                goingUp = true;
            }
            else { // Продолжаем движение вниз-влево
                row++;
                col--;
            }
        }
    }

    return scanned;
}

// Функция для обратного зигзаг-сканирования
vector<vector<float>> inverseZigzag(const vector<int>& scanned) {
    vector<vector<float>> block(8, vector<float>(8));
    int index = 0;
    int row = 0, col = 0;
    bool goingUp = true;

    for (int i = 0; i < 64; i++) {
        block[row][col] = static_cast<float>(scanned[index++]);

        if (goingUp) {
            if (col == 7) {
                row++;
                goingUp = false;
            }
            else if (row == 0) {
                col++;
                goingUp = false;
            }
            else {
                row--;
                col++;
            }
        }
        else {
            if (row == 7) {
                col++;
                goingUp = true;
            }
            else if (col == 0) {
                row++;
                goingUp = true;
            }
            else {
                row++;
                col--;
            }
        }
    }

    return block;
}

vector<vector<float>> reconstructChannel(const vector<vector<vector<int>>>& blocks, int height, int width, const int quantMatrix[8][8], int quality, bool isLuminance) {
    vector<vector<float>> channel(height, vector<float>(width));

    for (size_t by = 0; by < blocks.size(); by++) {
        for (size_t bx = 0; bx < blocks[by].size(); bx++) {
            // Обратное зигзаг-сканирование
            auto block = inverseZigzag(blocks[by][bx]);

            // Обратное квантование
            dequantize(block, quantMatrix, quality);

            // Обратное DCT
            IDCT(block);

            // Запись в изображение
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;
                    if (imgY < height && imgX < width) {
                        channel[imgY][imgX] = block[y][x] + 128.0f;
                    }
                }
            }
        }
    }
    return channel;
}

vector<vector<float>> upsample(const vector<vector<float>>& channel, int newHeight, int newWidth) {
    int oldHeight = channel.size();
    int oldWidth = oldHeight > 0 ? channel[0].size() : 0;
    vector<vector<float>> upsampled(newHeight, vector<float>(newWidth));

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            float fy = (y * (oldHeight - 1)) / (float)(newHeight - 1);
            float fx = (x * (oldWidth - 1)) / (float)(newWidth - 1);

            int y1 = floor(fy), x1 = floor(fx);
            int y2 = min(y1 + 1, oldHeight - 1);
            int x2 = min(x1 + 1, oldWidth - 1);

            float a = fy - y1, b = fx - x1;
            upsampled[y][x] =
                (1 - a) * (1 - b) * channel[y1][x1] +
                (1 - a) * b * channel[y1][x2] +
                a * (1 - b) * channel[y2][x1] +
                a * b * channel[y2][x2];
        }
    }
    return upsampled;
}

// Функция для загрузки изображения
vector<vector<vector<float>>> loadImage(const char* filename, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);
    if (!data) {
        cerr << "Failed to load image: " << filename << endl;
        exit(1);
    }

    vector<vector<vector<float>>> image(height, vector<vector<float>>(width, vector<float>(3)));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            image[y][x][0] = data[idx];
            image[y][x][1] = data[idx + 1];
            image[y][x][2] = data[idx + 2];
        }
    }

    stbi_image_free(data);
    return image;
}

// Функция для сохранения изображения
void saveImage(const char* filename, const vector<vector<vector<float>>>& image) {
    int height = image.size();
    if (height == 0) return;
    int width = image[0].size();

    vector<uint8_t> data(height * width * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            data[idx] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][0])));
            data[idx + 1] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][1])));
            data[idx + 2] = static_cast<uint8_t>(max(0.0f, min(255.0f, image[y][x][2])));
        }
    }

    stbi_write_jpg(filename, width, height, 3, data.data(), 90);
}

// Функция для конвертации YCbCr в RGB
void YCbCrtoRGB(vector<vector<vector<float>>>& image) {
    int height = image.size();
    if (height == 0) return;
    int width = image[0].size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float Y = image[y][x][0];
            float Cb = image[y][x][1] - 128;
            float Cr = image[y][x][2] - 128;

            float R = Y + 1.402f * Cr;
            float G = Y - 0.344136f * Cb - 0.714136f * Cr;
            float B = Y + 1.772f * Cb;

            image[y][x][0] = max(0.0f, min(255.0f, R));
            image[y][x][1] = max(0.0f, min(255.0f, G));
            image[y][x][2] = max(0.0f, min(255.0f, B));
        }
    }
}

// Основная функция сжатия с сохранением промежуточных результатов. Добавляю везде цифры, чтобы
// было проще ориентироватьс в коде...
vector<uint8_t> compressImage(const vector<vector<vector<float>>>& image, int quality, BitStream* bitStream) {
    // 1. Разделение на каналы
    int height = image.size();
    int width = image[0].size();

    vector<vector<float>> Y(height, vector<float>(width));
    vector<vector<float>> Cb(height, vector<float>(width));
    vector<vector<float>> Cr(height, vector<float>(width));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Y[y][x] = image[y][x][0];
            Cb[y][x] = image[y][x][1];
            Cr[y][x] = image[y][x][2];
        }
    }

    // 2. Даунсэмплинг Cb и Cr
    downsample(Cb, "Cb");
    downsample(Cr, "Cr");

    cout << "[Info] Downsampled" << endl;

    // 3. Разбиение на блоки 8x8 и DCT
    int blockRowsY = (height + 7) / 8;
    int blockColsY = (width + 7) / 8;
    int blockRowsC = (Cb.size() + 7) / 8;
    int blockColsC = (Cb[0].size() + 7) / 8;

    vector<vector<vector<int>>> blocksY(blockRowsY, vector<vector<int>>(blockColsY, vector<int>(64)));
    vector<vector<vector<int>>> blocksCb(blockRowsC, vector<vector<int>>(blockColsC, vector<int>(64)));
    vector<vector<vector<int>>> blocksCr(blockRowsC, vector<vector<int>>(blockColsC, vector<int>(64)));

    // Обработка Y канала
    vector<int> dcValuesY;
    for (int by = 0; by < blockRowsY; by++) {
        for (int bx = 0; bx < blockColsY; bx++) {
            vector<vector<float>> block(8, vector<float>(8));

            // Заполнение блока
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;

                    if (imgY < height && imgX < width) {
                        block[y][x] = Y[imgY][imgX] - 128; // Сдвиг уровня
                    }
                    else {
                        block[y][x] = 0; // Заполнение нулями
                    }
                }
            }

            // DCT и квантование
            DCT(block);
            quantize(block, QY, quality);

            // Зигзаг-сканирование
            vector<int> scanned = zigzagScan(block);
            for (int i = 0; i < 64; i++) {
                blocksY[by][bx][i] = scanned[i];
            }

            // Сохраняем DC коэффициент
            dcValuesY.push_back(scanned[0]);
        }
    }

    cout << "[Info] Y channel processing" << endl;

    // Обработка Cb канала
    vector<int> dcValuesCb;
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            vector<vector<float>> block(8, vector<float>(8));

            // Заполнение блока
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;

                    if (imgY < Cb.size() && imgX < Cb[0].size()) {
                        block[y][x] = Cb[imgY][imgX] - 128; // Сдвиг уровня
                    }
                    else {
                        block[y][x] = 0; // Заполнение нулями
                    }
                }
            }

            // DCT и квантование
            DCT(block);
            quantize(block, QC, quality);

            // Зигзаг-сканирование
            vector<int> scanned = zigzagScan(block);
            for (int i = 0; i < 64; i++) {
                blocksCb[by][bx][i] = scanned[i];
            }

            // Сохраняем DC коэффициент
            dcValuesCb.push_back(scanned[0]);
        }
    }

    cout << "[Info] Cb channel processing" << endl;

    // Обработка Cr канала
    vector<int> dcValuesCr;
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            vector<vector<float>> block(8, vector<float>(8));

            // Заполнение блока
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;

                    if (imgY < Cr.size() && imgX < Cr[0].size()) {
                        block[y][x] = Cr[imgY][imgX] - 128; // Сдвиг уровня
                    }
                    else {
                        block[y][x] = 0; // Заполнение нулями
                    }
                }
            }

            // DCT и квантование
            DCT(block);
            quantize(block, QC, quality);

            // Зигзаг-сканирование
            vector<int> scanned = zigzagScan(block);
            for (int i = 0; i < 64; i++) {
                blocksCr[by][bx][i] = scanned[i];
            }

            // Сохраняем DC коэффициент
            dcValuesCr.push_back(scanned[0]);
        }
    }

    cout << "[Info] Cr channel processing" << endl;

    // Визуализация блоков после DCT и квантования
    visualizeDCTBlocks(blocksY, "3_Y_DCT_quantized.jpg");
    visualizeDCTBlocks(blocksCb, "3_Cb_DCT_quantized.jpg");
    visualizeDCTBlocks(blocksCr, "3_Cr_DCT_quantized.jpg");

    // 4. Разностное кодирование DC коэффициентов
    vector<int> diffDCY = encodeDCDifferences(dcValuesY);
    vector<int> diffDCCb = encodeDCDifferences(dcValuesCb);
    vector<int> diffDCCr = encodeDCDifferences(dcValuesCr);

    cout << "[Info] DC differential coded" << endl;

    // 5. Кодирование Хаффмана и создание битового потока
    // Записываем метаданные
    writeByte(bitStream, width >> 8);
    writeByte(bitStream, width & 0xFF);
    writeByte(bitStream, height >> 8);
    writeByte(bitStream, height & 0xFF);
    writeByte(bitStream, quality);

    // Записываем DC коэффициенты
    for (int diff : diffDCY) {
        writeByte(bitStream, (diff >> 8) & 0xFF);
        writeByte(bitStream, diff & 0xFF);
    }
    for (int diff : diffDCCb) {
        writeByte(bitStream, (diff >> 8) & 0xFF);
        writeByte(bitStream, diff & 0xFF);
    }
    for (int diff : diffDCCr) {
        writeByte(bitStream, (diff >> 8) & 0xFF);
        writeByte(bitStream, diff & 0xFF);
    }

    // Кодируем блоки Y
    for (const auto& blockRow : blocksY) {
        for (const auto& block : blockRow) {
            encodeBlock(block, bitStream, true); // true for luminance
        }
    }

    // Кодируем блоки Cb
    for (const auto& blockRow : blocksCb) {
        for (const auto& block : blockRow) {
            encodeBlock(block, bitStream, false); // false for chrominance
        }
    }

    // Кодируем блоки Cr
    for (const auto& blockRow : blocksCr) {
        for (const auto& block : blockRow) {
            encodeBlock(block, bitStream, false); // false for chrominance
        }
    }

    cout << "[Info] Compress completed" << endl;

    return getData(bitStream);
}

// Основная функция декомпрессии с сохранением промежуточных результатов
vector<vector<vector<float>>> decompressImage(const vector<uint8_t>& compressedData) {
    BitStreamReader reader = createBitStreamReader(compressedData);

    // Читаем метаданные
    int width = (readByte(&reader) << 8) | readByte(&reader);
    int height = (readByte(&reader) << 8) | readByte(&reader);
    int quality = readByte(&reader);
    cout << "Info [dec]: Metadata read" << endl;

    // Определяем количество блоков
    int blockRowsY = (height + 7) / 8;
    int blockColsY = (width + 7) / 8;
    int blockRowsC = ((height / 2) + 7) / 8;
    int blockColsC = ((width / 2) + 7) / 8;

    // Чтение DC коэффициентов
    vector<int> diffDCY, diffDCCb, diffDCCr;

    for (int i = 0; i < blockRowsY * blockColsY; i++) {
        int16_t diff1 = (readByte(&reader) << 8) | readByte(&reader);
        diffDCY.push_back(static_cast<int>(diff1));
    }
    for (int i = 0; i < blockRowsC * blockColsC; i++) {
        int16_t diff2 = (readByte(&reader) << 8) | readByte(&reader);
        diffDCCb.push_back(static_cast<int>(diff2));
    }
    for (int i = 0; i < blockRowsC * blockColsC; i++) {
        int16_t diff3 = (readByte(&reader) << 8) | readByte(&reader);
        diffDCCr.push_back(static_cast<int>(diff3));
    }
    cout << "Info [dec]: DC read" << endl;

    // Декодирование DC коэффициентов
    vector<int> dcValuesY = decodeDCDifferences(diffDCY);
    vector<int> dcValuesCb = decodeDCDifferences(diffDCCb);
    vector<int> dcValuesCr = decodeDCDifferences(diffDCCr);
    cout << "Info [dec]: DC decoded" << endl;

    // Декодирование блоков Y
    vector<vector<vector<int>>> blocksY(blockRowsY, vector<vector<int>>(blockColsY, vector<int>(64)));
    int dcIndex = 0;
    for (int by = 0; by < blockRowsY; by++) {
        for (int bx = 0; bx < blockColsY; bx++) {
            vector<int> block = decodeHuffmanBlock(&reader, true);
            block[0] = dcValuesY[dcIndex++];
            blocksY[by][bx] = block;
        }
    }
    cout << "Info [dec]: Y blocks decoded" << endl;

    // Декодирование блоков Cb
    vector<vector<vector<int>>> blocksCb(blockRowsC, vector<vector<int>>(blockColsC, vector<int>(64)));
    dcIndex = 0;
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            vector<int> block = decodeHuffmanBlock(&reader, false);
            block[0] = dcValuesCb[dcIndex++];
            blocksCb[by][bx] = block;
        }
    }
    cout << "Info [dec]: Cb block decoded" << endl;

    // Декодирование блоков Cr
    vector<vector<vector<int>>> blocksCr(blockRowsC, vector<vector<int>>(blockColsC, vector<int>(64)));
    dcIndex = 0;
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            vector<int> block = decodeHuffmanBlock(&reader, false);
            block[0] = dcValuesCr[dcIndex++];
            blocksCr[by][bx] = block;
        }
    }
    cout << "Info [dec]: Cr blocks decoded" << endl;

    // Восстановление изображения Y
    vector<vector<float>> Y(height, vector<float>(width));
    for (int by = 0; by < blockRowsY; by++) {
        for (int bx = 0; bx < blockColsY; bx++) {
            auto block = inverseZigzag(blocksY[by][bx]);
            dequantize(block, QY, quality);
            IDCT(block);

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;
                    if (imgY < height && imgX < width) {
                        float val = block[y][x] + 128.0f;
                        Y[imgY][imgX] = max(0.0f, min(255.0f, val));
                    }
                }
            }
        }
    }
    cout << "Info [dec]: Y image recovery" << endl;
    saveChannelAsImage("4_Y_adter_IDCT.jpg", Y);

    // Восстановление изображения Cb
    vector<vector<float>> Cb((height + 1) / 2, vector<float>((width + 1) / 2));
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            auto block = inverseZigzag(blocksCb[by][bx]);
            dequantize(block, QC, quality);
            IDCT(block);

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;
                    if (imgY < Cb.size() && imgX < Cb[0].size()) {
                        float val = block[y][x] + 128.0f;
                        Cb[imgY][imgX] = max(0.0f, min(255.0f, val));
                    }
                }
            }
        }
    }
    cout << "Info [dec]: Cb image recovery" << endl;
    saveChannelAsImage("4_Cb_adter_IDCT.jpg", Cb);

    // Восстановление изображения Cr
    vector<vector<float>> Cr((height + 1) / 2, vector<float>((width + 1) / 2));
    for (int by = 0; by < blockRowsC; by++) {
        for (int bx = 0; bx < blockColsC; bx++) {
            auto block = inverseZigzag(blocksCr[by][bx]);
            dequantize(block, QC, quality);
            IDCT(block);

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int imgY = by * 8 + y;
                    int imgX = bx * 8 + x;
                    if (imgY < Cr.size() && imgX < Cr[0].size()) {
                        float val = block[y][x] + 128.0f;
                        Cr[imgY][imgX] = max(0.0f, min(255.0f, val));
                    }
                }
            }
        }
    }
    cout << "Info [dec]: Cr image recovery" << endl;
    saveChannelAsImage("4_Cr_adter_IDCT.jpg", Cr);

    // Апсемплинг Cb и Cr
    vector<vector<float>> CbUpsampled(height, vector<float>(width));
    vector<vector<float>> CrUpsampled(height, vector<float>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            CbUpsampled[y][x] = Cb[y / 2][x / 2];
            CrUpsampled[y][x] = Cr[y / 2][x / 2];
        }
    }
    cout << "Info [dec]: Cb and Cr upsampled" << endl;

    // Создание YCbCr изображения
    vector<vector<vector<float>>> ycbcr(height, vector<vector<float>>(width, vector<float>(3)));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            ycbcr[y][x][0] = Y[y][x];
            ycbcr[y][x][1] = CbUpsampled[y][x];
            ycbcr[y][x][2] = CrUpsampled[y][x];
        }
    }
    cout << "Info [dec]: YCbCr recreated" << endl;

    // Сохраняем промежуточное изображение YCbCr
    saveYCbCrImage("5_decompressed_ycbcr.jpg", ycbcr);
    cout << "Info [dec]: The image is saved" << endl;

    // Конвертация обратно в RGB
    YCbCrtoRGB(ycbcr);
    cout << "Info [dec]: Image was converted back to RGB" << endl;

    return ycbcr;
}

int main() {
    // 1. Загрузка RAW изображения
    int width = 512, height = 512;
    auto image = loadRawImage("lenna_bw_dithered.raw", width, height);

    // 2. Конвертация в YCbCr и сжатие
    RGBtoYCbCr(image);
    int quality = 40;
    BitStream bitStream;
    auto compressedData = compressImage(image, quality, &bitStream);

    // 3. Сохраняем сжатые данные + метаданные в compressed.bin
    writeCompressedToFile("6_compressed.bin", compressedData, width, height, quality);

    // 4. Читаем compressed.bin и разжимаем
    int loadedWidth, loadedHeight, loadedQuality;
    auto loadedData = readCompressedFromFile("6_compressed.bin", loadedWidth, loadedHeight, loadedQuality);
    auto decompressedImage = decompressImage(loadedData);

    // 5. Сохраняем разжатое изображение
    saveRawImage("6_decompressed.raw", decompressedImage);
    saveImage("6_decompressed.jpg", decompressedImage);

    // 6. Сравниваем lenna_rgb.raw и decompressed.raw (например, через diff)
    cout << "Compress completed. Compare here:\n";
    cout << "- Original: lenna_rgb.raw (" << (image.size() * 512 * 3) << " bytes)\n";
    cout << "- Compressed: compressed.bin (" << compressedData.size() << " bytes)\n"; // 133 = 5 + 128 байт заголовка
    cout << "- Decompressed: decompressed.raw (" << (decompressedImage.size() * 512 * 3) << " bytes)\n";
    cout << "- JPG for check: decompressed.jpg\n";

    return 0;
}
