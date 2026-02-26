# Port Manager Refactoring - Risk Documentation

This document records risks and issues discovered in the original `port_manager.c` / `port_manager.h` code during the refactoring process. These are **pre-existing issues**, not introduced by the refactoring.

## Risk Summary

| # | Risk | Location | Severity | Status |
|---|------|----------|----------|--------|
| R1 | PPS mode can be overwritten by ADP_FIX | port0/1/2/3_connect_closed | High | Not fixed (preserved original behavior) |
| R2 | BIT macro missing parentheses around parameter | port_manager.h:81 | Medium | Fixed in Task 1 |
| R3 | Global variable defined in header file | port_manager.h:94 | Medium | Fixed in Task 1 (extern + .c definition) |
| R4 | Port1 charger mode uses Port0 event name | port1_connect_success:1348 | Low | Not fixed (preserved original behavior) |
| R5 | Self-assignment no-op code | snk_setcharge:786-787 | Low | Not fixed (preserved original behavior) |
| R6 | Redundant branches in WPC mode determination | Multiple connect_closed functions | Low | Not fixed (preserved original behavior) |
| R7 | Garbled comments (GBK->UTF-8 encoding corruption) | Multiple locations | Low | Fixed in Task 4 |
| R8 | Header guard end-comment mismatch | port_manager.h:114 | Low | Fixed in Task 1 |

## Detailed Risk Descriptions

### R1: PPS mode can be overwritten by ADP_FIX (HIGH)

**Location**: `port_enum_port0_connect_closed()`, `port_enum_port1_connect_closed()`, `port_enum_port2_connect_closed()`, `port_enum_port3_connect_closed()` - WPC mode determination in charger mode when no source ports active.

**Description**: In the `connect_closed` functions, when `pdlib_is_connect()` is true, two independent `if` statements are used:
```c
if(pdlib_is_pps_sink())  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
else tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
```
If both conditions are true, the PPS mode set by the first `if` is immediately overwritten by `TCPM_WPC_WORK_ADP_FIX` from the second `if`.

**Contrast**: In `port_enum_port_enum_done()`, the same logic correctly uses `if/else`:
```c
if(pdlib_is_pps_sink())
    tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
else {
    if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2)
        tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
    else
        tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
}
```

**Impact**: PPS charging mode may never be activated through connect_closed paths.

**Decision**: Not fixed during refactoring to maintain behavior equivalence. The extracted helper function `update_wpc_work_mode_for_charger()` preserves this distinction:
- `pps_exclusive=true` (used by enum_done): if/else mutually exclusive
- `pps_exclusive=false` (used by connect_closed): two independent ifs, preserving R1 behavior

### R2: BIT macro missing parentheses (MEDIUM) - FIXED

**Location**: `port_manager.h:81`

**Original**: `#define BIT(n) (0x01ul << n)`
**Fixed**: `#define BIT(n) (0x01ul << (n))`

**Impact**: `BIT(a+1)` would expand to `(0x01ul << a+1)` which is `(0x01ul << a) + 1` due to operator precedence, not the intended `(0x01ul << (a+1))`.

### R3: Global variable defined in header file (MEDIUM) - FIXED

**Location**: `port_manager.h:94`

**Original**: `struct port_infos g_port;` (definition in header)
**Fixed**: `extern struct port_infos g_port;` (declaration in header) + `struct port_infos g_port;` (definition in port_manager.c)

**Impact**: Every translation unit that includes `port_manager.h` would get its own tentative definition of `g_port`. This works due to C's "common symbol" linker behavior, but is non-standard and fragile.

### R4: Port1 charger mode uses Port0 event name (LOW)

**Location**: `port_enum_port1_connect_success()` line 1348

**Description**: When Port1 is in charger+sink mode, the timer fires `PORT_ENUM_EVT_PORT0_SINK_SETVOLT` instead of `PORT_ENUM_EVT_PORT1_SINK_SETVOLT`:
```c
osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
```

**Impact**: Both PORT0 and PORT1 SINK_SETVOLT events call the same function `port_enum_port_snk_setvolt()`, so the runtime behavior is identical. However, this makes debugging/tracing confusing since the event name doesn't match the port.

**Decision**: Not fixed to maintain exact behavior equivalence. Logged for future fix.

### R5: Self-assignment no-op code (LOW)

**Location**: `port_enum_port_snk_setcharge()` lines 786-787

**Description**:
```c
g_port.ibat_limit = g_port.ibat_limit;
g_port.ibus_limit = g_port.ibus_limit;
```
These are no-op self-assignments in the `else` branch (when no wireless present).

**Impact**: No functional impact, just dead code.

### R6: Redundant branches in WPC mode determination (LOW)

**Location**: Multiple `connect_closed` and `connect_start` functions, in the BC1.2 fallback path.

**Description**:
```c
if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
    tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
else if(bc12_type > BC1P2_CDP)
    tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
else
    tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
```
The `else if` and `else` branches produce the same result (`TCPM_WPC_WORK_FIX5V`).

**Impact**: No functional impact, just unnecessary code complexity.

### R7: Garbled comments (LOW) - FIXED in Task 4

**Location**: Multiple locations throughout port_manager.c

**Description**: Comments containing Chinese characters were corrupted during GBK->UTF-8 encoding conversion, producing garbled text like `锟斤拷锟铰匡拷锟斤拷toogle`.

**Impact**: No functional impact, but reduces code readability.

### R8: Header guard end-comment mismatch (LOW) - FIXED

**Location**: `port_manager.h:114`

**Original**: `#endif /* FML_H_ */`
**Fixed**: `#endif /* PORT_MANAGER_H_ */`

**Impact**: No functional impact (comment only), but misleading for maintainers.
