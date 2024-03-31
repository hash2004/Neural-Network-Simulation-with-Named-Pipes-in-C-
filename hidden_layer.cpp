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
pthread_mutex_t mutexConvert;

//Declaring Thread data
struct ThreadData {
    string str;
    bool result;
};

// Declare a mutex
pthread_mutex_t mutex;

// The function to be executed by the thread
void* checkString(void* arg) {
    // Cast the argument to ThreadData
    ThreadData* data = static_cast<ThreadData*>(arg);

    // Lock the mutex
    pthread_mutex_lock(&mutex);

    // Check if the last character of the string is '0' or '1'
    if (!data->str.empty()) {
        char lastChar = data->str.back();
        data->result = (lastChar == '0' || lastChar == '1');
    } else {
        data->result = false;
    }

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);

    return nullptr;
}
// Struct to pass data to the conversion thread function
struct ConvertToStringData {
    char* charArray;
    string& outputString; // Reference to the string where the result will be stored
    int size;

    ConvertToStringData(char* array, string& output, int size)
        : charArray(array), outputString(output), size(size) {}
};

// Thread function for converting char array to string
void* convertCharArrayToString(void* arg) {
    ConvertToStringData* data = (ConvertToStringData*)arg;

    pthread_mutex_lock(&mutexConvert);

    for (int i = 0; i < data->size; ++i) {
        data->outputString += data->charArray[i];
    }

    pthread_mutex_unlock(&mutexConvert);

    return NULL;
}


int main()
{
    int weight = 2;
    int bias = 3;

    // Initialize the mutex
    pthread_mutex_init(&mutexConvert, NULL);
    const char* receive_input = "receive_input";
    mkfifo(receive_input, 0666);

    const char* output_to_HLayer1 = "output_to_HLayer1";
    mkfifo(output_to_HLayer1, 0666); //creating named pipes for IPC

    int receive = open(output_to_HLayer1, O_RDONLY);

    char receive_buffer[1024]; 

cout<<"here"<<endl;

    read(receive, receive_buffer, sizeof(receive_buffer)); 

cout<<"here"<<endl;


   // cout << "Received message: " << receive_buffer << endl;

    string result;
    ConvertToStringData threadData(receive_buffer, result, ARRAY_SIZE);

    // Create a thread for conversion
    pthread_t convertThread;
    if (pthread_create(&convertThread, NULL, &convertCharArrayToString, &threadData)) {
        cout << "Error creating conversion thread" << endl;
        return 1;
    }

    // Wait for the conversion thread to finish
    pthread_join(convertThread, NULL);

    // Output the result
    cout << "Converted string: " << result << endl;

    pthread_t thread;
    ThreadData data;
    data.str = result; // Example string

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Create a thread
    if (pthread_create(&thread, NULL, &checkString, &data)) {
        cout << "Error creating thread" << endl;
        return 1;
    }

    // Join the thread
    pthread_join(thread, NULL);

    // Check the result
    char signal_to_output_layer = data.result ? '1' : '0';

    // Named pipe for sending signal to the output layer
    const char* output_to_OLayer = "output_to_OLayer";
    mkfifo(output_to_OLayer, 0666);

    int hidden_to_output = open(output_to_OLayer, O_WRONLY);
    write(hidden_to_output, &signal_to_output_layer, sizeof(signal_to_output_layer));

    // Close the pipe after sending the signal
    close(hidden_to_output);

    //change weight and bias here

    const char* backprop_from_OLayer = "backprop_to_HLayer";
    mkfifo(backprop_from_OLayer, 0666); //creating new named pipe for backpropagation

    int hidden_from_output_backprop = open(backprop_from_OLayer, O_RDONLY);

    char error_signal;
    read(hidden_from_output_backprop, &error_signal, sizeof(error_signal));

    cout << "Error signal received from output layer: " << error_signal << endl;

    // Mock adjustment of weights and biases based on error signal
    // As it's a mock, we're not performing actual calculations here
    cout << "Adjusting weights and biases based on error signal" << endl;
    weight = weight+1;
    bias = bias*2;

    // Close the pipe after receiving the error signal
    close(hidden_from_output_backprop);
    return 0;
}