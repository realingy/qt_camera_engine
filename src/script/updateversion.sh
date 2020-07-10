#!/bin/bash    

cd $1
#cd ..
echo `pwd`

VERSION_MAJOR_NUMBER=$(cat version.mk | grep -E "VERSION_MAJOR_NUMBER" | cut -d = -f 2)
#echo $VERSION_MAJOR_NUMBER

VERSION_MEDIUM_NUMBER=$(cat version.mk | grep -E "VERSION_MEDIUM_NUMBER" | cut -d = -f 2)
#echo $VERSION_MEDIUM_NUMBER

VERSION_MINOR_NUMBER=$(cat version.mk | grep -E "VERSION_MINOR_NUMBER" | cut -d = -f 2)
#echo $VERSION_MINOR_NUMBER

VERSION_BUILD_NUMBER=$(cat buildnumber)
#echo $VERSION_BUILD_NUMBER

echo "#ifndef VERSION_H" > version.h
echo "#define VERSION_H" >> version.h
echo "" >> version.h
#echo "#include <QString>" >> version.h
echo "" >> version.h
echo "#define VERSION_MAJOR_NUMBER $VERSION_MAJOR_NUMBER" >> version.h
echo "#define VERSION_MEDIUM_NUMBER $VERSION_MEDIUM_NUMBER" >> version.h
echo "#define VERSION_MINOR_NUMBER $VERSION_MINOR_NUMBER" >> version.h
echo "#define VERSION_BUILD_NUMBER $VERSION_BUILD_NUMBER" >> version.h
echo "" >> version.h
#echo "QString getVersion()" >> version.h
#echo "{" >> version.h
echo "#define VERSION QString(\"V %1.%2.%3.%4\").arg(VERSION_MAJOR_NUMBER) \\" >> version.h
echo "         .arg(VERSION_MEDIUM_NUMBER) \\" >> version.h
echo "         .arg(VERSION_MINOR_NUMBER).arg(VERSION_BUILD_NUMBER)" >> version.h
echo "" >> version.h
#echo "    return version;" >> version.h
#echo "}" >> version.h
#echo "" >> version.h
echo "#endif //VERSION_H" >> version.h

#while read line
#do
#    echo "${line}"
#done < test.mk

#buildnumber=$(cat ../version.mk)

#if(($buildnumber < 500 )); then
#    let buildnumber=buildnumber+1
#    echo $buildnumber
#    echo $buildnumber > ../buildnumber
#    echo "#define VERSION_BUILD_NUMBER $buildnumber" > ../version.h
#fi

#while(($buildnumber < 500 ))
#do
#    let buildnumber=buildnumber+1
#    echo $buildnumber
#done
#echo $buildnumber

