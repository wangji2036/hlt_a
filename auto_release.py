#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通用固件发布脚本
功能：自动查找最新的固件文件，计算校验码，生成带版本信息的文件并复制到 release 目录
特点：可迁移，适用于任何工程
"""

import sys
import os
import glob
import json
import re
import shutil
import hashlib
from datetime import datetime

# 设置 Windows 控制台编码为 UTF-8
if sys.platform == 'win32':
    try:
        import io
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
        sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')
    except Exception:
        pass


def calc_file_checksum(file_path):
    """
    计算文件的校验码（NuVolta ISP Programming Tool 算法）
    
    参数:
        file_path: 文件路径
    
    返回:
        (32位校验码, 16位校验码)
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"文件不存在: {file_path}")
    
    with open(file_path, 'rb') as f:
        data = f.read()
    
    length = len(data)
    sum_bytes = sum(data)
    
    # 算法: (len * 0x100) - (sum + len)
    checksum32 = (length * 0x100) - (sum_bytes + length)
    checksum32 = checksum32 & 0xFFFFFFFF
    checksum16 = checksum32 & 0xFFFF
    
    return checksum32, checksum16


def get_file_hash(file_path):
    """
    计算文件的 MD5 哈希值，用于判断文件是否已处理
    
    参数:
        file_path: 文件路径
    
    返回:
        MD5 哈希值（十六进制字符串）
    """
    hash_md5 = hashlib.md5()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def load_processed_files(record_file):
    """
    加载已处理文件记录
    
    参数:
        record_file: 记录文件路径
    
    返回:
        已处理文件的 MD5 哈希值集合
    """
    if not os.path.exists(record_file):
        return set()
    
    try:
        with open(record_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            return set(data.get('processed_files', []))
    except Exception:
        return set()


def save_processed_file(record_file, file_hash):
    """
    保存已处理文件记录
    
    参数:
        record_file: 记录文件路径
        file_hash: 文件的 MD5 哈希值
    """
    processed_files = load_processed_files(record_file)
    processed_files.add(file_hash)
    
    os.makedirs(os.path.dirname(record_file), exist_ok=True)
    with open(record_file, 'w', encoding='utf-8') as f:
        json.dump({'processed_files': list(processed_files)}, f, indent=2, ensure_ascii=False)


def find_latest_firmware(build_dir):
    """
    查找编译目录下最新的固件文件（.bin 文件优先）
    
    参数:
        build_dir: 构建输出目录（如 out、Release 或 Debug）
    
    返回:
        最新固件文件的路径，如果未找到则返回 None
    """
    if not os.path.exists(build_dir):
        return None
    
    # 优先查找 .bin 文件，然后是 .elf 和 .hex
    firmware_extensions = ['*.bin', '*.elf', '*.hex']
    firmware_files = []
    
    # 查找所有固件文件
    for ext in firmware_extensions:
        pattern = os.path.join(build_dir, ext)
        firmware_files.extend(glob.glob(pattern))
    
    if not firmware_files:
        return None
    
    # 排除已经包含校验码的文件（文件名中包含4位十六进制）
    filtered_files = []
    for f in firmware_files:
        filename = os.path.basename(f)
        # 检查文件名是否已经包含校验码（4位十六进制，通常在末尾）
        if not re.search(r'_[0-9A-Fa-f]{4}\.(bin|elf|hex)$', filename):
            filtered_files.append(f)
    
    if not filtered_files:
        # 如果所有文件都包含校验码，返回最新的
        filtered_files = firmware_files
    
    # 按修改时间排序，返回最新的文件
    filtered_files.sort(key=lambda x: os.path.getmtime(x), reverse=True)
    return filtered_files[0]


def get_project_name():
    """
    从 .project 文件或目录名获取项目名称
    """
    # 尝试从 .project 文件读取
    project_file = '.project'
    if os.path.exists(project_file):
        try:
            import xml.etree.ElementTree as ET
            tree = ET.parse(project_file)
            root = tree.getroot()
            name_elem = root.find('name')
            if name_elem is not None:
                return name_elem.text
        except Exception:
            pass
    
    # 如果无法读取项目文件，使用当前目录名
    return os.path.basename(os.path.abspath('.'))


def get_version_from_config():
    """
    尝试从常见位置提取版本号
    
    返回:
        版本号字符串，如果未找到则返回 None
    """
    config_files = [
        os.path.join('app', 'config.h'),
        'config.h',
        os.path.join('inc', 'config.h'),
    ]
    
    for config_file in config_files:
        if not os.path.exists(config_file):
            continue
        
        try:
            with open(config_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                # 查找 TX_FW_VER 定义
                match = re.search(r'#define\s+TX_FW_VER\s+(0x[0-9A-Fa-f]+|\d+)', content)
                if match:
                    version_hex = match.group(1)
                    if version_hex.startswith('0x') or version_hex.startswith('0X'):
                        version_num = int(version_hex, 16)
                        return f"{version_num:02X}"
                    else:
                        version_num = int(version_hex)
                        return f"{version_num:02X}"
        except Exception:
            continue
    
    return None


def generate_release_filename(original_path, version=None, checksum=None, date=None):
    """
    生成带版本信息的文件名
    
    参数:
        original_path: 原始文件路径
        version: 版本号（可选）
        checksum: 校验码（16位，可选）
        date: 日期（格式：YYYYMMDD，可选）
    
    返回:
        新文件名
    """
    file_dir = os.path.dirname(original_path)
    file_name = os.path.basename(original_path)
    file_base, file_ext = os.path.splitext(file_name)
    
    # 移除可能已存在的版本信息
    file_base = re.sub(r'_V[0-9A-Fa-f]+(_\d{8})?(_[0-9A-Fa-f]{4})?$', '', file_base)
    
    parts = [file_base]
    
    # 添加版本号
    if version:
        parts.append(f"V{version}")
    
    # 添加日期
    if date:
        parts.append(date)
    else:
        parts.append(datetime.now().strftime('%Y%m%d'))
    
    # 添加校验码
    if checksum:
        parts.append(f"{checksum:04X}")
    
    new_filename = "_".join(parts) + file_ext
    return os.path.join(file_dir, new_filename)


def main():
    """主函数"""
    # 自动找到项目根目录（包含 .project 文件的目录）
    script_dir = os.path.dirname(os.path.abspath(__file__ if '__file__' in globals() else sys.argv[0]))
    project_root = script_dir
    
    # 向上查找包含 .project 文件的目录
    current_dir = script_dir
    while current_dir != os.path.dirname(current_dir):
        if os.path.exists(os.path.join(current_dir, '.project')):
            project_root = current_dir
            break
        current_dir = os.path.dirname(current_dir)
    
    # 切换到项目根目录
    os.chdir(project_root)
    
    # 查找最新的固件文件
    build_dirs = ['out', 'Release', 'Debug']
    firmware_path = None
    
    for build_dir in build_dirs:
        if os.path.exists(build_dir):
            firmware_path = find_latest_firmware(build_dir)
            if firmware_path:
                break
    
    if not firmware_path:
        print("提示: 未找到固件文件", file=sys.stderr)
        print("请确保在 out、Release 或 Debug 目录中存在 .bin、.elf 或 .hex 文件", file=sys.stderr)
        sys.exit(0)  # 不报错，只是提示
    
    # 检查文件是否已处理
    record_file = os.path.join('release', '.processed_files.json')
    file_hash = get_file_hash(firmware_path)
    processed_files = load_processed_files(record_file)
    
    if file_hash in processed_files:
        print(f"提示: 文件已处理过，跳过: {os.path.basename(firmware_path)}")
        sys.exit(0)
    
    try:
        # 计算校验码
        checksum32, checksum16 = calc_file_checksum(firmware_path)
        
        # 获取版本号
        version = get_version_from_config()
        
        # 获取项目名称
        project_name = get_project_name()
        
        # 获取文件信息
        file_size = os.path.getsize(firmware_path)
        file_mtime = os.path.getmtime(firmware_path)
        file_mtime_str = datetime.fromtimestamp(file_mtime).strftime('%Y-%m-%d %H:%M:%S')
        build_date = datetime.fromtimestamp(file_mtime).strftime('%Y%m%d')
        
        # 生成新文件名
        new_filepath = generate_release_filename(firmware_path, version, checksum16, build_date)
        new_filename = os.path.basename(new_filepath)
        
        # 创建 release 目录
        release_dir = 'release'
        os.makedirs(release_dir, exist_ok=True)
        
        # 复制文件到 release 目录
        release_filepath = os.path.join(release_dir, new_filename)
        shutil.copy2(firmware_path, release_filepath)
        
        # 保存 release 信息
        release_info = {
            'project_name': project_name,
            'original_file': os.path.basename(firmware_path),
            'release_file': new_filename,
            'file_size': file_size,
            'file_size_kb': round(file_size / 1024, 2),
            'checksum_32bit': checksum32,
            'checksum_32bit_hex': f'0x{checksum32:08X}',
            'checksum_16bit': checksum16,
            'checksum_16bit_hex': f'0x{checksum16:04X}',
            'build_time': file_mtime_str,
            'build_date': build_date,
            'version': version if version else 'N/A',
            'file_hash': file_hash
        }
        
        # 保存 JSON 文件
        json_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.json")
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(release_info, f, indent=2, ensure_ascii=False)
        
        # 保存 TXT 文件
        txt_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.txt")
        with open(txt_file, 'w', encoding='utf-8') as f:
            f.write("=" * 60 + "\n")
            f.write("固件 Release 信息\n")
            f.write("=" * 60 + "\n\n")
            f.write(f"项目名称: {project_name}\n")
            f.write(f"原始文件: {release_info['original_file']}\n")
            f.write(f"发布文件: {new_filename}\n")
            f.write(f"文件大小: {file_size} 字节 ({release_info['file_size_kb']} KB)\n")
            f.write(f"构建时间: {file_mtime_str}\n")
            if version:
                f.write(f"版本号: V{version}\n")
            f.write(f"构建日期: {build_date}\n")
            f.write("\n")
            f.write("-" * 60 + "\n")
            f.write("校验码信息\n")
            f.write("-" * 60 + "\n")
            f.write(f"32位校验码: {release_info['checksum_32bit_hex']} ({checksum32})\n")
            f.write(f"16位校验码: {release_info['checksum_16bit_hex']} ({checksum16})\n")
            f.write(f"\n工具显示格式: {release_info['checksum_16bit_hex']}\n")
            f.write("=" * 60 + "\n")
        
        # 记录已处理文件
        save_processed_file(record_file, file_hash)
        
        # 打印摘要信息
        print("\n" + "=" * 60)
        print("固件发布处理完成")
        print("=" * 60)
        print(f"原始文件: {release_info['original_file']}")
        print(f"发布文件: {new_filename}")
        print(f"文件大小: {release_info['file_size_kb']} KB")
        if version:
            print(f"版本号: V{version}")
        print(f"构建日期: {build_date}")
        print(f"16位校验码: {release_info['checksum_16bit_hex']}")
        print(f"Release 文件已保存到: {release_dir}/")
        print("=" * 60)
        
    except Exception as e:
        print(f"错误: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
