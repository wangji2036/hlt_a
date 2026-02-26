import sys
import threading
import tkinter as tk
from tkinter import ttk, messagebox

try:
    import hid
except Exception as e:
    raise RuntimeError("需要安装hidapi库: 请运行 `pip install hidapi`")


VENDOR_ID = 0xFFFF
PRODUCT_ID = 0xFFFF
USAGE_PAGE = 0xFF00
USAGE_ID = 0x01


class HidReaderApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("USB HID读取工具")
        self.devices = []
        self.selected_index = tk.IntVar(value=-1)
        self.auto_running = False
        self.auto_job = None
        self.reading_busy = False

        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.grid(row=0, column=0, sticky="nsew")
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)

        device_row = ttk.Frame(main_frame)
        device_row.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        device_row.columnconfigure(1, weight=1)

        ttk.Label(device_row, text="设备").grid(row=0, column=0, sticky="w", padx=(0, 8))
        self.device_combo = ttk.Combobox(device_row, state="readonly")
        self.device_combo.grid(row=0, column=1, sticky="ew")
        ttk.Button(device_row, text="刷新", command=self.refresh_devices).grid(row=0, column=2, padx=(8, 0))

        length_row = ttk.Frame(main_frame)
        length_row.grid(row=1, column=0, sticky="ew", pady=(0, 8))
        ttk.Label(length_row, text="报告长度").grid(row=0, column=0, sticky="w", padx=(0, 8))
        self.report_length_var = tk.StringVar(value="64")
        self.report_length_entry = ttk.Entry(length_row, textvariable=self.report_length_var, width=8)
        self.report_length_entry.grid(row=0, column=1, sticky="w")
        ttk.Label(length_row, text="超时(ms)").grid(row=0, column=2, sticky="w", padx=(16, 8))
        self.timeout_var = tk.StringVar(value="1000")
        self.timeout_entry = ttk.Entry(length_row, textvariable=self.timeout_var, width=8)
        self.timeout_entry.grid(row=0, column=3, sticky="w")
        ttk.Label(length_row, text="自动(ms)").grid(row=0, column=4, sticky="w", padx=(16, 8))
        self.auto_interval_var = tk.StringVar(value="1000")
        self.auto_interval_entry = ttk.Entry(length_row, textvariable=self.auto_interval_var, width=8)
        self.auto_interval_entry.grid(row=0, column=5, sticky="w")

        action_row = ttk.Frame(main_frame)
        action_row.grid(row=2, column=0, sticky="ew", pady=(0, 8))
        ttk.Button(action_row, text="读取数据", command=self.read_data).grid(row=0, column=0)
        ttk.Button(action_row, text="清空显示", command=self.clear_output).grid(row=0, column=1, padx=(8, 0))
        ttk.Button(action_row, text="开始自动读取", command=self.start_auto).grid(row=0, column=2, padx=(8, 0))
        ttk.Button(action_row, text="停止自动读取", command=self.stop_auto).grid(row=0, column=3, padx=(8, 0))

        latest_frame = ttk.LabelFrame(main_frame, text="最新数据")
        latest_frame.grid(row=3, column=0, sticky="nsew", pady=(0, 8))
        latest_frame.columnconfigure(0, weight=1)
        latest_frame.rowconfigure(0, weight=1)
        self.latest_text = tk.Text(latest_frame, wrap="none", height=6)
        self.latest_text.grid(row=0, column=0, sticky="nsew")
        latest_scroll_y = ttk.Scrollbar(latest_frame, orient="vertical", command=self.latest_text.yview)
        latest_scroll_y.grid(row=0, column=1, sticky="ns")
        self.latest_text.configure(yscrollcommand=latest_scroll_y.set)

        output_frame = ttk.Frame(main_frame)
        output_frame.grid(row=4, column=0, sticky="nsew")
        main_frame.rowconfigure(4, weight=1)
        output_frame.columnconfigure(0, weight=1)
        output_frame.rowconfigure(0, weight=1)

        self.output_text = tk.Text(output_frame, wrap="none", height=16)
        self.output_text.grid(row=0, column=0, sticky="nsew")
        scroll_y = ttk.Scrollbar(output_frame, orient="vertical", command=self.output_text.yview)
        scroll_y.grid(row=0, column=1, sticky="ns")
        self.output_text.configure(yscrollcommand=scroll_y.set)

        status_row = ttk.Frame(main_frame)
        status_row.grid(row=5, column=0, sticky="ew", pady=(8, 0))
        status_row.columnconfigure(0, weight=1)
        self.status_var = tk.StringVar(value="就绪")
        self.status_label = ttk.Label(status_row, textvariable=self.status_var, anchor="w")
        self.status_label.grid(row=0, column=0, sticky="ew")

        self.refresh_devices()

    def set_status(self, text: str):
        self.status_var.set(text)

    def clear_output(self):
        self.output_text.delete("1.0", tk.END)

    def refresh_devices(self):
        try:
            # 终端日志：平台、设备总数
            print(f"平台: {sys.platform}", flush=True)
            all_devs = list(hid.enumerate())
            print(f"系统HID设备总数: {len(all_devs)}", flush=True)

            # 先按VID/PID筛选，再按usage筛选
            candidates = [d for d in all_devs if d.get("vendor_id") == VENDOR_ID and d.get("product_id") == PRODUCT_ID]
            print(f"符合VID/PID的候选设备数量: {len(candidates)} (VID=0x{VENDOR_ID:04X}, PID=0x{PRODUCT_ID:04X})", flush=True)

            matches = []
            for d in candidates:
                up = d.get("usage_page")
                us = d.get("usage")
                if up == USAGE_PAGE and us == USAGE_ID:
                    matches.append(d)
                else:
                    print(f"候选未通过: usage_page={up}, usage={us}", flush=True)

            self.devices = matches
            display_items = []
            for d in self.devices:
                manufacturer = d.get("manufacturer_string") or ""
                product = d.get("product_string") or "HID Device"
                serial = d.get("serial_number") or ""
                item = f"{product} {serial} [{manufacturer}]"
                display_items.append(item)
            self.device_combo["values"] = display_items
            if display_items:
                self.device_combo.current(0)
                self.set_status(f"发现 {len(display_items)} 个匹配设备")
            else:
                self.device_combo.set("")
                self.set_status("未发现匹配设备")
        except Exception as e:
            self.set_status(f"枚举失败: {e}")
            print(f"枚举异常: {e}", flush=True)

    def read_data(self):
        if not self.devices:
            messagebox.showwarning("提示", "没有找到匹配的设备")
            return
        idx = self.device_combo.current()
        if idx < 0 or idx >= len(self.devices):
            idx = 0  # 默认选择第一个设备
        try:
            report_len = int(self.report_length_var.get())
            timeout_ms = int(self.timeout_var.get())
            if report_len <= 0:
                raise ValueError("报告长度必须为正数")
        except Exception:
            messagebox.showerror("错误", "请输入有效的报告长度和超时")
            return

        self.spawn_read(idx, report_len, timeout_ms)

    def spawn_read(self, idx: int, report_len: int, timeout_ms: int):
        if self.reading_busy:
            # 正在读取，跳过本次触发以避免重入
            return
        self.set_status("正在读取...")
        threading.Thread(target=self._read_worker, args=(idx, report_len, timeout_ms), daemon=True).start()

    def start_auto(self):
        try:
            interval = int(self.auto_interval_var.get())
            if interval <= 0:
                raise ValueError
        except Exception:
            messagebox.showerror("错误", "请输入有效的自动读取间隔(ms)")
            return
        self.auto_running = True
        self.set_status("自动读取: 已启动")
        self._schedule_auto()

    def stop_auto(self):
        self.auto_running = False
        if self.auto_job is not None:
            try:
                self.root.after_cancel(self.auto_job)
            except Exception:
                pass
            self.auto_job = None
        self.set_status("自动读取: 已停止")

    def _schedule_auto(self):
        if not self.auto_running:
            return
        try:
            interval = int(self.auto_interval_var.get())
        except Exception:
            interval = 1000
        self.auto_job = self.root.after(interval, self._auto_tick)

    def _auto_tick(self):
        if not self.auto_running:
            return
        if not self.devices:
            # 没有设备，继续调度
            self._schedule_auto()
            return
        idx = self.device_combo.current()
        if idx < 0 or idx >= len(self.devices):
            idx = 0
        try:
            report_len = int(self.report_length_var.get())
            timeout_ms = int(self.timeout_var.get())
        except Exception:
            report_len = 64
            timeout_ms = 1000
        self.spawn_read(idx, report_len, timeout_ms)
        self._schedule_auto()

    def _read_worker(self, idx: int, report_len: int, timeout_ms: int):
        d = self.devices[idx]
        try:
            self.reading_busy = True
            dev = hid.device()
            if d.get("path") is not None:
                dev.open_path(d["path"])
            else:
                dev.open(VENDOR_ID, PRODUCT_ID, d.get("serial_number") or None)
            try:
                zero_report = bytes([0] * report_len)
                print(f"写入零包: 长度={report_len} 字节, 超时={timeout_ms}ms", flush=True)
                dev.write(zero_report)
                data = dev.read(report_len, timeout_ms)
                if data is None:
                    self._append_output("读超时或无数据\n")
                    self.set_status("读取完成: 无数据")
                else:
                    hex_str = " ".join(f"{b:02X}" for b in data)
                    ascii_str = "".join(chr(b) if 32 <= b <= 126 else "." for b in data)
                    self._append_output(f"HEX: {hex_str}\nASCII: {ascii_str}\n\n")
                    self._update_latest(f"HEX: {hex_str}\nASCII: {ascii_str}")
                    self.set_status(f"读取完成: {len(data)} 字节")
            finally:
                try:
                    dev.close()
                except Exception:
                    pass
        except Exception as e:
            self._append_output(f"错误: {e}\n")
            self.set_status("读取失败")
            print(f"读取异常: {e}", flush=True)
        finally:
            self.reading_busy = False

    def _append_output(self, text: str):
        def _do():
            self.output_text.insert(tk.END, text)
            self.output_text.see(tk.END)
        self.root.after(0, _do)

    def _update_latest(self, text: str):
        def _do():
            self.latest_text.configure(state="normal")
            self.latest_text.delete("1.0", tk.END)
            self.latest_text.insert("1.0", text)
            self.latest_text.configure(state="disabled")
        self.root.after(0, _do)


def main():
    root = tk.Tk()
    app = HidReaderApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()