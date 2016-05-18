#!/bin/bash
GL_LIBS="-lGL -lGLU -lglut -lGLEW -lpng -lSDL -lpthread -lX11 -lm"

input=${1}
exefile=`echo "${input}" | awk -F'\.' '{print $1}'`
type=`echo "${input}" | awk -F'\.' '{print $2}'`

echo "output will be: ${exefile}"
if [ ${type} == "c" -o ${type} == "C" ];then
  gcc -o ${exefile} ${input} ${GL_LIBS}
elif [ ${type} == "cpp" -o ${type} == "CPP" ];then
  g++ -o ${exefile} ${input} ${GL_LIBS}
else
  echo "file error, pls check..."
fi

