#! /usr/bin/env python
import subprocess
from subprocess import Popen, call
import time
import os
import argparse

genome_files = '''
1a02N_1a3qA
1a04A_1dz3A
1a04A_1ibjA
1a04A_3chy
1a0cA_1a0dA
1a0cA_1bxbD
1a0cA_1ubpC
1a0fA_1hqoA
1a0fA_1ljrA
1a0fA_1pd21
1a0hA_5hpgA
1a0p_1a36A
1a0p_1ae9A
1a0tP_2mprA
1a12A_1jtdB
1a1z_1d2zA
1a28A_2prgA
1a28A_3erdA
1a2vA_1ksiB
1a36A_1a41
1a3aA_1a6jA
1a3c_1tc1A
1a3k_1c1lA
1a3k_1lcl
1a3qA_1gof
1a44_1qouB
1a49A_1dxeA
1a49A_1pkyC
1a49A_2tpsA
1a4iA_1b0aA
1a4iA_1ee9A
1a53_1dvjA
1a53_1hg3A
1a53_1nsj
1a53_1qo2A
1a62_1mjc
1a65A_1aozA
1a65A_1nif
1a6cA_1bmv1
1a6dA_1ass
1a6dA_1derA
1a6jA_1hynP
1a6l_2fdn
1a6m_1ash
1a6m_1cg5B
1a6m_1ewaA
1a6m_1flp
1a6m_1h97A
1a6m_1hlb
1a6m_1ithA
1a6m_2fal
1a6m_2gdm
1a6m_2hbg
1a6m_2vhbA
1a6m_3sdhA
1a6o_1buhA
1a75A_1b8cB
1a7s_1agjA
1a7s_2cgaB
1a7tA_1e5dA
1a7tA_1smlA
1a7w_1aoiA
1a7w_1aoiB
1a7w_1b67B
1a7w_1bh9B
1a7w_1tafA
1a7w_1tafB
1a8d_1wba
1a8h_1qqtA
1a8l_1hyuA
1a8o_1qrjB
1a8rA_1fb1D
1a9nA_1dceA
1aba_1h75A
1aba_1kte
1aba_1qfnA
1aba_3grx
1abrB_1dqgA
1abwA_1cqxA
1ac5_1auoA
1ac5_1cpy
1ad3A_1bpwA
1ad3A_1euhA
1adeA_1dj3A
1adjA_1atiA
1adjA_1httD
1ae9A_1a36A
1ae9A_1aihA
1aerA_1aerB
1ag4_1amm
1agdA_1bqsA
1agdA_1iakA
1agdB_1iakA
1agjA_1cqqA
1agqA_1vpfA
1ah1_1cid
1ah1_1f97A
1ah1_1kb5B
1ah1_1qfoA
1ah1_1tlk
'''
genome_files_list = list(map(lambda x: x.strip(), genome_files.split()))

devnull = open('/dev/null', 'w')
#worker_out = open('worker_output.txt', 'w')
alignments = []
#for first in range(len(genome_files_list)):
#    for second in range(len(genome_files_list)):
#for first in range(len(genome_files_list)):
#    for second in range(len(genome_files_list)):
for first in range(20):
    for second in range(10):
        alignments.append( ('data/' + genome_files_list[first], 'data/' + genome_files_list[second]) )

times = []
for x in range(5):
    print('starting computation... Here we go!')
    startIndex = 0
    intervalSize = 1
    clientCount = 5
    start = time.time()
    for (f, s) in alignments:
        call(['./matrix_test', f, s], stdout=devnull, stderr=devnull)
    
    finish = time.time()
    times.append(finish - start)

print('times = ')
print(times)
print('average = ' + str(sum(times) / len(times)))
