# Testing the Effects of Concurrency on Dictionary Attacks

## Overview

This project benchmarks and analyzes the impact of concurrency on dictionary-based password cracking. By comparing sequential and multi-threaded approaches across multiple hash algorithms (MD5, SHA-1, SHA-256, SHA-512), the project highlights performance differences, speedup, and efficiency when using 1, 2, 4, 8, or 16 threads.

## Features

- **Sequential vs. Parallel Cracking**: Measures password cracking time using both single-threaded and multi-threaded approaches
- **Multi-Hash Support**: Tests MD5, SHA-1, SHA-256, and SHA-512 hash algorithms
- **Thread Scaling Analysis**: Compares using 1, 2, 4, 8, and 16 threads to evaluate speedup and efficiency
- **Randomized Testing**: Selects passwords randomly from a wordlist (I used rockyou.txt) for realistic benchmarking 
- **Performance Metrics**: Outputs execution times, speedup factors, and efficiency calculations for each thread count

## Technology Stack

- **C++17**: Used for cracking logic and multithreading 
- **Python**: Used to automate experiments, averages results, and generates performance plots  
- **OpenSSL**: Used for cryptographic functions for MD5 and SHA hashing 



