#!/bin/bash

~/astyle \
    --indent=spaces=4 \
    --style=java \
    --add-brackets \
    --pad-oper \
    --pad-header \
    --align-pointer=type \
    --align-reference=name \
    --convert-tabs \
    --indent-switches \
    --recursive \
    ./*.c \
    ./*.h \
    --exclude=SVC/src/HAL.c \
    --exclude=SVC/src/memory.h
