#!/bin/bash

LG_LIB="include/lg/lib/liblg.a"

if [ ! -f "$LG_LIB" ]; then
  echo "liblg.a not found. Building lg library..."
  (cd include/lg && make) || { echo "Failed to build lg library"; exit 1; }
else
  echo "lg library found."
fi

echo "Building main project..."
make || { echo "Make failed"; exit 1; }
