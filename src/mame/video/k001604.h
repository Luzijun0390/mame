// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_K001604_H
#define MAME_VIDEO_K001604_H

#pragma once

#include "tilemap.h"


class k001604_device : public device_t, public device_gfx_interface
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_layer_size(int size) { m_layer_size = size; }
	void set_roz_size(int size) { m_roz_size = size; }
	void set_txt_mem_offset(int offs) { m_txt_mem_offset = offs; }
	void set_roz_mem_offset(int offs) { m_roz_mem_offset = offs; }

	void draw_back_layer( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void draw_front_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void tile_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t tile_r(offs_t offset);
	void char_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t char_r(offs_t offset);
	void reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t reg_r(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	int            m_layer_size;        // 0 -> width = 128 tiles, 1 -> width = 256 tiles
	int            m_roz_size;          // 0 -> 8x8, 1 -> 16x16
	int            m_txt_mem_offset;
	int            m_roz_mem_offset;

	tilemap_t      *m_layer_8x8[2];
	tilemap_t      *m_layer_roz;

	std::unique_ptr<uint32_t[]>       m_tile_ram;
	std::unique_ptr<uint32_t[]>       m_char_ram;
	std::unique_ptr<uint32_t[]>       m_reg;

	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_0_size0);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_0_size1);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_1_size0);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_1_size1);
	TILEMAP_MAPPER_MEMBER(scan_layer_roz_256);
	TILEMAP_MAPPER_MEMBER(scan_layer_roz_128);
	TILE_GET_INFO_MEMBER(tile_info_layer_8x8);
	TILE_GET_INFO_MEMBER(tile_info_layer_roz);
};

DECLARE_DEVICE_TYPE(K001604, k001604_device)

#endif // MAME_VIDEO_K001604_H
