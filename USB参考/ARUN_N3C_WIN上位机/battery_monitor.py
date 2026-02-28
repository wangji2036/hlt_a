"""
ARUN 充电宝智能监测系统 - Windows 上位机
版本: v6.0 (单列滚动 UI，与设计 PDF 对齐)
基于固件: NANFU_USB_1052_20260121A (WB7720 MCU)
目标硬件: ARUN IP162N 15W PowerBank (NU17112 + NU6805)

协议: USB HID (VID=0xFFFF, PID=0xFFFF, 64B Report)
布局: 单列滚动 (标题 / SOC圆环 / 循环次数 / 电芯温度卡片 / 电芯电压卡片 / 设备信息卡片 / 异常记录卡片)

依赖: pip install hidapi Pillow
"""

import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import threading
import struct
import time
import math
import os
import sys
from dataclasses import dataclass, field
from typing import List, Dict, Optional

# ---------------------------------------------------------------------------
# 尝试导入 hidapi，未安装时给出提示
# ---------------------------------------------------------------------------
try:
    import hid
    HID_AVAILABLE = True
except ImportError:
    HID_AVAILABLE = False
    print("[WARNING] hidapi 未安装，请运行: pip install hidapi")

# ---------------------------------------------------------------------------
# 尝试导入 Pillow
# ---------------------------------------------------------------------------
try:
    from PIL import Image, ImageTk
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("[WARNING] Pillow 未安装，请运行: pip install Pillow")

# ---------------------------------------------------------------------------
# 路径常量（兼容 PyInstaller 打包后的 _MEIPASS 路径）
# ---------------------------------------------------------------------------
def _get_ui_dir() -> str:
    if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
        # 打包后：资源在 _MEIPASS/UI/
        return os.path.join(sys._MEIPASS, 'UI')
    else:
        # 开发模式：相对脚本的 ../ARUN_N3C_UI设计/UI
        this_dir = os.path.dirname(os.path.abspath(__file__))
        return os.path.normpath(os.path.join(this_dir, '..', 'ARUN_N3C_UI设计', 'UI'))

UI_DIR = _get_ui_dir()
FONT_DIR = os.path.join(UI_DIR, 'Font-OPPOSans')

# ---------------------------------------------------------------------------
# 常量
# ---------------------------------------------------------------------------
DEFAULT_VENDOR_ID  = 0xFFFF
DEFAULT_PRODUCT_ID = 0xFFFF
DEFAULT_USAGE_PAGE = 0xFF00
DEFAULT_USAGE_ID   = 0x01
REPORT_LENGTH      = 64        # 实际 HID 报告长度
HID_WRITE_LENGTH   = 65        # hidapi write 需多加 0x00 前缀

# 命令码
CMD_READ_STATUS      = 0x01
CMD_READ_DEVICE_INFO = 0x02
CMD_WRITE_REGISTER   = 0x0C

# 回复 Type
TYPE_TELEMETRY    = 0x01
TYPE_EXCEPT_LOG   = 0x02
TYPE_DEVICE_INFO  = 0x03   # 接口规范 v1.2: 设备信息使用 Type 0x03

# SOF 帧头
SOF_BYTES = bytes([0x05, 0xA5, 0x5A])
PROTOCOL_VER = 0x02

# 工程模式解锁密码
ENG_MODE_PASSWORD = "123456"
PROD_MODE_PASSWORD = "123456"

# 安全使用年限（GB/T 35590 硬编码）
SAFETY_YEARS_STR = "5年"

# 状态阈值
TEMP_WARN_THRESHOLD   = 45.0   # °C，超过为异常
CELL_VOLT_MIN         = 3.0    # V，低于为异常
CELL_VOLT_MAX         = 4.35   # V，高于为异常

# 异常日志最大显示条数
MAX_EXCEPTION_DISPLAY = 20

# 设备信息读取超时（秒）
DEVICE_INFO_TIMEOUT = 5.0

# ProductInfo 刷新间隔（秒）。0 = 仅连接时读取一次；>0 = 定期刷新
PROD_INFO_REFRESH_INTERVAL_S = 3

# 写入步骤间隔（秒）
WRITE_STEP_INTERVAL = 0.05

# 窗口宽度（固定）
WIN_WIDTH = 600

# 配色
COLORS = {
    'body_bg':        '#F0F4F8',   # 浅蓝灰背景（与设计一致）
    'card_bg':        '#FFFFFF',   # 卡片白色
    'card_border':    '#E0E0E0',   # 卡片边框
    'text_primary':   '#333333',   # 主文字深灰
    'text_secondary': '#888888',   # 副文字中灰
    'normal':         '#4CAF50',   # 正常绿
    'error':          '#FF4444',   # 异常红
    'warning':        '#FFAA00',   # 警告橙
    'ring_fill':      '#4CAF50',   # SOC 圆弧填充色（绿）
    'ring_empty':     '#CCCCCC',   # SOC 圆弧空白
    'exception_text': '#FF4444',   # 异常记录文字
    'icon_blue':      '#4B8BFF',   # 蓝色图标
    'icon_red':       '#FF4444',   # 红色图标
    'arun_green':     '#3AAA35',   # ARUN logo 绿色
    'conn_bar_bg':    '#EAEEF2',   # 连接控制栏背景
    'header_text':    '#FFFFFF',
}

# 异常类型映射
ERR_TYPE_MAP = {0x01: "电压过高", 0x02: "过温", 0x03: "欠温"}
SUB_TYPE_TEMP = {0: "充电", 1: "放电"}

# ---------------------------------------------------------------------------
# 字体初始化（OPPOSans 注册）
# ---------------------------------------------------------------------------
FONT_REGISTERED = False

def _register_opposan_fonts():
    """在 Windows 上通过 GDI 注册 OPPOSans 字体，失败时静默回退"""
    global FONT_REGISTERED
    if FONT_REGISTERED:
        return
    try:
        from ctypes import windll
        for fname in ['OPPOSans-M.ttf', 'OPPOSans-B.ttf', 'OPPOSans-R.ttf']:
            fpath = os.path.join(FONT_DIR, fname)
            if os.path.isfile(fpath):
                windll.gdi32.AddFontResourceW(fpath)
        FONT_REGISTERED = True
    except Exception:
        pass


def _get_tk_font(size, bold=False):
    """返回 tkinter font tuple，优先 OPPOSans，回退 Microsoft YaHei"""
    _register_opposan_fonts()
    if FONT_REGISTERED:
        family = 'OPPOSans B' if bold else 'OPPOSans M'
        return (family, size)
    weight = 'bold' if bold else 'normal'
    return ('Microsoft YaHei', size, weight)

# ---------------------------------------------------------------------------
# 数据结构
# ---------------------------------------------------------------------------
@dataclass
class BatteryData:
    """电池遥测数据"""
    # 显示字段
    soc: int = 0
    cycle_count: int = 0
    bat_temp: float = 0.0
    total_voltage: float = 0.0
    cell1_voltage: float = 0.0
    cell2_voltage: float = 0.0
    voltage_delta: float = 0.0
    exception_logs: List[Dict] = field(default_factory=list)

    # 内部字段（不显示，保留供调试）
    capacity: int = 0
    current: int = 0
    power: float = 0.0
    board_temp: float = 0.0
    r_internal: int = 0
    soh: int = 0
    charge_state: int = 0
    err_overtemp_cnt: int = 0
    err_overvolt_cnt: int = 0
    err_overcurr_cnt: int = 0
    exception_log_cnt: int = 0


@dataclass
class DeviceInfo:
    """设备基本信息（来自 CMD_READ_DEVICE_INFO）"""
    manufacturer: str = ""
    model: str = ""
    battery_mfr: str = ""
    battery_model: str = ""
    prod_date: str = ""
    safety_years: str = SAFETY_YEARS_STR   # 硬编码 GB/T 35590
    loaded: bool = False
    # 子页收集状态
    _sub_received: set = field(default_factory=set)


# ---------------------------------------------------------------------------
# 协议解析工具
# ---------------------------------------------------------------------------
def _decode_str(raw: bytes) -> str:
    """将字节串解码为字符串，去除尾部 0x00"""
    try:
        return raw.rstrip(b'\x00').decode('utf-8', errors='replace').strip()
    except Exception:
        return ""


