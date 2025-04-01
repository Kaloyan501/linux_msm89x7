// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2025 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct jd9365zplus_c3e {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
};

static inline
struct jd9365zplus_c3e *to_jd9365zplus_c3e(struct drm_panel *panel)
{
	return container_of(panel, struct jd9365zplus_c3e, panel);
}

static void jd9365zplus_c3e_reset(struct jd9365zplus_c3e *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(120);
}

static int jd9365zplus_c3e_on(struct jd9365zplus_c3e *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xdf, 0x93, 0x65, 0xf8);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x51, 0xff);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x55, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 30);

	return dsi_ctx.accum_err;
}

static int jd9365zplus_c3e_off(struct jd9365zplus_c3e *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xdf, 0x93, 0x65, 0xf8);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int jd9365zplus_c3e_prepare(struct drm_panel *panel)
{
	struct jd9365zplus_c3e *ctx = to_jd9365zplus_c3e(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	jd9365zplus_c3e_reset(ctx);

	ret = jd9365zplus_c3e_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		return ret;
	}

	return 0;
}

static int jd9365zplus_c3e_unprepare(struct drm_panel *panel)
{
	struct jd9365zplus_c3e *ctx = to_jd9365zplus_c3e(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = jd9365zplus_c3e_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);

	return 0;
}

static const struct drm_display_mode jd9365zplus_c3e_mode = {
	.clock = (720 + 30 + 20 + 20) * (1440 + 8 + 2 + 16) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 30,
	.hsync_end = 720 + 30 + 20,
	.htotal = 720 + 30 + 20 + 20,
	.vdisplay = 1440,
	.vsync_start = 1440 + 8,
	.vsync_end = 1440 + 8 + 2,
	.vtotal = 1440 + 8 + 2 + 16,
	.width_mm = 62,
	.height_mm = 124,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int jd9365zplus_c3e_get_modes(struct drm_panel *panel,
				     struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &jd9365zplus_c3e_mode);
}

static const struct drm_panel_funcs jd9365zplus_c3e_panel_funcs = {
	.prepare = jd9365zplus_c3e_prepare,
	.unprepare = jd9365zplus_c3e_unprepare,
	.get_modes = jd9365zplus_c3e_get_modes,
};

static int jd9365zplus_c3e_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct jd9365zplus_c3e *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	drm_panel_init(&ctx->panel, dev, &jd9365zplus_c3e_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void jd9365zplus_c3e_remove(struct mipi_dsi_device *dsi)
{
	struct jd9365zplus_c3e *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id jd9365zplus_c3e_of_match[] = {
	{ .compatible = "xiaomi,pine-jd9365z-c3e" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, jd9365zplus_c3e_of_match);

static struct mipi_dsi_driver jd9365zplus_c3e_driver = {
	.probe = jd9365zplus_c3e_probe,
	.remove = jd9365zplus_c3e_remove,
	.driver = {
		.name = "panel-xiaomi-pine-jd9365z-c3e",
		.of_match_table = jd9365zplus_c3e_of_match,
	},
};
module_mipi_dsi_driver(jd9365zplus_c3e_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for jd9365z hdplus c3e video mode dsi panel");
MODULE_LICENSE("GPL");
