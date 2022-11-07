#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os

Import("env")

print("update/get gcc plugin submodule")
os.system("git submodule update --init")

print("build submodule")
os.system("cd avr-flash-vtbl; make")

# add gcc plugin to c++ compile only
env.Append(CXXFLAGS=["-fplugin=./avr-flash-vtbl/avr-flash-vtbl.so"])
