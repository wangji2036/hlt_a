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
import subprocess
from datetime import datetime

FIRMWARE_EXTENSION_PRIORITY = {
    '.bin': 3,
    '.elf': 2,
    '.hex': 1,
}

TEMPORARY_FIRMWARE_MARKERS = (
    'worktree',
    'tmp',
    'temp',
    'backup',
    'bak',
    'copy',
    '副本',
)

# 设置 Windows 控制台编码为 UTF-8
if sys.platform == 'win32':
    try:
        import io
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
        sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')
    except Exception:
        pass


def emit_result(key, value):
    """Emit stable result markers for wrapper scripts."""
    print(f"DD_RESULT|{key}|{value}")


def pass_fail(flag):
    return 'PASS' if flag else 'FAIL'


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


def get_code_change_description(project_root):
    """
    获取代码修改说明内容，用于与 Release 同步生成。
    优先使用项目根目录下的说明文件，否则尝试 git log，最后使用占位内容。
    
    参数:
        project_root: 项目根目录路径
    
    返回:
        字符串，可作为 代码修改说明 文件内容
    """
    # 优先从项目根已有文件读取
    candidates = [
        os.path.join(project_root, '代码修改说明.txt'),
        os.path.join(project_root, '代码修改说明.md'),
        os.path.join(project_root, 'git_commit_history.md'),
    ]
    for path in candidates:
        if os.path.isfile(path):
            try:
                with open(path, 'r', encoding='utf-8', errors='replace') as f:
                    content = f.read().strip()
                if content:
                    return content
            except Exception:
                pass
    # CHANGELOG_*.md 取第一个存在的
    for name in sorted(glob.glob(os.path.join(project_root, 'CHANGELOG_*.md'))):
        try:
            with open(name, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read().strip()
            if content:
                return content
        except Exception:
            pass
    # 尝试 git log
    try:
        r = subprocess.run(
            ['git', 'log', '-n', '15', '--pretty=format:%h %s'],
            cwd=project_root,
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='replace',
            timeout=5,
        )
        if r.returncode == 0 and (r.stdout or '').strip():
            lines = (r.stdout or '').strip().split('\n')
            head = "本次发布涉及提交（最近 15 条）:\n" + "=" * 50 + "\n"
            return head + "\n".join(lines)
    except Exception:
        pass
    # 占位
    return "代码修改说明（请在项目根目录添加 代码修改说明.txt 或使用 git 提交记录自动生成）\n生成时间: " + datetime.now().strftime('%Y-%m-%d %H:%M:%S')


def find_latest_firmware(build_dir):
    """
    查找编译目录下最新的固件文件（.bin 文件优先）
    
    参数:
        build_dir: 构建输出目录（如 out、Release 或 Debug）
    
    返回:
        最新固件文件的路径，如果未找到则返回 None
    """
    candidates = collect_firmware_candidates(build_dir)
    if not candidates:
        return None
    return candidates[0]['path']


def is_release_named_firmware(filename):
    """排除已带版本/日期/校验码的历史发布件。"""
    patterns = (
        r'_V[0-9A-Fa-f]+_\d{8}_[0-9A-Fa-f]{4}\.(bin|elf|hex)$',
        r'_[0-9A-Fa-f]{4}_\d{8}_\d{4}\.(bin|elf|hex)$',
    )
    return any(re.search(pattern, filename) is not None for pattern in patterns)


def is_temporary_firmware(filename):
    """排除 worktree、临时导出等非正式发布固件。"""
    lowered = filename.lower()
    return any(marker in lowered for marker in TEMPORARY_FIRMWARE_MARKERS)


def collect_firmware_candidates(build_dir):
    """
    收集单个构建目录下的有效候选固件。
    优先级：.bin > .elf > .hex；同类型内按修改时间排序。
    """
    if not os.path.isdir(build_dir):
        return []

    candidates = []
    for extension, priority in FIRMWARE_EXTENSION_PRIORITY.items():
        pattern = os.path.join(build_dir, f'*{extension}')
        for path in glob.glob(pattern):
            filename = os.path.basename(path)
            if is_release_named_firmware(filename):
                continue
            if is_temporary_firmware(filename):
                continue
            candidates.append({
                'path': path,
                'extension_priority': priority,
                'mtime': os.path.getmtime(path),
            })

    candidates.sort(
        key=lambda item: (item['extension_priority'], item['mtime']),
        reverse=True,
    )
    return candidates


def find_best_firmware(build_dirs):
    """
    从多个构建目录中选择最佳候选固件。
    规则：优先 .bin，其次 .elf/.hex；同类型内选择最新文件。
    """
    dir_priority = {name: len(build_dirs) - index for index, name in enumerate(build_dirs)}
    all_candidates = []

    for build_dir in build_dirs:
        for candidate in collect_firmware_candidates(build_dir):
            candidate['dir_priority'] = dir_priority.get(build_dir, 0)
            all_candidates.append(candidate)

    if not all_candidates:
        return None

    all_candidates.sort(
        key=lambda item: (
            item['extension_priority'],
            item['mtime'],
            item['dir_priority'],
        ),
        reverse=True,
    )
    return all_candidates[0]['path']


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


def generate_release_filename(original_path, project_name=None, checksum=None, date=None, time_hm=None):
    """
    生成发布文件名：项目名 + 校验码 + 日期 + 时间
    
    参数:
        original_path: 原始文件路径
        project_name: 项目名称（优先使用 .project 中的名称）
        checksum: 校验码（16位，可选）
        date: 日期（格式：YYYYMMDD，可选）
        time_hm: 时间（格式：HHMM，可选）
    
    返回:
        新文件名
    """
    file_dir = os.path.dirname(original_path)
    _, file_ext = os.path.splitext(os.path.basename(original_path))
    base_name = project_name or os.path.splitext(os.path.basename(original_path))[0]
    safe_project = re.sub(r'[<>:"/\\|?*\s]', '_', base_name).strip('_') or 'firmware'

    parts = [safe_project]

    if checksum is not None:
        parts.append(f"{checksum:04X}")

    parts.append(date or datetime.now().strftime('%Y%m%d'))
    parts.append(time_hm or datetime.now().strftime('%H%M'))
    
    new_filename = "_".join(parts) + file_ext
    return os.path.join(file_dir, new_filename)


def is_release_package_ready(
    release_filepath,
    json_file,
    txt_file,
    changelog_file,
    expected_hash,
    expected_release_name,
    expected_checksum16,
    expected_checksum32,
):
    """判断已存在的 Release 包是否与当前源文件完全一致。"""
    required_files = (release_filepath, json_file, txt_file, changelog_file)
    if not all(os.path.exists(path) for path in required_files):
        return False

    try:
        if get_file_hash(release_filepath) != expected_hash:
            return False
        with open(json_file, 'r', encoding='utf-8') as f:
            release_info = json.load(f)
    except Exception:
        return False

    return (
        release_info.get('file_hash') == expected_hash
        and release_info.get('release_file') == expected_release_name
        and release_info.get('checksum_16bit') == expected_checksum16
        and release_info.get('checksum_32bit') == expected_checksum32
    )


def choose_unique_release_dir(release_base, release_subdir):
    """当目标目录已存在时，为强制发布生成新的独立目录名。"""
    primary = os.path.join(release_base, release_subdir)
    if not os.path.exists(primary):
        return primary

    publish_tag = datetime.now().strftime('%H%M%S')
    candidate = os.path.join(release_base, f"{release_subdir}_R{publish_tag}")
    if not os.path.exists(candidate):
        return candidate

    index = 1
    while True:
        next_candidate = os.path.join(
            release_base,
            f"{release_subdir}_R{publish_tag}_{index:02d}",
        )
        if not os.path.exists(next_candidate):
            return next_candidate
        index += 1


def main():
    """主函数"""
    # --force: 强制重新发布当前固件，不因“已处理过”跳过
    force_release = '--force' in sys.argv or os.environ.get('DD_FORCE_RELEASE', '').strip() in ('1', 'true', 'yes', 'on')

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
    
    # 汇总候选目录并统一排序，避免先到先得选中旧产物
    build_dirs = ['Debug', 'Release', 'out']
    firmware_path = find_best_firmware(build_dirs)
    
    if not firmware_path:
        print("提示: 未找到固件文件", file=sys.stderr)
        print("请确保在 out、Release 或 Debug 目录中存在 .bin、.elf 或 .hex 文件", file=sys.stderr)
        emit_result('release_status', 'FAIL')
        emit_result('release_reason', 'no_firmware')
        sys.exit(1)

    firmware_path = os.path.abspath(firmware_path)
    print(f"源固件: {firmware_path}")
    emit_result('source_firmware', firmware_path)
    
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
        build_time_hm = datetime.fromtimestamp(file_mtime).strftime('%H%M')
        
        # 生成新文件名
        new_filepath = generate_release_filename(
            firmware_path,
            project_name=project_name,
            checksum=checksum16,
            date=build_date,
            time_hm=build_time_hm,
        )
        new_filename = os.path.basename(new_filepath)
        
        # 本次发布单独文件夹：项目名+校验码+时间（YYYYMMDD_HHMM）
        safe_project = re.sub(r'[<>:"/\\|?*\s]', '_', project_name).strip('_') or 'firmware'
        release_subdir = f"{safe_project}_{checksum16:04X}_{build_date}_{build_time_hm}"
        release_base = 'release'
        release_dir = os.path.join(release_base, release_subdir)
        release_filepath = os.path.join(release_dir, new_filename)
        json_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.json")
        txt_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.txt")
        release_basename = os.path.splitext(new_filename)[0]
        changelog_file = os.path.join(release_dir, f"{release_basename}_代码修改说明.txt")

        # 检查文件是否已处理；仅当完整发布包已存在时才跳过
        record_file = os.path.join('release', '.processed_files.json')
        file_hash = get_file_hash(firmware_path)
        processed_files = load_processed_files(record_file)
        package_ready = is_release_package_ready(
            release_filepath,
            json_file,
            txt_file,
            changelog_file,
            file_hash,
            new_filename,
            checksum16,
            checksum32,
        )
        if not force_release and file_hash in processed_files and package_ready:
            print(f"提示: 文件已处理过，跳过: {firmware_path}")
            print(f"Release 目录: {os.path.abspath(release_dir)}")
            emit_result('release_status', 'SKIPPED')
            emit_result('release_reason', 'already_processed')
            emit_result('release_dir', os.path.abspath(release_dir))
            emit_result('release_bin_path', os.path.abspath(release_filepath))
            emit_result('package_bin', pass_fail(os.path.exists(release_filepath)))
            emit_result('package_info_json', pass_fail(os.path.exists(json_file)))
            emit_result('package_info_txt', pass_fail(os.path.exists(txt_file)))
            emit_result('package_changelog', pass_fail(os.path.exists(changelog_file)))
            sys.exit(0)

        if force_release:
            unique_release_dir = choose_unique_release_dir(release_base, release_subdir)
            if os.path.abspath(unique_release_dir) != os.path.abspath(release_dir):
                release_dir = unique_release_dir
                release_filepath = os.path.join(release_dir, new_filename)
                json_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.json")
                txt_file = os.path.join(release_dir, f"{os.path.splitext(new_filename)[0]}_info.txt")
                changelog_file = os.path.join(release_dir, f"{release_basename}_代码修改说明.txt")
                print(f"提示: 使用 --force，生成新的独立目录: {os.path.abspath(release_dir)}")
            else:
                print("提示: 使用 --force，强制重新发布当前固件")

        if (not force_release) and file_hash in processed_files and not package_ready:
            print(f"提示: 检测到已处理记录，但发布目录不完整，正在补建: {os.path.abspath(release_dir)}")

        os.makedirs(release_base, exist_ok=True)
        os.makedirs(release_dir, exist_ok=True)
        
        # 复制文件到本次发布目录
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
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(release_info, f, indent=2, ensure_ascii=False)
        
        # 保存 TXT 文件
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
        
        # 同步生成代码修改说明（与本次 Release 对应）
        change_content = get_code_change_description(project_root)
        with open(changelog_file, 'w', encoding='utf-8') as f:
            f.write(change_content)
        print(f"PASS 代码修改说明: {os.path.basename(changelog_file)}")
        
        # 记录已处理文件
        save_processed_file(record_file, file_hash)
        
        # 打印摘要信息
        print("\n" + "=" * 60)
        print("固件发布处理完成")
        print("=" * 60)
        print(f"原始文件: {release_info['original_file']}")
        print(f"源固件路径: {firmware_path}")
        print(f"发布文件: {new_filename}")
        print(f"目标发布文件: {os.path.abspath(release_filepath)}")
        print(f"文件大小: {release_info['file_size_kb']} KB")
        if version:
            print(f"版本号: V{version}")
        print(f"构建日期: {build_date}")
        print(f"16位校验码: {release_info['checksum_16bit_hex']}")
        print(f"Release 目录: {os.path.abspath(release_dir)}")
        print("=" * 60)
        emit_result('release_status', 'PASS')
        emit_result('release_dir', os.path.abspath(release_dir))
        emit_result('release_bin_path', os.path.abspath(release_filepath))
        emit_result('package_bin', pass_fail(os.path.exists(release_filepath)))
        emit_result('package_info_json', pass_fail(os.path.exists(json_file)))
        emit_result('package_info_txt', pass_fail(os.path.exists(txt_file)))
        emit_result('package_changelog', pass_fail(os.path.exists(changelog_file)))
        
    except Exception as e:
        print(f"错误: {e}", file=sys.stderr)
        emit_result('release_status', 'FAIL')
        emit_result('release_reason', 'exception')
        if 'release_dir' in locals():
            emit_result('release_dir', os.path.abspath(release_dir))
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
