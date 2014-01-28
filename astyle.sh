#!/bin/bash

~/astyle \
    --indent=spaces=4 \
    --style=java \
    --add-brackets \
    --pad-oper \
    --align-pointer=type \
    --align-reference=name \
    --convert-tabs \
    --recursive \
    ./*.c \
    ./*.h
