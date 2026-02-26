# -*- coding: utf8 -*-

'''
    Copyright (C) 2024 Westberry Technology Corp., Ltd

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
'''

'''
  本脚本用于处理固件
'''

'''
脚本参数说明：
processing_firmware.py <input_hex> <output_hex> <vid> <pid> <fw_version> <usage_page> <usage_id> <fw_addr>

参数说明：
1. input_hex: 输入的原始固件文件(hex格式)
   - 地址范围要求: [0x08000000, 0x08008000)
   - 示例: app.hex

2. output_hex: 处理后的输出固件文件(hex格式)
   - 示例: app_processed.hex

3. vid: USB 供应商ID(Vendor ID)
   - 格式: 16进制
   - 示例: 0x0483

4. pid: USB 产品ID(Product ID)
   - 格式: 16进制
   - 示例: 0x5750

5. fw_version: 固件版本号
   - 格式: 16进制, 遵循 0xJMNN 格式
   - J: 主版本号 [12:15] (0-F)
   - M: 次版本号 [08:11] (0-F)
   - N: 子版本号 [00:07] (00-FF)
   - 示例: 0x1203 表示版本 1.2.3

6. usage_page: USB HID Usage Page
   - 格式: 16进制
   - 示例: 0xFF91 (厂商自定义)

7. usage_id: USB HID Usage ID
   - 格式: 16进制
   - 示例: 0x32 (厂商自定义设备类型)

8. fw_addr: 固件起始地址
   - 格式: 16进制
   - 示例: 0x08002000

使用示例：
python processing_firmware.py app.hex app_processed.hex 0x0483 0x5750 0x1203
'''

import sys
import binascii
from intelhex import IntelHex

print('====== Processing Firmware Start ======')

if len(sys.argv) != 9:
  print('script.py in.hex out.hex')
  sys.exit(-1)

print('Input hex file: {}'.format(sys.argv[1]))

ih = IntelHex()
ih.loadfile(sys.argv[1], format='hex')

offset = 0x10
vid = int(sys.argv[3], 16)
pid = int(sys.argv[4], 16)
fw_version = int(sys.argv[5], 16)
usage_page = int(sys.argv[6], 16)
usage_id = int(sys.argv[7], 16)
fw_addr = int(sys.argv[8], 16)
# usage_page = 0xFF91
# usage_id = 0x32

print('This image address range is [0x{:08X},0x{:08X}]'.format(ih.minaddr(), ih.maxaddr()))
if (ih.minaddr() != fw_addr) or (ih.maxaddr() >= 0x08008000):
  print('ERROR: The {} content error!'.format(sys.argv[1]))
  sys.exit(-1)

#ih.dump()

fwbin = bytearray()
fwdict = ih.todict()
for addr in range(ih.minaddr(), ih.maxaddr()+1):
  data = fwdict.get(addr)
  if data == None:
    fwbin.append(0x00)
  else:
    fwbin.append(data)
# 如果固件SIZE不是4的倍数，则补齐
remainder = len(fwbin) % 4
if remainder != 0:
  for i in range(4-remainder):
    fwbin.append(0x00)
fwbin_size = len(fwbin)

fwbin[offset + 0x00: offset + 0x02] = int.to_bytes(vid, 2, byteorder='little', signed=False)
fwbin[offset + 0x02: offset + 0x04] = int.to_bytes(usage_page, 2, byteorder='little', signed=False)
fwbin[offset + 0x04: offset + 0x06] = int.to_bytes(pid, 2, byteorder='little', signed=False)
fwbin[offset + 0x06: offset + 0x08] = int.to_bytes(usage_id, 2, byteorder='little', signed=False)
fwbin[offset + 0x08: offset + 0x0C] = int.to_bytes(fw_version, 4, byteorder='little', signed=False)
fwbin[offset + 0x14: offset + 0x18] = int.to_bytes(fw_addr, 4, byteorder='little', signed=False)

# 将固件SIZE和CRC32存放到指定位置
fwbin_crc32 = binascii.crc32(fwbin)
print('fwbin size = {}'.format(fwbin_size))
print('fwbin crc32 = 0x{:08X}'.format(fwbin_crc32))
print("fw version = 0x{:04X}, vid = 0x{:04X}, pid = 0x{:04X}, usage_page = 0x{:04X}, usage_id = 0x{:02X}".format(fw_version, vid, pid, usage_page, usage_id))
fwbin[offset + 0x0C: offset + 0x10] = int.to_bytes(fwbin_size, 4, byteorder='little', signed=False)
fwbin[offset + 0x10: offset + 0x14] = int.to_bytes(fwbin_crc32, 4, byteorder='little', signed=False)

# 将处理过的固件代码输出为指定Hex文件
pih = IntelHex()
pih.frombytes(fwbin, offset=ih.minaddr())
pih.start_addr = ih.start_addr
print('Output hex file: {}'.format(sys.argv[2]))
pih.write_hex_file(sys.argv[2])


print('====== Processing Firmware End ======')
