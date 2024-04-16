import re

with open('/home/vcm/File-Scheduler/scheduler_exec/bbfs.log', 'r') as file:
    log_content = file.readlines()

total_exec_time = 0
for line in log_content:
    match = re.match(r'\[EXEC_TIME\]: (\d+)', line)
    if match:
        total_exec_time += int(match.group(1))

print("schedule cost time:", total_exec_time, "ms")