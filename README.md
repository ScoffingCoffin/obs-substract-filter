# obs-substract-filter

(Temporary) To compile (WINDOWS 64bits)

gcc -shared -std=c99 -Wall -o obs-substract-filter.dll substract_filter.c obs-substract-filter.c -I "Path/To/libinclude" -L "Path/To/libobs" -lobs

add the lib to INSTALL_DIR\obs-studio\obs-plugins\64bit\
and the data to INSTALL_DIR\obs-studio\data\obs-plugins\obs-substract-filter\
