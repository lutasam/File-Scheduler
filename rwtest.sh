#!/bin/bash

mountpath="/home/vcm/File-Scheduler/mount/"

prefix="${mountpath}file"

content_size=1024  # 1KB

file_count=50

echo_files=()

total_time=0

# decide the percent of read and write task, e.g. read_threshold=2 means 20% read task, 80% write task
# range should be [1, 9]
read_threshold=5

for ((i=1; i<=file_count; i++)); do

    ls $mountpath > /dev/null

    start_time=$(date +%s%3N)

    method=$(($RANDOM % 10))
    
    filename="${prefix}${i}.txt"

    if [ $method -lt $read_threshold ]; then
        if [[ ${#echo_files[@]} != 0 ]]; then
            random_index=$(($RANDOM % ${#echo_files[@]}))
            random_file="${echo_files[$random_index]}"
            cat $random_file > /dev/null
        fi

    else
        content=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $content_size | head -n 1)
        echo $content > $filename
        echo_files+=("$filename")
    fi

    end_time=$(date +%s%3N)
    total_time=$((total_time + end_time - start_time))

done

echo "Total execution time: $total_time ms"
echo "Bitrate: $(echo "scale=2; $((file_count * content_size)) / ($total_time / 1000)" | bc) B/s"

ls $mountpath > /dev/null
rm $mountpath*
python3 ../time_used.py