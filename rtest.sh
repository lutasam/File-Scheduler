#!/bin/bash

mountpath="/home/vcm/File-Scheduler/mount/"

prefix="${mountpath}file"

content_size=1024  # 1KB

file_count=100

echo_files=()

total_time=0

for ((i=1; i<=file_count; i++)); do
    filename="${prefix}${i}.txt"
    content=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $content_size | head -n 1)
    echo $content > $filename
    echo_files+=("$filename")
done

for ((i=1; i<=file_count; i++)); do

    ls $mountpath > /dev/null

    start_time=$(date +%s%3N)
    
    filename="${prefix}${i}.txt"

    random_index=$(($RANDOM % ${#echo_files[@]}))
    random_file="${echo_files[$random_index]}"
    cat $random_file > /dev/null

    end_time=$(date +%s%3N)

    total_time=$((total_time + end_time - start_time))

done

echo "Total execution time: $total_time milliseconds"
echo "Bitrate: $(echo "scale=2; $((file_count * content_size)) / ($total_time / 1000)" | bc) B/s"

ls $mountpath > /dev/null
rm $mountpath*