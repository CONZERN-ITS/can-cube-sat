#!/bin/bash
FILES_LIST=$(ls ./raws/*.mavlog)
cat ${FILES_LIST} > "./raws/all.mavlog-combined"