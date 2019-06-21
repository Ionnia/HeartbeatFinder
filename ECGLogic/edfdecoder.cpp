#include "edfdecoder.h"

// Вывод информации заголовка .edf файла
void EDFHeader::printInfo(){
    std::cout << "             Version: " << version << std::endl;
    std::cout << "           PatientID: " << patientID << std::endl;
    std::cout << "         RecordingID: " << recordingID << std::endl;
    std::cout << "           StartDate: " << startDate << std::endl;
    std::cout << "           StartTime: " << startTime << std::endl;
    std::cout << "  NumOfBytesInHeader: " << numberOfBytesInHeader << std::endl;
    std::cout << "    NumOfDataRecords: " << numberOfDataRecords << std::endl;
    std::cout << "DurationOfDataRecord: " << durationOfDataRecord << std::endl;
    std::cout << "     NumberOfSignals: " << numberOfSignals << std::endl;
    std::cout << "             Signals: " << std::endl;
    for(int i = 0; i < numberOfSignals; ++i){
        std::cout << "                  #" << i << ": " << labels[i] << std::endl;
    }
    std::cout << "     TransducerTypes: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << transducerTypes[i] << std::endl;
    }
    std::cout << "  PhysicalDimensions: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << physicalDimensions[i] << std::endl;
    }
    std::cout << "     PhysicalMinimum: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << physicalMinimum[i] << std::endl;
    }
    std::cout << "     PhysicalMaximum: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << physicalMaximum[i] << std::endl;
    }
    std::cout << "      DigitalMinimum: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << digitalMinimum[i] << std::endl;
    }
    std::cout << "      DigitalMaximum: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << digitalMaximum[i] << std::endl;
    }
    std::cout << "        Prefiltering: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << prefiltering[i] << std::endl;
    }
    std::cout << "NumOfSamplesInDataRecord: " << std::endl;
    for (int i = 0; i < numberOfSignals; ++i)
    {
        std::cout << "                  #" << i << ": " << numOfSamplesInDataRecord[i] << std::endl;
    }
}
// Вывод информации 
void EDFData::printInfo(){
    for(int i = 0; i < data.size(); ++i){
        std::cout << "Signal #" << i+1 << std::endl;
        for(int j = 0; j < data[i].size(); ++j){
            std::cout << data[i][j] << std::endl;
        }
    }
}

// Функция std::ifstream::read() не добавляет \0 в конце
void readNBytes(std::ifstream &file, char *buf, uint32_t numOfBytes){
    file.read(buf, numOfBytes);
    buf[numOfBytes] = '\0';
}

// Функция читает весь заголовок .edf файла, так что дальнейшее чтение будет
// происходить в области данных
void loadEDFHeader(std::ifstream &file, EDFHeader &header){
  // Буфер для чтения данных
    char c[1024];
    // Читаем 1 параметр: 8 байт
    readNBytes(file, c, 8);
    header.version.append(c);
    // Читаем 2 параметр: 80 байт
    readNBytes(file, c, 80);
    header.patientID.append(c);
    // Читаем 3 параметр: 80 байт
    readNBytes(file, c, 80);
    header.recordingID.append(c);
    // Читаем 4 параметр: 8 байт
    readNBytes(file, c, 8);
    header.startDate.append(c);
    // Читаем 5 параметр: 8 байт
    readNBytes(file, c, 8);
    header.startTime.append(c);
    // Читаем 6 параметр: 8 байт
    readNBytes(file, c, 8);
    header.numberOfBytesInHeader = std::stoi(c);
    // Пропускаем зарезервированные 44 байта
    readNBytes(file, c, 44);
    // Читаем 7 параметр: 8 байт
    readNBytes(file, c, 8);
    header.numberOfDataRecords = std::stoi(c);
    // Читаем 8 параметр: 8 байт
    readNBytes(file, c, 8);
    header.durationOfDataRecord = std::stoi(c);
    // Читаем 9 параметр: 4 байта
    readNBytes(file, c, 4);
    header.numberOfSignals = std::stoi(c);
    // Читаем 10 параметры: numberOfSiganls * 16 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 16);
        header.labels.push_back(std::string(c));
    }
    // Читаем 11 параметры: numberOfSingals * 80 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 80);
        header.transducerTypes.push_back(std::string(c));
    }
    // Читаем 12 параметры: numberOfSingals * 8 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.physicalDimensions.push_back(std::string(c));
    }
    // Читаем 13 параметры: numberOfSignals * 8 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.physicalMinimum.push_back(std::stod(std::string(c)));
    }
    // Читаем 14 параметры: numberOfSignals * 8 ,fqn
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.physicalMaximum.push_back(std::stod(std::string(c)));
    }
    // Читаем 15 параметры: numberOfSignals * 8 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.digitalMinimum.push_back(std::stoi(std::string(c)));
    }
    // Читаем 16 параметры: numberOfSignals * 8 байт
    for(int i = 0;  i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.digitalMaximum.push_back(std::stoi(std::string(c)));
    }
    // Читаем 17 параметры: numberOfSignals * 80 байт
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 80);
        header.prefiltering.push_back(std::string(c));
    }
    // Читаем 18 параметры:
    for(int i = 0; i < header.numberOfSignals; ++i){
        readNBytes(file, c, 8);
        header.numOfSamplesInDataRecord.push_back(std::stoi(std::string(c)));
    }
    // Пропускаем зарезервированные 32 байта
    readNBytes(file, c, 32 * header.numberOfSignals);
}

void loadEDFData(std::ifstream &file, EDFData &data, const EDFHeader &header){
    int32_t  value;
    // Создаём вектора
    for(int i = 0; i < header.numberOfSignals; ++i){
        data.data.push_back(std::vector<double>());
    }
    for(int i = 0; i < header.numberOfDataRecords; ++i){
        for(int j = 0; j < header.numberOfSignals; ++j){
            for(int k = 0; k < header.numOfSamplesInDataRecord[j]/2; ++k){
                file.read((char *)&value, sizeof(value));
                data.data[j].push_back((double)value);
            }
        }
    }
}

// Функция открывает .edf файл
int loadEDF(std::string pathToFile, EDF &edf){
    std::ifstream file(pathToFile);
    if(!file.is_open()){
        std::cerr << "Error::Can't open file!" << std::endl;
        return -1;
    }
    // Создаём EDF Header
    EDFHeader header;
    loadEDFHeader(file, header);
    // Создаём EDF Data
    EDFData data;
    loadEDFData(file, data, header);

    edf.header = header;
    edf.data = data;

    file.close();

    return 0;
}
