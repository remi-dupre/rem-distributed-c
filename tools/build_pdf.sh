#!/bin/bash
to_build="algorithm slides report"

cd documentation
mkdir -p pdf

for file in $to_build
do
    xelatex $file
    bibtex $file || true
    xelatex $file
    xelatex $file

    cp $file.pdf pdf/
done
