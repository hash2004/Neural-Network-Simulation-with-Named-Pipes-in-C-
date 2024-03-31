#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

const int ARRAY_SIZE = 256;
pthread_mutex_t mutexFile;
pthread_mutex_t mutexConvert;
pthread_mutex_t SerializeMutex;

// Struct to pass data to the file reading thread function
struct ThreadData {
    string filePath;
    char** dataArray;
    int startRow;
    int endRow;
};

// Struct to pass data to the array conversion thread function
struct ConvertData {
    char** charArray;
    int width;
    int height;
    int** intArray;
    int startRow;
    int endRow;
};

struct Serialize {
    int** dataInt;
    int width;
    int height;
    string textData;
    int startRow;
    int endRow;
};

// Thread function for reading data from file
void* readDataFromFile(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    const string& filePath = data->filePath;

    ifstream file(filePath);
    if (!file.is_open()) {
        cout << "Error opening file" << endl;
        return NULL;
    }

    string line;
    int row = 0;
    while (getline(file, line) && row < ARRAY_SIZE) {
        if (row >= data->startRow && row < data->endRow) {
            istringstream iss(line);
            char ch;
            int col = 0;
            while (iss >> ch && col < ARRAY_SIZE) {
                data->dataArray[row][col] = ch;
                col++;
            }
        }
        row++;
    }

    file.close();
    return NULL;
}

// Thread function for converting char array to int array
void* convertCharArrayToIntArray(void* arg) {
    ConvertData* convertData = (ConvertData*)arg;
    pthread_mutex_lock(&mutexConvert);

    for (int i = convertData->startRow; i < convertData->endRow; ++i) {
        for (int j = 0; j < convertData->width; ++j) {
            if (convertData->charArray[i][j] != '0' && convertData->charArray[i][j] != '1') {
                throw runtime_error("Invalid character found in the array. Only '0' or '1' is allowed.");
            }
            convertData->intArray[i][j] = convertData->charArray[i][j] - '0';
        }
    }    
    pthread_mutex_unlock(&mutexConvert);


    return NULL;
}

void* serializeIntArray(void* arg){
    Serialize* serialize = (Serialize*)arg;
    pthread_mutex_lock(&SerializeMutex);

    for (int i = serialize->startRow; i < serialize->endRow; ++i) {
        for (int j = 0; j < serialize->width; ++j) {
            serialize->textData += to_string(serialize->dataInt[i][j]) + (j == serialize->width - 1 ? "" : ",");
        }
        if (i < serialize->endRow - 1) {
            serialize->textData += ";"; // Separator for rows
        }
    }
    pthread_mutex_unlock(&SerializeMutex);


    return NULL;
}

int main() {
    //initializing the mutex
    pthread_mutex_init(&mutexFile, NULL);
    pthread_mutex_init(&mutexConvert, NULL);
    pthread_mutex_init(&SerializeMutex, NULL); 

    //creating arrays to store data
    char** dataArray = new char*[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        dataArray[i] = new char[ARRAY_SIZE];
    }

    int** intArray = new int*[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        intArray[i] = new int[ARRAY_SIZE];
    }

    // Process first half
    ThreadData fileData1 = {"input.txt", dataArray, 0, ARRAY_SIZE / 2};
    ConvertData convertData1 = {dataArray, ARRAY_SIZE, ARRAY_SIZE / 2, intArray, 0, ARRAY_SIZE / 2};
    Serialize serialize1 = {intArray, ARRAY_SIZE, ARRAY_SIZE / 2, "", 0, ARRAY_SIZE / 2};

    cout<<"here"<<endl;

    cpu_set_t cpuset1;
    CPU_ZERO(&cpuset1);
    CPU_SET(0, &cpuset1);


    pthread_t fileThread1, convertThread1, serializeThread1;
    pthread_create(&fileThread1, NULL, &readDataFromFile, &fileData1);
    pthread_join(fileThread1, NULL);
    pthread_create(&convertThread1, NULL, &convertCharArrayToIntArray, &convertData1);
    pthread_join(convertThread1, NULL);
    pthread_create(&serializeThread1, NULL, &serializeIntArray, &serialize1);
    pthread_join(serializeThread1, NULL);

    string output_to_hidden_layer = serialize1.textData;// + serialize2.textData;
   // cout << output_to_hidden_layer << endl;

    cout << "here"<<endl;
  //  sleep(5);

    const char* receive_input = "receive_input";
  //  mkfifo(receive_input, 0666);

    const char* output_to_HLayer1 = "output_to_HLayer1";
   // mkfifo(output_to_HLayer1, 0666);
    int I_to_H = open(output_to_HLayer1, O_WRONLY);
    write(I_to_H, output_to_hidden_layer.c_str(), output_to_hidden_layer.length() + 1);
cout<<"ok"<<endl;
    // Process second half

    cpu_set_t cpuset2;
    CPU_ZERO(&cpuset2);
    CPU_SET(1, &cpuset2);

     const char* input_from_OLayer = "input_instruction";
    mkfifo(input_from_OLayer, 0666);

    int output_to_input = open(input_from_OLayer, O_RDONLY);

    char instruction_from_output_layer;
    read(output_to_input, &instruction_from_output_layer, sizeof(instruction_from_output_layer));

    // Check the instruction and process the next batch if '1'
    if (instruction_from_output_layer == '1') {
        
    ThreadData fileData2 = {"input.txt", dataArray, ARRAY_SIZE / 2, ARRAY_SIZE};
    ConvertData convertData2 = {dataArray, ARRAY_SIZE, ARRAY_SIZE, intArray, ARRAY_SIZE / 2, ARRAY_SIZE};
    Serialize serialize2 = {intArray, ARRAY_SIZE, ARRAY_SIZE, "", ARRAY_SIZE / 2, ARRAY_SIZE};

    pthread_t fileThread2, convertThread2, serializeThread2;
    pthread_create(&fileThread2, NULL, &readDataFromFile, &fileData2);
    pthread_join(fileThread2, NULL);
    pthread_create(&convertThread2, NULL, &convertCharArrayToIntArray, &convertData2);
    pthread_join(convertThread2, NULL);
    pthread_create(&serializeThread2, NULL, &serializeIntArray, &serialize2);
    pthread_join(serializeThread2, NULL);
    }

    // Close the pipe after reading the instruction
    close(output_to_input);
  

    // Clean up
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        delete[] dataArray[i];
        delete[] intArray[i];
    }
    delete[] dataArray;
    delete[] intArray;
    close(I_to_H);

    return 0;
}
