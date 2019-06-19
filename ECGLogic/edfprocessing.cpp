#include "edfprocessing.h"

#include <vector>


// Функция для дифференцирования сигнала
std::vector<double> differentiateSignal(std::vector<double> values){
    std::vector<double> diffSignal;
    diffSignal.push_back(values[1] - values[0]);
    for(uint32_t i = 1; i < values.size(); ++i){
        diffSignal.push_back(values[i] - values[i-1]);
    }
    return diffSignal;
}

// Функция интегрирования окном
std::vector<double> integrateSignalWindow(std::vector<double> values, uint32_t windowSize){
    double integrator = 0;
    std::vector<double> integSignal;
    for(uint32_t i = 0; i < values.size(); ++i){
        integrator += values[i];
        if(int64_t(i) - windowSize >= 0){
            integrator -= values[i - windowSize];
        }
        integSignal.push_back(integrator);
    }
    return integSignal;
}

// Функция нахожодения среднего значения с окном
std::vector<double> averageWindow(std::vector<double> values, int32_t windowSize){
    double integrator = 0;
    std::vector<double> avgSignal;
    for(int i = 1; i < windowSize/2 - 1; ++i){
        integrator += values[i];
    }
    for(uint32_t i = 0; i < values.size(); ++i){
        if(int64_t(i) - windowSize/2 >= 0){
            integrator -= values[i - windowSize/2];
        }
        if(i + windowSize/2 < values.size()){
            integrator += values[i + windowSize/2];
        }
        avgSignal.push_back(integrator/windowSize);
    }
    return avgSignal;
}

// Функция нормализации сигнала
std::vector<double> normalizeSignal(std::vector<double> values){
    double maxValue = getMaxValue(values);
    double avgValue = getAvgValue(values);
    double normCoefficient = maxValue / avgValue;
    std::vector<double> normSignal;
    for(uint32_t i = 0; i < values.size(); ++i){
        if(values[i] < avgValue){
            normSignal.push_back(values[i] * normCoefficient);
        } else {
            normSignal.push_back(values[i] * normCoefficient);
        }
    }
    return normSignal;
}

double getMaxValue(std::vector<double> values){
    double maxValue = values[0];
    for(uint32_t i = 0; i < values.size(); ++i){
        if(maxValue < values[i]){
            maxValue = values[i];
        }
    }
    return maxValue;
}

double getAvgValue(std::vector<double> values){
    double integrator = 0;
    for(uint32_t i = 0; i < values.size(); ++i){
        integrator += values[i];
    }
    return integrator/values.size();
}

std::vector<double> cutWithArray(std::vector<double> values, std::vector<double> cutValues){
    std::vector<double> cuttedArray;
    for(uint32_t i = 0; i < values.size(); ++i){
        if(values[i] < cutValues[i]){
            cuttedArray.push_back(0);
        } else {
            cuttedArray.push_back(values[i]);
        }
    }
    return cuttedArray;
}

std::vector<double> cutWithAverage(std::vector<double> values, double averageValue){
    std::vector<double> cuttedArray;
    for(uint32_t i = 0; i < values.size(); ++i){
        if(values[i] < averageValue){
            cuttedArray.push_back(0);
        } else {
            cuttedArray.push_back(values[i]);
        }
    }
    return cuttedArray;
}

// Функция возвращает массив структур QRS комплекса (создерижит индекс начала и конца)
// Принимает на вход заранее обработанный массив
std::vector<QRS> findQRSComplexes(std::vector<double> values, uint32_t MIN_QRS_THRESHOLD){
    std::vector<QRS> QRSComplexes;
    uint32_t start_x = 0;
    uint32_t end_x = 0;
    bool isInsideQRS = false;
    for(uint32_t i = 0; i < values.size(); ++i){
        if(!isInsideQRS && values[i] != 0){
            start_x = i;
            isInsideQRS = true;
        }
        if(isInsideQRS && values[i] == 0){
            end_x = i;
            isInsideQRS = false;
            if(end_x - start_x > MIN_QRS_THRESHOLD){
                QRSComplexes.push_back({start_x, end_x});
            }
        }
    }
    return QRSComplexes;
}