def _parse_telemetry(payload: bytes) -> Optional[BatteryData]:
    """解析 Type 0x01 遥测 Payload（从偏移 9 之后的内容，长度应 >= 35）"""
    if len(payload) < 35:
        return None

    bd = BatteryData()
    bd.soc          = payload[0]
    bd.capacity,    = struct.unpack_from('<I', payload, 1)
    total_v_raw,    = struct.unpack_from('<H', payload, 5)
    bd.total_voltage = total_v_raw / 100.0

    current_raw,    = struct.unpack_from('<h', payload, 7)
    bd.current      = current_raw         # A×100，内部保留

    power_raw,      = struct.unpack_from('<h', payload, 9)
    bd.power        = power_raw / 10.0

    bd.charge_state = payload[11]

    cycle_raw,      = struct.unpack_from('<H', payload, 12)
    bd.cycle_count  = cycle_raw

    # 温度智能识别：raw < 100 → 直接 °C；raw >= 100 → ÷10 得 °C
    temp_raw,       = struct.unpack_from('<h', payload, 14)
    if temp_raw < 100:
        bd.bat_temp = float(temp_raw)
    else:
        bd.bat_temp = temp_raw / 10.0

    board_raw,      = struct.unpack_from('<h', payload, 16)
    if board_raw < 100:
        bd.board_temp = float(board_raw)
    else:
        bd.board_temp = board_raw / 10.0

    # cell count at +18 (ignored, fixed 2)
    cell1_raw,      = struct.unpack_from('<H', payload, 19)
    cell2_raw,      = struct.unpack_from('<H', payload, 21)
    bd.cell1_voltage = cell1_raw / 100.0
    bd.cell2_voltage = cell2_raw / 100.0
    bd.voltage_delta = abs(bd.cell1_voltage - bd.cell2_voltage)

    r_raw,          = struct.unpack_from('<H', payload, 23)
    bd.r_internal   = r_raw

    soh_raw,        = struct.unpack_from('<H', payload, 25)
    bd.soh          = soh_raw    # pct×100

    bd.err_overtemp_cnt,  = struct.unpack_from('<H', payload, 27)
    bd.err_overvolt_cnt,  = struct.unpack_from('<H', payload, 29)
    bd.err_overcurr_cnt,  = struct.unpack_from('<H', payload, 31)
    bd.exception_log_cnt  = payload[33]

    return bd


def _parse_exception_record(raw20: bytes) -> Optional[Dict]:
    """解析一条 20 字节 BatteryExceptionRecord_t"""
    if len(raw20) < 20:
        return None
    year,  = struct.unpack_from('<H', raw20, 0)
    month  = raw20[2]
    day    = raw20[3]
    hour   = raw20[4]
    minute = raw20[5]
    second = raw20[6]
    # +7 reserved
    error_type = raw20[8]
    sub_type   = raw20[9]
    data_low,  = struct.unpack_from('<H', raw20, 10)
    data_high, = struct.unpack_from('<H', raw20, 12)
    # +14~+15 padding
    record_id, = struct.unpack_from('<I', raw20, 16)

    ts_str = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}"

    if error_type == 0x01:
        # OV 过压
        prefix = f"电芯{sub_type}"
        desc   = "电池充电电压异常"
        value_str = f"{data_low / 1000:.1f}V"
        text = f"{prefix} {desc} {value_str}[ {ts_str} ]"
    elif error_type == 0x02:
        # OT 过温
        prefix = f"电芯{sub_type}"
        charge_str = SUB_TYPE_TEMP.get(sub_type, f"状态{sub_type}")
        desc   = f"电池{charge_str}温度异常"
        dl_signed = struct.unpack_from('<h', raw20, 10)[0]
        value_str = f"{dl_signed / 10:.0f}℃"
        text = f"{prefix} {desc} {value_str}[ {ts_str} ]"
    elif error_type == 0x03:
        # UT 欠温
        prefix = f"电芯{sub_type}"
        charge_str = SUB_TYPE_TEMP.get(sub_type, f"状态{sub_type}")
        desc   = f"电池{charge_str}温度异常"
        dl_signed = struct.unpack_from('<h', raw20, 10)[0]
        value_str = f"{dl_signed / 10:.0f}℃"
        text = f"{prefix} {desc} {value_str}[ {ts_str} ]"
    else:
        text = f"未知异常类型 0x{error_type:02X}[ {ts_str} ]"

    return {
        'record_id':   record_id,
        'error_type':  error_type,
        'sub_type':    sub_type,
        'data_low':    data_low,
        'data_high':   data_high,
        'timestamp':   ts_str,
        'text':        text,
    }


def _parse_exception_payload(payload: bytes) -> List[Dict]:
    """解析 Type 0x02 异常日志 Payload，返回记录列表"""
    if len(payload) < 2:
        return []
    # offset_page = payload[0]
    return_count = payload[1]
    records = []
    for i in range(return_count):
        offset = 2 + i * 20
        if offset + 20 > len(payload):
            break
        rec = _parse_exception_record(payload[offset:offset + 20])
        if rec:
            records.append(rec)
    return records


def _parse_device_info_payload(payload: bytes, dev_info: DeviceInfo) -> bool:
    """
    解析 Type 0x03 设备信息 Payload（Len=41）。
    payload 从偏移 9 开始（不含协议头）。
    返回 True 表示 3 包均已收到。
    """
    if len(payload) < 41:
        return False

    sub_idx = payload[0]
    field1  = payload[1:21]
    field2  = payload[21:41]

    if sub_idx == 0x00:
        # 收到包1：重置缓冲
        dev_info._sub_received.clear()
        dev_info.manufacturer = _decode_str(field1)
        dev_info.model        = _decode_str(field2)
        dev_info._sub_received.add(0x00)
    elif sub_idx == 0x01:
        dev_info.battery_mfr   = _decode_str(field1)
        dev_info.battery_model = _decode_str(field2)
        dev_info._sub_received.add(0x01)
    elif sub_idx == 0x02:
        dev_info.prod_date     = _decode_str(field1)
        # field2[0] 是 safety_years 字节，但上位机硬编码为 "5年"
        dev_info.safety_years  = SAFETY_YEARS_STR
        dev_info._sub_received.add(0x02)

    if {0x00, 0x01, 0x02}.issubset(dev_info._sub_received):
        dev_info.loaded = True
        return True
    return False


def _parse_hid_report(data: bytes):
    """
    解析一帧 64 字节 HID 报告。
    返回 (type, payload_bytes) 或 (None, None) 表示解析失败。
    """
    if len(data) < 9:
        return None, None
    # SOF 验证
    if data[0:3] != SOF_BYTES:
        print(f"[WARN] SOF mismatch: {data[0:3].hex()}")
        return None, None
    # 协议版本（宽容检查，不强制失败）
    if data[3] != PROTOCOL_VER:
        print(f"[WARN] Protocol version: 0x{data[3]:02X} (expected 0x{PROTOCOL_VER:02X})")

    report_type = data[4]
    seq,        = struct.unpack_from('<H', data, 5)
    length,     = struct.unpack_from('<H', data, 7)

    payload_end = 9 + length
    if payload_end > len(data):
        payload_end = len(data)
    payload = data[9:payload_end]

    return report_type, payload


# ---------------------------------------------------------------------------
# PNG 工具函数
# ---------------------------------------------------------------------------
def _load_png_composite(png_path: str, scale: float = 1.0,
                         bg_color=(240, 244, 248, 255)) -> Optional['Image.Image']:
    """
    加载 PNG，与背景色合成，返回 PIL Image。
    如果 Pillow 不可用或文件不存在返回 None。
    """
    if not PIL_AVAILABLE:
        return None
    if not os.path.isfile(png_path):
        return None
    try:
        img = Image.open(png_path).convert('RGBA')
        if scale != 1.0:
            new_w = int(img.width * scale)
            new_h = int(img.height * scale)
            img = img.resize((new_w, new_h), Image.LANCZOS)
        bg = Image.new('RGBA', img.size, bg_color)
        composite = Image.alpha_composite(bg, img)
        return composite
    except Exception as e:
        print(f"[WARN] Cannot load PNG {png_path}: {e}")
        return None


# ---------------------------------------------------------------------------
# 卡片工具：白色圆角卡片（Frame 模拟）
# ---------------------------------------------------------------------------
def _make_card_frame(parent: tk.Widget, title: str,
                     icon_color: str = COLORS['icon_blue']) -> tk.Frame:
    """
    创建白色卡片 Frame，带标题行（彩色圆点 + 标题文字）。
    返回 card_body（内容区 Frame），调用者向其中填充内容。
    """
    # 外层提供 padding，视觉上形成卡片间距
    outer = tk.Frame(parent, bg=COLORS['body_bg'])
    outer.pack(fill='x', padx=16, pady=(0, 8))

    # 白色卡片体
    card = tk.Frame(outer, bg=COLORS['card_bg'],
                    relief='flat', bd=0,
                    highlightthickness=1,
                    highlightbackground='#E8ECF0',
                    highlightcolor='#E8ECF0')
    card.pack(fill='x')

    # 标题行
    title_row = tk.Frame(card, bg=COLORS['card_bg'])
    title_row.pack(fill='x', padx=16, pady=(12, 8))

    tk.Label(title_row, text='●',
             fg=icon_color, bg=COLORS['card_bg'],
             font=_get_tk_font(11)).pack(side='left')
    tk.Label(title_row, text=f'  {title}',
             bg=COLORS['card_bg'], fg=COLORS['text_primary'],
             font=_get_tk_font(12, bold=True)).pack(side='left')

    # 分隔线
    sep = tk.Frame(card, bg='#E8ECF0', height=1)
    sep.pack(fill='x', padx=0)

    # 内容区
    body = tk.Frame(card, bg=COLORS['card_bg'])
    body.pack(fill='x', padx=16, pady=12)

    return body


