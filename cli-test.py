#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input', help='input file')
parser.add_argument('output', help='output file')
parser.add_argument('temp')
parser.add_argument('-f', '--first', type=int, help='first int optional argument', default=1)
parser.add_argument('-s', '--second',type=str, help='second string optional argument', default="default")
parser.add_argument('-t', type=int, help='third int optional argument')
parser.add_argument('--fourth', type=int)
args = parser.parse_args()
