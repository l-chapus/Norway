for f in `find data/*`
do

PNGFILE=`echo $f | sed s/data/images/ | sed s/\.dat/.png/`
echo ${f} ${PNGFILE}
cat <<EOSCRIPT | gnuplot - &
set term png
set output "${PNGFILE}"
set yrange [0:1.5]
plot "$f" binary array=8192 format='%double' with lines
EOSCRIPT

done
wait