# ---------------------------------------------------------------------------
# SOC 圆环组件（Canvas 绘制，PNG 外框叠加）
# ---------------------------------------------------------------------------
class SOCRingWidget:
    """
    SOC 圆弧圈：
    - Canvas 底层画分段圆弧
    - 叠加 PNG 外框装饰（环形电量显示.png）
    - 中央显示百分比（大字）+ "电量"（小字）
    居中放置，宽度跟随 WIN_WIDTH
    """

    SEGMENT_COUNT = 36
    GAP_DEG       = 3.0
    ARC_START     = 225     # tkinter 角度，逆时针为正
    ARC_SPAN      = 270

    def __init__(self, parent: tk.Widget):
        # Canvas 宽固定 WIN_WIDTH，高 260
        self.canvas_w = WIN_WIDTH
        self.canvas_h = 260

        # 圆环参数（居中）
        self.cx = self.canvas_w // 2
        self.cy = 125
        self.ring_r = 100
        self.arc_width = 14

        png_path = os.path.join(UI_DIR, '环形电量显示.png')
        # 按圆环尺寸缩放 PNG（原始 577x496，圆心 288,248，半径 ~160）
        # 目标：圆环半径 100px → scale = 100/160 ≈ 0.625
        scale = self.ring_r / 160.0
        bg_rgba = tuple(int(COLORS['body_bg'].lstrip('#')[i:i+2], 16)
                        for i in (0, 2, 4)) + (255,)
        self._ring_img = _load_png_composite(png_path, scale=scale, bg_color=bg_rgba)

        self.canvas = tk.Canvas(parent,
                                width=self.canvas_w,
                                height=self.canvas_h,
                                highlightthickness=0,
                                bg=COLORS['body_bg'])
        self.canvas.pack(fill='x')

        self._soc = 0
        self._arc_items: List[int] = []
        self._pct_item: Optional[int] = None
        self._lbl_item: Optional[int] = None

        self._draw_initial()

    def _draw_initial(self):
        self.canvas.delete('all')

        # 背景填充
        self.canvas.create_rectangle(0, 0, self.canvas_w, self.canvas_h,
                                     fill=COLORS['body_bg'], outline='')

        # 绘制分段圆弧
        seg_span = (self.ARC_SPAN - self.GAP_DEG * self.SEGMENT_COUNT) / self.SEGMENT_COUNT
        r = self.ring_r
        x0 = self.cx - r
        y0 = self.cy - r
        x1 = self.cx + r
        y1 = self.cy + r

        for i in range(self.SEGMENT_COUNT):
            seg_start = self.ARC_START - i * (seg_span + self.GAP_DEG)
            item = self.canvas.create_arc(
                x0, y0, x1, y1,
                start=seg_start, extent=seg_span,
                style='arc',
                outline=COLORS['ring_empty'],
                width=self.arc_width
            )
            self._arc_items.append(item)

        # 叠加 PNG 外框装饰
        if self._ring_img is not None:
            self._tk_img = ImageTk.PhotoImage(self._ring_img)
            # PNG 居中对齐到圆环中心
            png_w = self._ring_img.width
            png_h = self._ring_img.height
            # 原始 PNG 圆心在 288/577 处（x 方向），288*scale 偏移
            png_cx = int(288 * self.ring_r / 160.0)
            png_cy = int(248 * self.ring_r / 160.0)
            img_x = self.cx - png_cx
            img_y = self.cy - png_cy
            self.canvas.create_image(img_x, img_y, anchor='nw', image=self._tk_img)

        # 中央文字
        font_pct = _get_tk_font(32, bold=True)
        font_lbl = _get_tk_font(13)
        self._pct_item = self.canvas.create_text(
            self.cx, self.cy - 14,
            text="0%", font=font_pct,
            fill=COLORS['text_primary'], anchor='center'
        )
        self._lbl_item = self.canvas.create_text(
            self.cx, self.cy + 22,
            text="电量", font=font_lbl,
            fill=COLORS['text_secondary'], anchor='center'
        )

    def _soc_color(self, soc: int) -> str:
        if soc > 50:
            return COLORS['ring_fill']
        elif soc > 20:
            return COLORS['warning']
        else:
            return COLORS['error']

    def update_soc(self, soc: int):
        self._soc = max(0, min(100, soc))
        filled = round(self._soc * self.SEGMENT_COUNT / 100)
        color  = self._soc_color(self._soc)

        for i, item in enumerate(self._arc_items):
            c = color if i < filled else COLORS['ring_empty']
            self.canvas.itemconfig(item, outline=c)

        if self._pct_item:
            self.canvas.itemconfig(self._pct_item,
                                   text=f"{self._soc}%",
                                   fill=color)


