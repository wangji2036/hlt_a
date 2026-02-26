# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['battery_monitor.py'],
    pathex=[],
    binaries=[],
    datas=[('..\\ARUN_N3C_UI设计\\UI\\logo.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\环形电量显示.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\电芯温度框.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\电芯电压框.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\设备基本信息框.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\异常监督记录框.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\红色电池图标.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\蓝色电池图标.png', 'UI'), ('..\\ARUN_N3C_UI设计\\UI\\Font-OPPOSans\\OPPOSans-M.ttf', 'UI\\Font-OPPOSans'), ('..\\ARUN_N3C_UI设计\\UI\\Font-OPPOSans\\OPPOSans-B.ttf', 'UI\\Font-OPPOSans'), ('..\\ARUN_N3C_UI设计\\UI\\Font-OPPOSans\\OPPOSans-R.ttf', 'UI\\Font-OPPOSans')],
    hiddenimports=['hid', 'PIL.Image', 'PIL.ImageTk'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='ARUN_Monitor',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
