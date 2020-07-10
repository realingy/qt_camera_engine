#!/bin/bash    

#cd ..
cd $1
echo `pwd`

if [ ! -r buildnumber ]; then 
    rm buildnumber
    echo 0 > buildnumber
else
    cp buildnumber .buildnumberback
    buildnumber=$(cat .buildnumberback)
    let buildnumber=buildnumber+1
    echo $buildnumber
    echo $buildnumber > buildnumber
fi


#cd ..
#buildnumber=$(cat buildnumber)
#
#if(($buildnumber < 10000 )); then
#    let buildnumber=buildnumber+1
#    echo $buildnumber
#    echo $buildnumber > buildnumber
#fi