# ---------------------------------------------------------------------------
# 主应用
# ---------------------------------------------------------------------------
class BatteryMonitorApp:

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("ARUN 充电宝智能监测系统")
        self.root.configure(bg=COLORS['body_bg'])
        self.root.geometry(f'{WIN_WIDTH}x900')
        self.root.resizable(False, True)

        # 数据
        self.battery_data  = BatteryData()
        self.device_info   = DeviceInfo()
        self._exception_seen_ids: set = set()
        self._exception_list: List[str] = []

        # HID 状态
        self._hid_device   = None
        self._connected    = False
        self._reading_busy = False
        self._poll_counter = 0
        self._dev_info_timeout_id = None

        # 工程/生产模式
        self._eng_mode_active  = False
        self._prod_mode_active = False

        # UI 组件引用
        self._soc_ring: Optional[SOCRingWidget] = None
        self._cycle_val_label: Optional[tk.Label] = None

        # 温度表格标签（key → Label widget）
        self._temp_labels: Dict[str, tk.Label] = {}
        # 电压表格标签
        self._volt_labels: Dict[str, tk.Label] = {}
        # 设备信息标签
        self._info_labels: Dict[str, tk.Label] = {}
        # 异常记录 Text
        self._exc_text: Optional[tk.Text] = None

        # 构建 UI
        self._build_ui()

        # 绑定关闭事件
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

    # -----------------------------------------------------------------------
    # UI 构建入口
    # -----------------------------------------------------------------------
    def _build_ui(self):
        # ---- 顶部连接控制栏（不滚动，固定在窗口顶部）----
        self._build_connection_bar(self.root)

        # ---- 主滚动区 ----
        scroll_canvas = tk.Canvas(self.root, bg=COLORS['body_bg'],
                                  highlightthickness=0)
        scrollbar = ttk.Scrollbar(self.root, orient='vertical',
                                  command=scroll_canvas.yview)
        scroll_canvas.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side='right', fill='y')
        scroll_canvas.pack(side='left', fill='both', expand=True)

        self._content_frame = tk.Frame(scroll_canvas, bg=COLORS['body_bg'],
                                       width=WIN_WIDTH)
        self._canvas_window = scroll_canvas.create_window(
            (0, 0), window=self._content_frame, anchor='nw'
        )
        self._content_frame.bind(
            '<Configure>',
            lambda *_: scroll_canvas.configure(
                scrollregion=scroll_canvas.bbox('all')
            )
        )
        scroll_canvas.bind(
            '<Configure>',
            lambda e: scroll_canvas.itemconfig(
                self._canvas_window, width=e.width
            )
        )
        # 鼠标滚轮
        self.root.bind(
            '<MouseWheel>',
            lambda e: scroll_canvas.yview_scroll(int(-1 * (e.delta / 120)), 'units')
        )

        content = self._content_frame

        # ---- 标题区 ----
        self._build_title(content)

        # ---- SOC 圆环 ----
        self._build_soc_ring(content)

        # ---- 电池循环次数行 ----
        self._build_cycle_row(content)

        # ---- 电芯温度卡片 ----
        self._build_temp_card(content)

        # ---- 电芯电压卡片 ----
        self._build_volt_card(content)

        # ---- 设备基本信息卡片 ----
        self._build_info_card(content)

        # ---- 异常监督记录卡片 ----
        self._build_exc_card(content)

        # ---- 工程/生产模式面板（隐藏，切换模式时显示）----
        self._build_panel_engineering(content)
        self._build_panel_production(content)

        # 底部留白
        tk.Frame(content, bg=COLORS['body_bg'], height=20).pack()

    # -----------------------------------------------------------------------
    # 连接控制栏（固定在窗口顶部，不随内容滚动）
    # -----------------------------------------------------------------------
    def _build_connection_bar(self, parent):
        bar = tk.Frame(parent, bg=COLORS['conn_bar_bg'], pady=4)
        bar.pack(fill='x', side='top')

        font_s = _get_tk_font(9)

        # 第一行: VID/PID/UsagePage/刷新/设备列表/连接/状态
        row1 = tk.Frame(bar, bg=COLORS['conn_bar_bg'])
        row1.pack(fill='x', padx=8, pady=(0, 2))

        for label, default, attr in [
            ('VID:', 'FFFF', '_vid_var'),
            ('PID:', 'FFFF', '_pid_var'),
            ('UsagePage:', 'FF00', '_page_var'),
        ]:
            tk.Label(row1, text=label,
                     bg=COLORS['conn_bar_bg'], fg=COLORS['text_primary'],
                     font=font_s).pack(side='left', padx=(4, 0))
            var = tk.StringVar(value=default)
            setattr(self, attr, var)
            tk.Entry(row1, textvariable=var, width=6, font=font_s).pack(side='left', padx=(0, 4))

        tk.Button(row1, text="刷新",
                  command=self._refresh_devices,
                  font=font_s, relief='flat',
                  bg='#D0D4D8', padx=4).pack(side='left', padx=2)

        self._dev_var = tk.StringVar()
        self._dev_combo = ttk.Combobox(row1, textvariable=self._dev_var,
                                       width=18, state='readonly', font=font_s)
        self._dev_combo.pack(side='left', padx=4)

        self._conn_btn = tk.Button(
            row1, text="连接",
            command=self._toggle_connection,
            font=font_s, relief='flat',
            bg=COLORS['normal'], fg='white', padx=6
        )
        self._conn_btn.pack(side='left', padx=4)

        self._status_var = tk.StringVar(value="未连接")
        tk.Label(row1, textvariable=self._status_var,
                 bg=COLORS['conn_bar_bg'], fg=COLORS['text_secondary'],
                 font=font_s).pack(side='left', padx=4)

        # 第二行: 三态模式切换（居左对齐）
        row2 = tk.Frame(bar, bg=COLORS['conn_bar_bg'])
        row2.pack(fill='x', padx=8, pady=(2, 0))

        tk.Label(row2, text="模式：",
                 bg=COLORS['conn_bar_bg'], fg=COLORS['text_secondary'],
                 font=font_s).pack(side='left')

        self._mode_var = tk.StringVar(value='user')
        for val, lbl in [('user', '用户'), ('eng', '工程'), ('prod', '生产')]:
            rb = tk.Radiobutton(
                row2, text=lbl, variable=self._mode_var, value=val,
                bg=COLORS['conn_bar_bg'], fg=COLORS['text_primary'],
                selectcolor=COLORS['conn_bar_bg'],
                activebackground=COLORS['conn_bar_bg'],
                font=font_s,
                command=self._on_mode_change
            )
            rb.pack(side='left', padx=2)

        self._refresh_devices()

    # -----------------------------------------------------------------------
    # 标题区（无卡片框，在背景上）
    # -----------------------------------------------------------------------
    def _build_title(self, parent):
        title_frame = tk.Frame(parent, bg=COLORS['body_bg'])
        title_frame.pack(fill='x', pady=(12, 4))

        inner = tk.Frame(title_frame, bg=COLORS['body_bg'])
        inner.pack(anchor='center')

        # 尝试加载 logo PNG
        logo_path = os.path.join(UI_DIR, 'logo.png')
        logo_loaded = False
        if PIL_AVAILABLE and os.path.isfile(logo_path):
            try:
                logo_img = Image.open(logo_path).convert('RGBA')
                # 缩放到高度 28px
                lh = 28
                lw = int(logo_img.width * lh / logo_img.height)
                logo_img = logo_img.resize((lw, lh), Image.LANCZOS)
                bg_rgba = tuple(int(COLORS['body_bg'].lstrip('#')[i:i+2], 16)
                                for i in (0, 2, 4)) + (255,)
                bg = Image.new('RGBA', logo_img.size, bg_rgba)
                comp = Image.alpha_composite(bg, logo_img)
                self._logo_tk = ImageTk.PhotoImage(comp)
                logo_lbl = tk.Label(inner, image=self._logo_tk,
                                    bg=COLORS['body_bg'])
                logo_lbl.pack(side='left', padx=(0, 6))
                logo_loaded = True
            except Exception:
                pass

        if not logo_loaded:
            # 文字 ARUN（绿色，仿 logo）
            tk.Label(inner, text="ARUN",
                     bg=COLORS['body_bg'], fg=COLORS['arun_green'],
                     font=_get_tk_font(18, bold=True)).pack(side='left', padx=(0, 6))

        tk.Label(inner, text="充电宝智能监测系统",
                 bg=COLORS['body_bg'], fg=COLORS['text_primary'],
                 font=_get_tk_font(16, bold=True)).pack(side='left')

    # -----------------------------------------------------------------------
    # SOC 圆环区（无卡片框）
    # -----------------------------------------------------------------------
    def _build_soc_ring(self, parent):
        container = tk.Frame(parent, bg=COLORS['body_bg'])
        container.pack(fill='x', pady=(0, 0))
        self._soc_ring = SOCRingWidget(container)

    # -----------------------------------------------------------------------
    # 电池循环次数行（无卡片框，纯文字行）
    # -----------------------------------------------------------------------
    def _build_cycle_row(self, parent):
        row = tk.Frame(parent, bg=COLORS['body_bg'])
        row.pack(fill='x', padx=24, pady=(6, 16))

        tk.Label(row, text='电池循环次数',
                 bg=COLORS['body_bg'], fg=COLORS['text_primary'],
                 font=_get_tk_font(12)).pack(side='left')

        self._cycle_val_label = tk.Label(
            row, text='-- 次',
            bg=COLORS['body_bg'], fg=COLORS['text_primary'],
            font=_get_tk_font(12)
        )
        self._cycle_val_label.pack(side='right')

    # -----------------------------------------------------------------------
    # 电芯温度卡片（白色卡片，蓝色图标）
    # -----------------------------------------------------------------------
    def _build_temp_card(self, parent):
        body = _make_card_frame(parent, '电芯温度', icon_color=COLORS['icon_blue'])

        # 表头行（灰色背景）
        hdr = tk.Frame(body, bg='#F6F8FA')
        hdr.pack(fill='x', pady=(0, 4))

        for col, text, width, anchor in [
            (0, '电芯',     10, 'w'),
            (1, '温度(℃)',  10, 'center'),
            (2, '状态',      8, 'center'),
        ]:
            tk.Label(hdr, text=text, width=width, anchor=anchor,
                     bg='#F6F8FA', fg='#666666',
                     font=_get_tk_font(10, bold=True),
                     pady=6).grid(row=0, column=col, padx=8)

        # 数据行
        self._temp_labels = {}
        for row_i, cell_name in enumerate(['电芯1', '电芯2']):
            row_bg = COLORS['card_bg']
            row_frame = tk.Frame(body, bg=row_bg)
            row_frame.pack(fill='x')

            # 电芯名
            tk.Label(row_frame, text=cell_name, width=10, anchor='w',
                     bg=row_bg, fg=COLORS['text_primary'],
                     font=_get_tk_font(11),
                     pady=8).grid(row=0, column=0, padx=8)

            # 温度值
            val_lbl = tk.Label(row_frame, text='--', width=10, anchor='center',
                                bg=row_bg, fg=COLORS['text_primary'],
                                font=_get_tk_font(12))
            val_lbl.grid(row=0, column=1, padx=8)
            self._temp_labels[f'c{row_i+1}_temp'] = val_lbl

            # 状态
            stat_lbl = tk.Label(row_frame, text='--', width=8, anchor='center',
                                 bg=row_bg, fg=COLORS['normal'],
                                 font=_get_tk_font(11))
            stat_lbl.grid(row=0, column=2, padx=8)
            self._temp_labels[f'c{row_i+1}_status'] = stat_lbl

            # 分隔线（不含最后一行）
            if row_i < 1:
                sep = tk.Frame(body, bg='#F0F0F0', height=1)
                sep.pack(fill='x')

    # -----------------------------------------------------------------------
    # 电芯电压卡片
    # -----------------------------------------------------------------------
    def _build_volt_card(self, parent):
        body = _make_card_frame(parent, '电芯电压', icon_color=COLORS['icon_blue'])

        # 表头行
        hdr = tk.Frame(body, bg='#F6F8FA')
        hdr.pack(fill='x', pady=(0, 4))

        for col, text, width, anchor in [
            (0, '电芯',    10, 'w'),
            (1, '电压(V)', 10, 'center'),
            (2, '状态',     8, 'center'),
        ]:
            tk.Label(hdr, text=text, width=width, anchor=anchor,
                     bg='#F6F8FA', fg='#666666',
                     font=_get_tk_font(10, bold=True),
                     pady=6).grid(row=0, column=col, padx=8)

        # 数据行 - 电芯1/2
        self._volt_labels = {}
        for row_i, cell_name in enumerate(['电芯1', '电芯2']):
            row_bg = COLORS['card_bg']
            row_frame = tk.Frame(body, bg=row_bg)
            row_frame.pack(fill='x')

            tk.Label(row_frame, text=cell_name, width=10, anchor='w',
                     bg=row_bg, fg=COLORS['text_primary'],
                     font=_get_tk_font(11),
                     pady=8).grid(row=0, column=0, padx=8)

            val_lbl = tk.Label(row_frame, text='--', width=10, anchor='center',
                                bg=row_bg, fg=COLORS['text_primary'],
                                font=_get_tk_font(12))
            val_lbl.grid(row=0, column=1, padx=8)
            self._volt_labels[f'v{row_i+1}_volt'] = val_lbl

            stat_lbl = tk.Label(row_frame, text='--', width=8, anchor='center',
                                 bg=row_bg, fg=COLORS['normal'],
                                 font=_get_tk_font(11))
            stat_lbl.grid(row=0, column=2, padx=8)
            self._volt_labels[f'v{row_i+1}_status'] = stat_lbl

            sep = tk.Frame(body, bg='#F0F0F0', height=1)
            sep.pack(fill='x')

        # 总电压行
        total_row = tk.Frame(body, bg=COLORS['card_bg'])
        total_row.pack(fill='x', pady=(6, 2))
        tk.Label(total_row, text='总电压',
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=_get_tk_font(11)).pack(side='left', padx=8)
        total_val = tk.Label(total_row, text='-- V',
                              bg=COLORS['card_bg'], fg=COLORS['text_primary'],
                              font=_get_tk_font(12, bold=True))
        total_val.pack(side='right', padx=8)
        self._volt_labels['total_volt'] = total_val

        # 电压差行
        delta_row = tk.Frame(body, bg=COLORS['card_bg'])
        delta_row.pack(fill='x', pady=(2, 0))
        tk.Label(delta_row, text='电压差',
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=_get_tk_font(11)).pack(side='left', padx=8)
        delta_val = tk.Label(delta_row, text='-- V',
                              bg=COLORS['card_bg'], fg=COLORS['text_primary'],
                              font=_get_tk_font(12, bold=True))
        delta_val.pack(side='right', padx=8)
        self._volt_labels['delta_volt'] = delta_val

    # -----------------------------------------------------------------------
    # 设备基本信息卡片
    # -----------------------------------------------------------------------
    def _build_info_card(self, parent):
        body = _make_card_frame(parent, '设备基本信息', icon_color=COLORS['icon_blue'])

        self._info_labels = {}
        fields = [
            ('生产厂家',       'di_mfr'),
            ('产品型号',       'di_model'),
            ('电池生产厂商',   'di_bat_mfr'),
            ('电池型号',       'di_bat_model'),
            ('代码/生产日期',  'di_date'),
            ('安全使用年限',   'di_safety'),
        ]

        for label, key in fields:
            row = tk.Frame(body, bg=COLORS['card_bg'])
            row.pack(fill='x', pady=6)

            tk.Label(row, text=label,
                     bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                     font=_get_tk_font(11),
                     anchor='w').pack(side='left')

            val_lbl = tk.Label(row, text='--',
                                bg=COLORS['card_bg'], fg=COLORS['text_primary'],
                                font=_get_tk_font(11, bold=True),
                                anchor='e')
            val_lbl.pack(side='right')
            self._info_labels[key] = val_lbl

        # 安全使用年限硬编码
        self._info_labels['di_safety'].config(text=SAFETY_YEARS_STR)

    # -----------------------------------------------------------------------
    # 异常监督记录卡片（红色图标）
    # -----------------------------------------------------------------------
    def _build_exc_card(self, parent):
        body = _make_card_frame(parent, '异常监督记录', icon_color=COLORS['icon_red'])

        # Text widget 显示异常记录
        font_exc = _get_tk_font(10)
        txt_frame = tk.Frame(body, bg=COLORS['card_bg'])
        txt_frame.pack(fill='x')

        self._exc_text = tk.Text(
            txt_frame,
            height=5,
            font=font_exc,
            fg=COLORS['exception_text'],
            bg=COLORS['card_bg'],
            bd=0, relief='flat',
            highlightthickness=0,
            wrap='word',
            state='disabled',
        )
        exc_sb = ttk.Scrollbar(txt_frame, orient='vertical',
                               command=self._exc_text.yview)
        self._exc_text.configure(yscrollcommand=exc_sb.set)

        exc_sb.pack(side='right', fill='y')
        self._exc_text.pack(side='left', fill='both', expand=True)

        # 初始提示
        self._exc_text.config(state='normal')
        self._exc_text.insert('end', "暂无异常记录")
        self._exc_text.config(fg=COLORS['text_secondary'], state='disabled')

    # -----------------------------------------------------------------------
    # 工程模式面板
    # -----------------------------------------------------------------------
    def _build_panel_engineering(self, parent):
        frame = tk.Frame(parent, bg=COLORS['card_bg'], bd=1, relief='solid')
        self._eng_panel = frame

        inner = tk.Frame(frame, bg=COLORS['card_bg'])
        inner.pack(fill='x', padx=12, pady=8)

        tk.Label(inner, text="工程模式面板",
                 bg=COLORS['card_bg'], fg=COLORS['text_primary'],
                 font=_get_tk_font(11, bold=True)).pack(anchor='w', pady=(0, 6))

        grid = tk.Frame(inner, bg=COLORS['card_bg'])
        grid.pack(fill='x')

        font_s = _get_tk_font(10)

        # 当前日期
        tk.Label(grid, text="当前日期 (YYYY/MM/DD):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=0, column=0, sticky='w', padx=(0, 8), pady=4)
        self._eng_cur_date_var = tk.StringVar(value=time.strftime("%Y/%m/%d"))
        tk.Entry(grid, textvariable=self._eng_cur_date_var,
                 width=14, font=font_s).grid(row=0, column=1, padx=(0, 16), pady=4)

        # 生产日期
        tk.Label(grid, text="生产日期 (YYYY/MM/DD):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=0, column=2, sticky='w', padx=(0, 8), pady=4)
        self._eng_prod_date_var = tk.StringVar()
        tk.Entry(grid, textvariable=self._eng_prod_date_var,
                 width=14, font=font_s).grid(row=0, column=3, pady=4)

        # 循环次数
        tk.Label(grid, text="循环次数 (0-65535):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=1, column=0, sticky='w', padx=(0, 8), pady=4)
        self._eng_cycle_var = tk.StringVar(value="0")
        tk.Entry(grid, textvariable=self._eng_cycle_var,
                 width=10, font=font_s).grid(row=1, column=1, pady=4)

        # Cell1 电压
        tk.Label(grid, text="Cell1电压(mV):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=2, column=0, sticky='w', padx=(0, 8), pady=4)
        self._eng_cell1_var = tk.StringVar(value="0")
        tk.Entry(grid, textvariable=self._eng_cell1_var,
                 width=10, font=font_s).grid(row=2, column=1, pady=4)

        # Cell2 电压
        tk.Label(grid, text="Cell2电压(mV):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=2, column=2, sticky='w', padx=(0, 8), pady=4)
        self._eng_cell2_var = tk.StringVar(value="0")
        tk.Entry(grid, textvariable=self._eng_cell2_var,
                 width=10, font=font_s).grid(row=2, column=3, pady=4)

        # 虚拟温度
        tk.Label(grid, text="虚拟温度(\u00d70.1\u00b0C):",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).grid(row=3, column=0, sticky='w', padx=(0, 8), pady=4)
        self._eng_temp_var = tk.StringVar(value="0")
        tk.Entry(grid, textvariable=self._eng_temp_var,
                 width=10, font=font_s).grid(row=3, column=1, pady=4)

        # 清除全部记录按钮
        tk.Button(grid, text="清除全部记录",
                  command=self._erase_all_records,
                  font=font_s, relief='flat',
                  bg='#E74C3C', fg='white').grid(row=3, column=2, columnspan=2,
                                                  sticky='w', padx=(0, 8), pady=4)

        btn_frame = tk.Frame(inner, bg=COLORS['card_bg'])
        btn_frame.pack(fill='x', pady=(4, 0))
        tk.Button(btn_frame, text="写入设备",
                  command=self._write_engineering_data,
                  font=font_s, relief='flat',
                  bg=COLORS['normal'], fg='white').pack(side='left')
        self._eng_status_var = tk.StringVar(value="就绪")
        tk.Label(btn_frame, text="状态:",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).pack(side='left', padx=(12, 4))
        tk.Label(btn_frame, textvariable=self._eng_status_var,
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).pack(side='left')

        frame.pack_forget()

    # -----------------------------------------------------------------------
    # 生产模式面板
    # -----------------------------------------------------------------------
    def _build_panel_production(self, parent):
        frame = tk.Frame(parent, bg=COLORS['card_bg'], bd=1, relief='solid')
        self._prod_panel = frame

        inner = tk.Frame(frame, bg=COLORS['card_bg'])
        inner.pack(fill='x', padx=12, pady=8)

        tk.Label(inner, text="生产模式面板",
                 bg=COLORS['card_bg'], fg=COLORS['text_primary'],
                 font=_get_tk_font(11, bold=True)).pack(anchor='w', pady=(0, 6))

        grid = tk.Frame(inner, bg=COLORS['card_bg'])
        grid.pack(fill='x')

        font_s = _get_tk_font(10)
        fields_prod = [
            ("生产厂家:",      '_prod_mfr_var',       0, 0),
            ("产品型号:",      '_prod_model_var',      0, 2),
            ("电池生产厂商:",  '_prod_bat_mfr_var',    1, 0),
            ("电池型号:",      '_prod_bat_model_var',  1, 2),
            ("代码/生产日期:", '_prod_date_var',        2, 0),
        ]
        for label, attr, row, col in fields_prod:
            tk.Label(grid, text=label,
                     bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                     font=font_s).grid(row=row, column=col, sticky='w', padx=(0, 4), pady=4)
            var = tk.StringVar()
            setattr(self, attr, var)
            tk.Entry(grid, textvariable=var,
                     width=22, font=font_s).grid(row=row, column=col + 1, padx=(0, 16), pady=4)

        btn_frame = tk.Frame(inner, bg=COLORS['card_bg'])
        btn_frame.pack(fill='x', pady=(4, 0))
        tk.Button(btn_frame, text="写入设备",
                  command=self._write_production_data,
                  font=font_s, relief='flat',
                  bg=COLORS['normal'], fg='white').pack(side='left')
        self._prod_status_var = tk.StringVar(value="就绪")
        tk.Label(btn_frame, text="状态:",
                 bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
                 font=font_s).pack(side='left', padx=(12, 4))
        self._prod_status_lbl = tk.Label(
            btn_frame, textvariable=self._prod_status_var,
            bg=COLORS['card_bg'], fg=COLORS['text_secondary'],
            font=font_s)
        self._prod_status_lbl.pack(side='left')

        frame.pack_forget()

    # -----------------------------------------------------------------------
    # 模式切换
    # -----------------------------------------------------------------------
    def _on_mode_change(self):
        mode = self._mode_var.get()
        was_eng = self._eng_mode_active

        if mode == 'eng':
            pwd = simpledialog.askstring(
                "工程模式", "请输入工程模式密码：",
                show='*', parent=self.root
            )
            if pwd != ENG_MODE_PASSWORD:
                messagebox.showerror("错误", "密码错误，无法进入工程模式")
                self._mode_var.set('user')
                return
            self._eng_mode_active  = True
            self._prod_mode_active = False
            self._eng_panel.pack(fill='x', padx=16, pady=(0, 8))
            self._prod_panel.pack_forget()

        elif mode == 'prod':
            pwd = simpledialog.askstring(
                "生产模式", "请输入生产模式密码：",
                show='*', parent=self.root
            )
            if pwd != PROD_MODE_PASSWORD:
                messagebox.showerror("错误", "密码错误，无法进入生产模式")
                self._mode_var.set('user')
                return
            self._prod_mode_active = True
            self._eng_mode_active  = False
            self._prod_panel.pack(fill='x', padx=16, pady=(0, 8))
            self._eng_panel.pack_forget()

        else:
            self._eng_mode_active  = False
            self._prod_mode_active = False
            self._eng_panel.pack_forget()
            self._prod_panel.pack_forget()

        # 退出工程模式时通知设备
        if was_eng and not self._eng_mode_active and self._connected:
            threading.Thread(target=self._exit_eng_on_device, daemon=True).start()

    def _exit_eng_on_device(self):
        """写 0x50=0x00 通知 NU17112 退出工程模式"""
        try:
            payload = bytes([0x50, 1, 0x00])
            self._hid_write(CMD_WRITE_REGISTER, payload)
        except Exception:
            pass

    # -----------------------------------------------------------------------
    # 设备枚举
    # -----------------------------------------------------------------------
    def _refresh_devices(self):
        if not HID_AVAILABLE:
            self._status_var.set("hidapi 未安装")
            return
        try:
            vid  = int(self._vid_var.get().strip(), 16)
            pid  = int(self._pid_var.get().strip(), 16)
            page = int(self._page_var.get().strip(), 16)
        except ValueError:
            messagebox.showerror("参数错误", "VID/PID/UsagePage 格式不正确，请输入十六进制")
            return

        devices = hid.enumerate(vid, pid)
        matched = [
            d for d in devices
            if d.get('usage_page', 0) == page
            and d.get('usage', 0) == DEFAULT_USAGE_ID
        ]

        self._dev_paths = []
        combo_values = []
        for d in matched:
            path = d.get('path', b'').decode('utf-8', errors='replace')
            mfr  = d.get('manufacturer_string', '')
            prod = d.get('product_string', '')
            label = f"{mfr} {prod} [{path[:28]}]" if mfr or prod else f"[{path[:40]}]"
            self._dev_paths.append(d.get('path', b''))
            combo_values.append(label)

        self._dev_combo['values'] = combo_values
        if combo_values:
            self._dev_combo.current(0)
            self._status_var.set(f"找到 {len(combo_values)} 个设备")
        else:
            self._status_var.set("未找到匹配设备")

    # -----------------------------------------------------------------------
    # 连接/断开
    # -----------------------------------------------------------------------
    def _toggle_connection(self):
        if self._connected:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        if not HID_AVAILABLE:
            messagebox.showinfo("提示", "hidapi 未安装，请运行: pip install hidapi")
            return

        idx = self._dev_combo.current()
        if idx < 0 or not hasattr(self, '_dev_paths') or not self._dev_paths:
            messagebox.showwarning("未选择设备", "请先刷新并选择设备")
            return

        path = self._dev_paths[idx]
        try:
            dev = hid.device()
            dev.open_path(path)
            dev.set_nonblocking(0)
            self._hid_device = dev
            self._connected  = True
            self._poll_counter = 0
            self._status_var.set("已连接")
            self._conn_btn.config(text="断开", bg=COLORS['error'])

            # 重置设备信息
            self.device_info = DeviceInfo()
            self._clear_device_info_ui()
            self._status_var.set("已连接 — 读取设备信息...")

            # 后台获取设备信息
            t = threading.Thread(target=self._read_device_info_worker, daemon=True)
            t.start()

            # 启动轮询
            self._schedule_poll()

        except Exception as e:
            self._status_var.set(f"连接失败: {e}")
            messagebox.showerror("连接失败", str(e))

    def _disconnect(self):
        self._connected = False
        if self._hid_device:
            try:
                self._hid_device.close()
            except Exception:
                pass
            self._hid_device = None
        self._status_var.set("已断开")
        self._conn_btn.config(text="连接", bg=COLORS['normal'])
        self._clear_all_data()

    def _clear_all_data(self):
        """断开后清空所有卡片数据"""
        if self._soc_ring:
            self._soc_ring.update_soc(0)
        if self._cycle_val_label:
            self._cycle_val_label.config(text='-- 次')

        for key in ('c1_temp', 'c2_temp'):
            if key in self._temp_labels:
                self._temp_labels[key].config(text='--')
        for key in ('c1_status', 'c2_status'):
            if key in self._temp_labels:
                self._temp_labels[key].config(text='--', fg=COLORS['text_secondary'])

        for key in ('v1_volt', 'v2_volt'):
            if key in self._volt_labels:
                self._volt_labels[key].config(text='--')
        for key in ('v1_status', 'v2_status'):
            if key in self._volt_labels:
                self._volt_labels[key].config(text='--', fg=COLORS['text_secondary'])
        if 'total_volt' in self._volt_labels:
            self._volt_labels['total_volt'].config(text='-- V')
        if 'delta_volt' in self._volt_labels:
            self._volt_labels['delta_volt'].config(text='-- V')

        self._clear_device_info_ui()

        if self._exc_text:
            self._exc_text.config(state='normal')
            self._exc_text.delete('1.0', 'end')
            self._exc_text.insert('end', "暂无异常记录")
            self._exc_text.config(fg=COLORS['text_secondary'], state='disabled')
        self._exception_seen_ids.clear()
        self._exception_list.clear()

    def _clear_device_info_ui(self):
        for key in ('di_mfr', 'di_model', 'di_bat_mfr', 'di_bat_model', 'di_date'):
            if key in self._info_labels:
                self._info_labels[key].config(text='读取中...')
        if 'di_safety' in self._info_labels:
            self._info_labels['di_safety'].config(text=SAFETY_YEARS_STR)

    # -----------------------------------------------------------------------
    # HID 读写辅助
    # -----------------------------------------------------------------------
    def _hid_write(self, cmd: int, payload: bytes = b'') -> bool:
        if not self._hid_device or not self._connected:
            return False
        buf = bytearray(HID_WRITE_LENGTH)
        buf[0] = 0x00       # hidapi 前缀
        buf[1] = cmd
        for i, b in enumerate(payload):
            if 2 + i < HID_WRITE_LENGTH:
                buf[2 + i] = b
        try:
            written = self._hid_device.write(bytes(buf))
            return written > 0
        except Exception as e:
            print(f"[ERROR] HID write failed: {e}")
            return False

    def _hid_read(self, timeout_ms: int = 1000) -> Optional[bytes]:
        if not self._hid_device or not self._connected:
            return None
        try:
            data = self._hid_device.read(REPORT_LENGTH, timeout_ms)
            if data:
                return bytes(data)
        except Exception as e:
            print(f"[ERROR] HID read failed: {e}")
            return None
        return None

    # -----------------------------------------------------------------------
    # 设备信息读取（连接后一次性）
    # -----------------------------------------------------------------------
    def _read_device_info_worker(self):
        """后台线程：发送 3 次 CMD_READ_DEVICE_INFO，收集 3 包 Type 0x03"""
        while self._reading_busy:
            time.sleep(0.01)

        self._reading_busy = True
        success = False
        deadline = time.time() + DEVICE_INFO_TIMEOUT

        try:
            for sub_idx in range(3):
                if not self._connected:
                    break
                ok = self._hid_write(CMD_READ_DEVICE_INFO, bytes([sub_idx]))
                if not ok:
                    break

                got_this_sub = False
                retry = 0
                while time.time() < deadline and retry < 6:
                    raw = self._hid_read(1000)
                    if not raw:
                        retry += 1
                        continue
                    print(f"[RX DEV_INFO] {raw.hex()}")
                    rtype, payload = _parse_hid_report(raw)
                    if rtype == TYPE_DEVICE_INFO:
                        done = _parse_device_info_payload(payload, self.device_info)
                        if done:
                            success = True
                        got_this_sub = True
                        break
                    retry += 1

                if got_this_sub and not success:
                    time.sleep(WRITE_STEP_INTERVAL)

                if success:
                    break

        finally:
            self._reading_busy = False

        if success:
            self.root.after(0, self._update_device_info_ui)
        elif self._connected:
            self.root.after(0, self._device_info_timeout)

    def _update_device_info_ui(self):
        di = self.device_info
        mapping = {
            'di_mfr':       di.manufacturer if di.manufacturer else "--",
            'di_model':     di.model        if di.model        else "--",
            'di_bat_mfr':   di.battery_mfr  if di.battery_mfr  else "--",
            'di_bat_model': di.battery_model if di.battery_model else "--",
            'di_date':      di.prod_date    if di.prod_date    else "--",
            'di_safety':    SAFETY_YEARS_STR,
        }
        for key, val in mapping.items():
            if key in self._info_labels:
                self._info_labels[key].config(text=val)
        self._status_var.set("已连接")

    def _device_info_timeout(self):
        if 'di_mfr' in self._info_labels:
            self._info_labels['di_mfr'].config(text="设备信息读取超时")
        for key in ('di_model', 'di_bat_mfr', 'di_bat_model', 'di_date'):
            if key in self._info_labels:
                self._info_labels[key].config(text="")
        if 'di_safety' in self._info_labels:
            self._info_labels['di_safety'].config(text=SAFETY_YEARS_STR)

    # -----------------------------------------------------------------------
    # 周期轮询（1Hz）
    # -----------------------------------------------------------------------
    def _schedule_poll(self):
        if not self._connected:
            return
        self.root.after(1000, self._poll_tick)

    def _poll_tick(self):
        if not self._connected:
            return
        self._poll_counter += 1
        # 定期刷新 ProductInfo
        if PROD_INFO_REFRESH_INTERVAL_S > 0 and self._poll_counter % PROD_INFO_REFRESH_INTERVAL_S == 0:
            if not self._reading_busy:
                self.device_info = DeviceInfo()
                t = threading.Thread(target=self._read_device_info_worker, daemon=True)
                t.start()
        elif not self._reading_busy:
            t = threading.Thread(target=self._read_data_worker, daemon=True)
            t.start()
        self.root.after(1000, self._poll_tick)

    def _read_data_worker(self):
        """后台线程：发一次 CMD_READ_STATUS，解析回复"""
        if self._reading_busy:
            return
        self._reading_busy = True
        try:
            ok = self._hid_write(CMD_READ_STATUS)
            if not ok:
                if self._connected:
                    self.root.after(0, lambda: self._status_var.set("设备已断开"))
                    self._connected = False
                return

            raw = self._hid_read(1000)
            if not raw:
                return

            print(f"[RX] {raw.hex()}")

            rtype, payload = _parse_hid_report(raw)
            if rtype is None:
                return

            if rtype == TYPE_TELEMETRY:
                bd = _parse_telemetry(payload)
                if bd:
                    bd.exception_logs = self.battery_data.exception_logs
                    self.battery_data = bd
                    print(f"  SOC={bd.soc}% V_total={bd.total_voltage:.2f}V "
                          f"Cycle={bd.cycle_count} Temp={bd.bat_temp:.1f}°C "
                          f"Cell1={bd.cell1_voltage:.2f}V Cell2={bd.cell2_voltage:.2f}V")
                    self.root.after(0, self._update_telemetry_ui)

            elif rtype == TYPE_EXCEPT_LOG:
                records = _parse_exception_payload(payload)
                if records:
                    print(f"  [EXC] {len(records)} 条异常日志")
                    self.root.after(0, lambda r=records: self._append_exception_logs(r))

            elif rtype == TYPE_DEVICE_INFO:
                _parse_device_info_payload(payload, self.device_info)
                if self.device_info.loaded:
                    self.root.after(0, self._update_device_info_ui)

        except Exception as e:
            print(f"[ERROR] read_data_worker: {e}")
            if self._connected:
                self.root.after(0, lambda: self._status_var.set(f"读取异常: {e}"))
                self._connected = False
        finally:
            self._reading_busy = False

    # -----------------------------------------------------------------------
    # UI 刷新：遥测
    # -----------------------------------------------------------------------
    def _update_telemetry_ui(self):
        bd = self.battery_data

        # SOC 圆环
        if self._soc_ring:
            self._soc_ring.update_soc(bd.soc)

        # 循环次数
        if self._cycle_val_label:
            self._cycle_val_label.config(text=f"{bd.cycle_count} 次")

        # 电芯温度（两芯共用同一温度传感器）
        temp_str = f"{bd.bat_temp:.2f}"
        temp_ok  = bd.bat_temp <= TEMP_WARN_THRESHOLD
        status_str   = "正常" if temp_ok else "异常"
        status_color = COLORS['normal'] if temp_ok else COLORS['error']
        for prefix in ('c1', 'c2'):
            if f'{prefix}_temp' in self._temp_labels:
                self._temp_labels[f'{prefix}_temp'].config(text=temp_str)
            if f'{prefix}_status' in self._temp_labels:
                self._temp_labels[f'{prefix}_status'].config(
                    text=status_str, fg=status_color)

        # 电芯电压
        for prefix, volt in [('v1', bd.cell1_voltage), ('v2', bd.cell2_voltage)]:
            volt_str   = f"{volt:.2f}"
            volt_ok    = CELL_VOLT_MIN <= volt <= CELL_VOLT_MAX
            status_str_v = "正常" if volt_ok else "异常"
            status_col_v = COLORS['normal'] if volt_ok else COLORS['error']
            if f'{prefix}_volt' in self._volt_labels:
                self._volt_labels[f'{prefix}_volt'].config(text=volt_str)
            if f'{prefix}_status' in self._volt_labels:
                self._volt_labels[f'{prefix}_status'].config(
                    text=status_str_v, fg=status_col_v)

        if 'total_volt' in self._volt_labels:
            self._volt_labels['total_volt'].config(
                text=f"{bd.total_voltage:.2f}V")
        if 'delta_volt' in self._volt_labels:
            self._volt_labels['delta_volt'].config(
                text=f"{bd.voltage_delta:.2f}V")

    # -----------------------------------------------------------------------
    # UI 刷新：异常日志追加
    # -----------------------------------------------------------------------
    def _append_exception_logs(self, records: List[Dict]):
        new_added = False
        for rec in records:
            rid = rec['record_id']
            if rid == 0:        # 无效记录（WB7720 exc_cache 空槽），跳过
                continue
            if rid not in self._exception_seen_ids:
                self._exception_seen_ids.add(rid)
                self._exception_list.append(rec['text'])
                new_added = True

        if not new_added:
            return

        if len(self._exception_list) > MAX_EXCEPTION_DISPLAY:
            self._exception_list = self._exception_list[-MAX_EXCEPTION_DISPLAY:]

        if not self._exc_text:
            return

        self._exc_text.config(state='normal', fg=COLORS['exception_text'])
        self._exc_text.delete('1.0', 'end')
        if self._exception_list:
            for item in self._exception_list:
                self._exc_text.insert('end', item + '\n')
        else:
            self._exc_text.insert('end', "暂无异常记录")
            self._exc_text.config(fg=COLORS['text_secondary'])
        self._exc_text.config(state='disabled')
        self._exc_text.see('end')

    # -----------------------------------------------------------------------
    # 工程模式写入
    # -----------------------------------------------------------------------
    def _write_engineering_data(self):
        if not self._connected:
            messagebox.showwarning("未连接", "请先连接设备")
            return

        try:
            cur_parts = self._eng_cur_date_var.get().strip().split('/')
            cur_year, cur_month, cur_day = int(cur_parts[0]), int(cur_parts[1]), int(cur_parts[2])
            assert cur_year > 2000 and 1 <= cur_month <= 12 and 1 <= cur_day <= 31
        except Exception:
            messagebox.showerror("输入错误", "当前日期格式错误，请使用 YYYY/MM/DD")
            return

        try:
            prod_parts = self._eng_prod_date_var.get().strip().split('/')
            prod_year, prod_month, prod_day = int(prod_parts[0]), int(prod_parts[1]), int(prod_parts[2])
            assert prod_year > 2000 and 1 <= prod_month <= 12 and 1 <= prod_day <= 31
        except Exception:
            messagebox.showerror("输入错误", "生产日期格式错误，请使用 YYYY/MM/DD")
            return

        try:
            cycle = int(self._eng_cycle_var.get().strip())
            assert 0 <= cycle <= 65535
        except Exception:
            messagebox.showerror("输入错误", "循环次数范围 0-65535")
            return

        try:
            cell1_mv = int(self._eng_cell1_var.get().strip())
            if cell1_mv < 0 or cell1_mv > 5000:
                raise ValueError
        except (ValueError, Exception):
            messagebox.showerror("输入错误", "Cell1电压必须为 0-5000 mV")
            return

        try:
            cell2_mv = int(self._eng_cell2_var.get().strip())
            if cell2_mv < 0 or cell2_mv > 5000:
                raise ValueError
        except (ValueError, Exception):
            messagebox.showerror("输入错误", "Cell2电压必须为 0-5000 mV")
            return

        try:
            vtemp = int(self._eng_temp_var.get().strip())
            if vtemp < -500 or vtemp > 1000:
                raise ValueError
        except (ValueError, Exception):
            messagebox.showerror("输入错误", "虚拟温度必须为 -500~1000 (\u00d70.1\u00b0C)")
            return

        self._eng_status_var.set("写入中...")
        t = threading.Thread(
            target=self._write_engineering_worker,
            args=(cur_year, cur_month, cur_day,
                  prod_year, prod_month, prod_day,
                  cycle, cell1_mv, cell2_mv, vtemp),
            daemon=True
        )
        t.start()

    def _write_engineering_worker(
            self, cur_year, cur_month, cur_day,
            prod_year, prod_month, prod_day,
            cycle, cell1_mv, cell2_mv, vtemp):
        """后台线程执行 7 步工程模式写入序列"""
        while self._reading_busy:
            time.sleep(0.01)
        self._reading_busy = True

        try:
            def write_reg(reg, data_bytes):
                payload = bytes([reg, len(data_bytes)]) + data_bytes
                self._hid_write(CMD_WRITE_REGISTER, payload)
                time.sleep(WRITE_STEP_INTERVAL)

            # Step 1: 解锁工程模式
            write_reg(0x50, bytes([0xA5]))
            # Step 2: 当前日期
            write_reg(0x60, struct.pack('<H', cur_year) + bytes([cur_month, cur_day]))
            # Step 3: 生产日期
            write_reg(0x70, struct.pack('<H', prod_year) + bytes([prod_month, prod_day]))
            # Step 4: 循环次数
            write_reg(0x80, struct.pack('<H', cycle))
            # Step 5: 虚拟 Cell1 电压
            write_reg(0x82, struct.pack('<H', cell1_mv))
            # Step 6: 虚拟 Cell2 电压
            write_reg(0x84, struct.pack('<H', cell2_mv))
            # Step 7: 虚拟温度 (s16 LE)
            write_reg(0x86, struct.pack('<h', vtemp))
            # Step 8: 触发 NU17112 重新读取 (支持重复写入)
            write_reg(0x88, bytes([0xAA]))

            self.root.after(0, lambda: self._eng_status_var.set("写入成功"))
        except Exception as e:
            self.root.after(0, lambda: self._eng_status_var.set(f"写入失败: {e}"))
            self.root.after(0, lambda: messagebox.showerror("写入失败", str(e)))
        finally:
            self._reading_busy = False

    # -----------------------------------------------------------------------
    # 工程模式：清除全部异常记录
    # -----------------------------------------------------------------------
    def _erase_all_records(self):
        if not self._connected:
            messagebox.showwarning("未连接", "请先连接设备")
            return
        if not self._eng_mode_active:
            messagebox.showwarning("非工程模式", "请先进入工程模式并写入设备")
            return
        if not messagebox.askyesno("确认清除",
                                   "确定要清除所有异常记录吗？\n此操作不可恢复。"):
            return

        self._eng_status_var.set("清除中...")
        threading.Thread(target=self._erase_worker, daemon=True).start()

    def _erase_worker(self):
        """后台线程执行清除全部异常记录命令"""
        while self._reading_busy:
            time.sleep(0.01)
        self._reading_busy = True

        try:
            payload = bytes([0x88, 1, 0xEE])
            self._hid_write(CMD_WRITE_REGISTER, payload)
            time.sleep(0.5)
            # 清空 PC 端缓存
            self._exception_seen_ids.clear()
            self._exception_list.clear()
            self.root.after(0, self._refresh_exception_display)
            self.root.after(0, lambda: self._eng_status_var.set("清除成功"))
        except Exception as e:
            self.root.after(0, lambda: self._eng_status_var.set(f"清除失败: {e}"))
        finally:
            self._reading_busy = False

    def _refresh_exception_display(self):
        """刷新异常记录显示区域（清除后重新渲染）"""
        if not self._exc_text:
            return
        self._exc_text.config(state='normal')
        self._exc_text.delete('1.0', 'end')
        if self._exception_list:
            self._exc_text.config(fg=COLORS['exception_text'])
            for item in self._exception_list:
                self._exc_text.insert('end', item + '\n')
        else:
            self._exc_text.insert('end', "暂无异常记录")
            self._exc_text.config(fg=COLORS['text_secondary'])
        self._exc_text.config(state='disabled')

    # -----------------------------------------------------------------------
    # 生产模式写入
    # -----------------------------------------------------------------------
    def _write_production_data(self):
        if not self._connected:
            messagebox.showwarning("未连接", "请先连接设备")
            return

        def _encode_field(s: str, max_len=19) -> bytes:
            raw = s.encode('utf-8')[:max_len]
            return raw + b'\x00' * (20 - len(raw))

        mfr       = self._prod_mfr_var.get().strip()
        model     = self._prod_model_var.get().strip()
        bat_mfr   = self._prod_bat_mfr_var.get().strip()
        bat_model = self._prod_bat_model_var.get().strip()
        date_str  = self._prod_date_var.get().strip()

        if not any([mfr, model, bat_mfr, bat_model, date_str]):
            messagebox.showwarning("输入为空", "请至少填写一个字段")
            return

        fields_bytes = [
            _encode_field(mfr),
            _encode_field(model),
            _encode_field(bat_mfr),
            _encode_field(bat_model),
            _encode_field(date_str),
        ]

        self._prod_status_var.set("写入中...")
        self._prod_status_lbl.config(fg=COLORS['warning'])
        t = threading.Thread(
            target=self._write_production_worker,
            args=(fields_bytes,),
            daemon=True
        )
        t.start()

    def _write_production_worker(self, fields_bytes):
        """后台线程执行 7 步生产模式写入序列"""
        while self._reading_busy:
            time.sleep(0.01)
        self._reading_busy = True

        reg_addrs = [0x92, 0xA6, 0xBA, 0xCE, 0xE2]

        def write_reg(reg, data_bytes):
            payload = bytes([reg, len(data_bytes)]) + data_bytes
            self._hid_write(CMD_WRITE_REGISTER, payload)
            time.sleep(WRITE_STEP_INTERVAL)

        try:
            for addr, fb in zip(reg_addrs, fields_bytes):
                write_reg(addr, fb)

            write_reg(0x90, bytes([0xB5]))

            self.root.after(0, lambda: self._prod_status_var.set("等待确认..."))
            deadline = time.time() + 5.0
            status_result = None

            while time.time() < deadline:
                time.sleep(0.2)
                raw = self._hid_read(300)
                if raw:
                    rtype, payload = _parse_hid_report(raw)
                    if rtype == TYPE_TELEMETRY and payload and len(payload) > 0:
                        status_result = 0x02
                        break

            if status_result == 0x02:
                self.root.after(0, lambda: (
                    self._prod_status_var.set("写入成功"),
                    self._prod_status_lbl.config(fg=COLORS['normal'])
                ))
            else:
                self.root.after(0, lambda: (
                    self._prod_status_var.set("超时或写入失败"),
                    self._prod_status_lbl.config(fg=COLORS['error'])
                ))

        except Exception as e:
            self.root.after(0, lambda: (
                self._prod_status_var.set(f"写入失败: {e}"),
                self._prod_status_lbl.config(fg=COLORS['error'])
            ))
            self.root.after(0, lambda: messagebox.showerror("写入失败", str(e)))
        finally:
            self._reading_busy = False

    # -----------------------------------------------------------------------
    # 关闭
    # -----------------------------------------------------------------------
    def _on_close(self):
        self._connected = False
        if self._hid_device:
            try:
                self._hid_device.close()
            except Exception:
                pass
        self.root.destroy()


# ---------------------------------------------------------------------------
# 入口
# ---------------------------------------------------------------------------
def main():
    if not PIL_AVAILABLE:
        print("[ERROR] Pillow 未安装，请运行: pip install Pillow")
        print("UI 将以降级模式运行（无 PNG 叠加效果）")

    root = tk.Tk()

    # 尝试加载应用图标
    icon_path = os.path.join(UI_DIR, 'logo.png')
    if PIL_AVAILABLE and os.path.isfile(icon_path):
        try:
            img = Image.open(icon_path).convert('RGBA')
            bg = Image.new('RGBA', img.size, (255, 255, 255, 255))
            composite = Image.alpha_composite(bg, img)
            _icon_img = ImageTk.PhotoImage(composite)
            root.iconphoto(True, _icon_img)
        except Exception:
            pass

    app = BatteryMonitorApp(root)
    root.mainloop()


if __name__ == '__main__':
    main()
