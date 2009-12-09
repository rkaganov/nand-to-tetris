if [ $# -ne 1 ]
then
echo "Correct Syntax : ./JackCompiler.sh ( <directoryName> | <fileName> )"
exit
fi

if [ ! -e $1 ]
then
echo "$1 does not exist"
exit
fi

echo "Please wait..."
g++ JackCompiler.cpp -o JackCompiler.out

if [ -d $1 ]
then
dir=$1
set `ls $1 | grep .jack`
else
dir="."
fi

while [ $# -gt 0 ]
do
./JackCompiler.out $dir/$1
shift
done
read
