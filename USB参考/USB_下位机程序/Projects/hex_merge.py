#!/usr/bin/env python3
# yapf: disable
# -*- coding: utf8 -*-

'''
  本脚本将多个hex文件合并为1个hex文件
'''

import sys
import getopt
from intelhex import IntelHex

VERSION = '0.0.1'

# 在这里指定要合并的hex文件
hexfiles = []
output = ''

def main(args=None):
  if args is None:
    args = sys.argv[1:]
  try:
    opts, args = getopt.gnu_getopt(args, "hvf:g:o:", ["help", "version", "file1=", "file2=", "output="])

    for o,a in opts:
        if o in ('-h', '--help'):
            return 0
        elif o in ('-v', '--version'):
            print(VERSION)
            return 0
        elif o in ('-f', '--file1'):
            hexfiles.append(a)
        elif o in ('-g', '--file2'):
            hexfiles.append(a)
        elif o in ('-o', '--output'):
            output = a

    # print("hexfiles = ", hexfiles)
    # print("output = ", output)

    out_dict = {}
    for hf in hexfiles:
      ih = IntelHex()
      ih.loadfile(hf, format='hex')
      ih_dict = ih.todict()
      for addr in range(ih.minaddr(), ih.maxaddr()+1):
        data = ih_dict.get(addr)
        if data is None:
          continue
        if out_dict.get(addr) is None:
          out_dict[addr] = data
        else:
          print('Error: Address overlap!!!')
          sys.exit(1)

    ih_out = IntelHex(out_dict)
    ih_out.tofile(output, format="hex")

  except getopt.GetoptError:
      e = sys.exc_info()[1]     # current exception
      sys.stderr.write(str(e)+"\n")
      return 1

if __name__ == '__main__':
    sys.exit(main())
