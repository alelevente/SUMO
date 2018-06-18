#!/bin/bash
for i in `seq 1 19`;
        do
                ./sumoD -c /home/levente/Egyetem/6_felev/onlab/BAH/BAH36000_smart.sumocfg --step-length 0.25
        done    
