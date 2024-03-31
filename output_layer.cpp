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

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

// Structure to pass data to threads
struct ThreadData {
    int value;
};

// Function for neuron 1
void* neuron1(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);

    pthread_mutex_lock(&mutex1);
    if (data->value == 0) {
        cout << "Neuron 1 fired" << endl;
    }
    pthread_mutex_unlock(&mutex1);

    return nullptr;
}

// Function for neuron 2
void* neuron2(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);

    pthread_mutex_lock(&mutex2);
    if (data->value != 0) {
        cout << "Neuron 2 fired" << endl;
    }
    pthread_mutex_unlock(&mutex2);

    return nullptr;
}

int main() {
    // Named pipe for receiving signal from the hidden layer
    const char* input_from_HLayer = "output_to_OLayer";
    mkfifo(input_from_HLayer, 0666);

    // Open the pipe for reading
    int hidden_to_output = open(input_from_HLayer, O_RDONLY);

    char signal_from_hidden_layer;
    read(hidden_to_output, &signal_from_hidden_layer, sizeof(signal_from_hidden_layer));

    cout << "Signal received from hidden layer: " << signal_from_hidden_layer << endl;

    // Close the pipe after reading the signal
    close(hidden_to_output);

    // Logic to send instruction to input layer based on received signal
    const char* output_to_ILayer = "input_instruction";
    mkfifo(output_to_ILayer, 0666);

    int output_to_input = open(output_to_ILayer, O_WRONLY);

    char instruction_to_input_layer = '1';  
    write(output_to_input, &instruction_to_input_layer, sizeof(instruction_to_input_layer));

    // Close the pipe after sending the instruction
    close(output_to_input);

    pthread_mutex_init(&mutex1, NULL);  // Initialize the first mutex
    pthread_mutex_init(&mutex2, NULL);  // Initialize the second mutex

    int num = signal_from_hidden_layer - '0';  

    ThreadData data = {num};  // Example data

    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, neuron1, &data);
    pthread_create(&thread2, NULL, neuron2, &data);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

   

    char error_signal = 'E';  // Let's say 'E' represents an error signal

    // Named pipe for sending error signal back to the hidden layer
    const char* backprop_to_HLayer = "backprop_to_HLayer";
    mkfifo(backprop_to_HLayer, 0666);

    int output_to_hidden_backprop = open(backprop_to_HLayer, O_WRONLY);
    write(output_to_hidden_backprop, &error_signal, sizeof(error_signal));

    // Close the pipe after sending the error signal
    close(output_to_hidden_backprop);

    pthread_mutex_destroy(&mutex1);  // Clean up the first mutex
    pthread_mutex_destroy(&mutex2);  // Clean up the second mutex

    return 0;
}
