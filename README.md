# Dragon-revival
A university project on distributed programing written in C using MPI.
## Description
Any idiot can slay a dragon. But reviving one... takes a professional. There are **two types of processes**: one of them generates a **contract** from time to time. The contract is contested among professionals of three specializations (head, tail and torso). It takes **professionals of all three specializations** to fulfil the contract. Additionally one of them must **do the paperwork** sitting at one of **b** desks in the Dragon Revival Guild. The they get access to one of **s** dragons and start the revival.
## Compilation and execution
* **mpicc** -o revive.out **dragon-revival.c**
* **mpirun** (--oversubscribe) -np /number of processes/ revive.out
## Technologies
* **MPI**
## Authors
* **Mikołaj Frankowski** - [RolandMcDoland](https://github.com/RolandMcDoland)
* **Krzysztof Pasiewicz** - [Nebizar](https://github.com/Nebizar) 
