#!/bin/bash

# Change this to your netid
netid=acs170004

# Root directory of your project
PROJDIR=aos_proj_1

# Directory where the config file is located on your local system
CONFIGLOCAL=$HOME/Computer_Science/Courses/UTD/Advanced_Operating_Systems/Homework/01_Project/aos_proj_1/Config_Files/config.txt

# Your executable binary 
PROG=main

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    #echo $i
    while [[ $n -lt $i ]]
    do
    	read line
    	p=$( echo $line | awk '{ print $1 }' )
        host=$( echo $line | awk '{ print $2 }' )
		echo $host
	
	urxvt -e sh -c "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host ./$PROJDIR/$PROG Config_Files/config.txt $p; exec bash" &

        n=$(( n + 1 ))
    done
)
