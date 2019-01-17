#!/bin/bash

if [ $USER != sbmint ]; then
  echo "Not sbmint"
  su sbmint
fi

if [ $USER == sbmint ]; then
  echo "Currently sbmint"
fi
