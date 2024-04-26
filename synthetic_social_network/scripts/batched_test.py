import argparse
# import paramiko
import subprocess
import multiprocessing
import time

def set_range(low, high, interval):
    rpss = []
    rps = low
    while rps < high:
        rpss.append(rps)
        rps += interval
    rpss.append(high)
    return rpss

def run_command(command, output_file):
    with open(output_file, "a") as f:
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        for line in process.stdout:
            decoded_line = line.decode().strip()
            if "0.950000" in decoded_line:
                f.write(decoded_line + "\n")
        process.stdout.close()
        process.stderr.close()

def run_cmd(gen_work_cmd, output_file):
    process1 = multiprocessing.Process(target=run_command, args=(gen_work_cmd,output_file))
    # process2 = multiprocessing.Process(target=run_command, args=(command2,))

    process1.start()
    # process2.start()

    process1.join()
    # process2.join()

def refresh():
    # refresh_cmd = "sudo docker service update --force synthetic_social_network-jaeger-1"
    refresh_cmd = "sudo docker restart synthetic_social_network-jaeger-1"
    subprocess.run(refresh_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

output_file = "../output/example.txt"
base_cmd = "../../wrk2/wrk -D exp -t 20 -c 100 -d 60 -L http://localhost:8010/api/service_0/rpc_0 -R "

rpss = set_range(600, 1000, 40)

for rps in rpss:
    with open(output_file, "a") as f:
        f.write("------new_config------rps:"+str(rps)+"\n")
    gen_work_cmd = base_cmd + str(rps)
    print(gen_work_cmd)
    for i in range(3):
        run_cmd(gen_work_cmd, output_file)
        time.sleep(5)
    refresh()