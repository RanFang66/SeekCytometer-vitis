#ifndef SPEED_MEASURE_H
#define SPEED_MEASURE_H

#include "xil_types.h"

/*
 * 细胞测速（双光斑峰时间差）—— 配置与寄存器下发的独立模块。
 *
 * 目的: 把"测速"从 cytometer.c / signal_analyzer.c / udp_server.c 里散落的
 * 逻辑收拢成一处:
 *   - 保存一份当前配置(缓存), PL 重新下载 bitstream 或 START 采集后可重下发;
 *   - 唯一的下发入口, 参数夹取范围也在这里做;
 *   - 上层(udp_server)只负责解析报文, 不再直接碰寄存器。
 *
 * 物理含义: 一个细胞先后穿过两个固定间距的光斑, 两个通道各产生一个脉冲;
 * 用两个脉冲的峰时间差 dt 和光斑间距 d 得到流速 v = d / dt。
 * dt 的合法窗口是 [min_us, max_us]:
 *   - 上界挡掉"用了上一个细胞的 pre 峰时间"(漏检 pre 时 dt 约等于一个细胞间隔);
 *   - 下界挡掉"两个峰落在同一时间刻度"或 pre 通道被串扰触发形成的近零 dt;
 *   - 另有 peak_margin_codes: 峰值必须比检测阈值再高这么多码才算有效脉冲,
 *     用于丢弃刚跨过阈值的噪声毛刺(实测 peak 548/宽度 4 样本那种)。
 */
struct speed_measure_config
{
    u8_t  enabled;            /* 0 = 关闭 (控制位 slv_reg0[2]) */
    u8_t  pre_ch;             /* 前光斑通道, 1..8 */
    u8_t  post_ch;            /* 后光斑通道, 1..8 */
    u32_t min_us;             /* dt 下界(us), 0 = 不设下界  -> slv_reg45 */
    u32_t max_us;             /* dt 上界(us)              -> slv_reg53 */
    u32_t spot_dist_um;       /* 两光斑间距(um)           -> slv_reg54 */
    u32_t peak_margin_codes;  /* 峰值质量门余量(码), 0=关闭 -> slv_reg46 */
};

/* 下发并缓存配置。参数会先夹到合法范围。 */
void speed_measure_configure(const struct speed_measure_config *cfg);

/* 取当前配置副本。 */
void speed_measure_get_config(struct speed_measure_config *out);

/* PL 复位/重新下载后重新下发缓存值(不发一次性动作)。未配置过则什么都不做。 */
void speed_measure_resync(void);

u8_t  speed_measure_enabled(void);
u32_t speed_measure_dist_um(void);

#endif /* SPEED_MEASURE_H */
