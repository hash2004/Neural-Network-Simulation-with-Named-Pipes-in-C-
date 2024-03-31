# Neural-Network-Simulation-with-Named-Pipes-in-C++
This project demonstrates the workings of a neural network using inter-process communication (IPC) with named pipes in C++. It includes a basic simulation of data flow through the layers of a neural network, illustrating how different components interact in a multi-threaded environment.

## Overview

The neural network simulation consists of three main components:

- **Input Layer**: Reads and processes input data, then sends it to the hidden layer.
- **Hidden Layer**: Receives data from the input layer, performs computations, and forwards the results to the output layer.
- **Output Layer**: Receives data from the hidden layer, processes it, and sends back the error signal to the hidden layer for weight adjustments.

## Files

- `input_layer.cpp`: Simulates the input layer of the neural network.
- `hidden_layer.cpp`: Represents the hidden layer where the main computations are done.
- `output_layer.cpp`: Acts as the output layer that finalizes the processing and sends feedback.
- `input.txt`: Contains the sample input data for the neural network.

## Prerequisites

- C++ Compiler (e.g., g++, clang++)
- Make (for building the project)
- POSIX-compliant operating system (for named pipes and threading support)

## Compilation

You can compile each component separately using the following bash command:

g++ -pthread -o [output_file_name] [source_file.cpp]
For example, for the input layer, the command would be: g++ -pthread -o input_layer input_layer.cpp

## Running the simulation
Open three different terminals and run the following commands: 
- ./output_layer
- ./hidden_layer
- ./input_layer


## Customization
You can modify the source code to experiment with different data processing logic, neuron functions, or IPC mechanisms to enhance the simulation and explore various aspects of neural network operations.
