#!/bin/bash


# Change this to your netid
netid=acs170004


# Root directory of your project
PROJDIR=$HOME/Project_01

#
# Directory where the config file is located on your local system
CONFIGLOCAL=$HOME/Computer_Science/Courses/UTD/Advanced_Operating_Systems/Homework/01_Project/config.txt

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    echo $i
    while [[ $n -lt $i ]]
    do
    	read line
        host=$( echo $line | awk '{ print $2 }' )

        echo $host
        urxvt -e sh -c "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host killall -u $netid" &
        sleep 1

        n=$(( n + 1 ))
    done
   
)


echo "Cleanup complete"
