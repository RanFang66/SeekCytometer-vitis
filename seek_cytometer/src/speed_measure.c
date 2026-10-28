#include "speed_measure.h"

#include "signal_analyzer.h"
#include "log.h"

/* 默认: 关闭; pre=CH1 post=CH2; 20um 光斑间距、1~20us 窗口(1~20 m/s)。
 * 这些值只是"第一次上电且上位机还没下发"时的兜底, 上位机会立即覆盖。 */
static struct speed_measure_config m_cfg = {
    .enabled           = 0,
    .pre_ch            = 1,
    .post_ch           = 2,
    .min_us            = 1,
    .max_us            = 20,
    .spot_dist_um      = 20,
    .peak_margin_codes = 0
};

static u8_t m_cfg_valid = 0;   /* 是否收到过上位机配置 */

static u32_t clamp_u32(u32_t v, u32_t lo, u32_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* 把配置写进 PL。 */
static void speed_measure_write_pl(void)
{
    /* 寄存器层: 控制位 + 通道选择 + min/max + 间距。 */
    write_speed_measure_settings(m_cfg.enabled,
                                 m_cfg.pre_ch,
                                 m_cfg.post_ch,
                                 (int)m_cfg.max_us,
                                 (int)m_cfg.min_us,
                                 (int)m_cfg.spot_dist_um);

    /* 峰值质量门: 全局参数(所有通道共用), 不是测速专用, 但由这里统一维护。 */
    write_peak_margin(m_cfg.peak_margin_codes);
}

void speed_measure_configure(const struct speed_measure_config *cfg)
{
    if (cfg == 0)
        return;

    m_cfg = *cfg;

    /* 夹取范围: 上下界都按 us, 硬件是 32 位寄存器, 这里只做"防呆"夹取。 */
    m_cfg.min_us       = clamp_u32(m_cfg.min_us, 0, 10000);
    m_cfg.max_us       = clamp_u32(m_cfg.max_us, 1, 1000000);
    m_cfg.spot_dist_um = clamp_u32(m_cfg.spot_dist_um, 1, 100000);
    m_cfg.peak_margin_codes = clamp_u32(m_cfg.peak_margin_codes, 0, 262143);

    if (m_cfg.pre_ch < 1) m_cfg.pre_ch = 1;
    if (m_cfg.pre_ch > 8) m_cfg.pre_ch = 8;
    if (m_cfg.post_ch < 1) m_cfg.post_ch = 1;
    if (m_cfg.post_ch > 8) m_cfg.post_ch = 8;

    /* 下界大于等于上界时, 窗口为空 -> 所有测量都会判无效, 这里直接纠正成
     * "上界至少比下界大 1us", 免得现场调成空窗口后以为测速坏了。 */
    if (m_cfg.max_us <= m_cfg.min_us)
        m_cfg.max_us = m_cfg.min_us + 1;

    m_cfg_valid = 1;
    speed_measure_write_pl();

    LOG_INFO("Speed measure: en=%d pre=CH%d post=CH%d dt=[%d,%d]us dist=%dum margin=%d",
             m_cfg.enabled, m_cfg.pre_ch, m_cfg.post_ch,
             (int)m_cfg.min_us, (int)m_cfg.max_us,
             (int)m_cfg.spot_dist_um, (int)m_cfg.peak_margin_codes);
}

void speed_measure_get_config(struct speed_measure_config *out)
{
    if (out)
        *out = m_cfg;
}

void speed_measure_resync(void)
{
    if (!m_cfg_valid)
        return;
    speed_measure_write_pl();
    LOG_INFO("Speed measure: re-applied cached config (en=%d)", m_cfg.enabled);
}

u8_t speed_measure_enabled(void)
{
    return m_cfg.enabled;
}

u32_t speed_measure_dist_um(void)
{
    return m_cfg.spot_dist_um;
}
